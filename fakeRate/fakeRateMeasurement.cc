

//include c++ library classes 
#include <vector>
#include <utility>
#include <memory>
#include <string>
#include <iterator>
#include <fstream>
#include <thread>

//include ROOT classes 
#include "TH1D.h"
#include "TFile.h"

//include parts of analysis framework
#include "../Event/interface/Event.h"
#include "../Tools/interface/RangedMap.h"
#include "../Tools/interface/HistInfo.h"
#include "../Tools/interface/stringTools.h"
#include "../Tools/interface/analysisTools.h"
#include "../Tools/interface/systemTools.h"
#include "interface/fakeRateSelection.h"
#include "interface/fakeRateTools.h"
#include "interface/Prescale.h"
#include "../plotting/tdrStyle.h"
#include "../plotting/plotCode.h"
#include "../weights/interface/ConcreteReweighterFactory.h"


HistInfo makeVarHistInfo( const unsigned numberOfBins, const double cut, const double max, const bool useMT = true){
    if( cut >= max ){
        throw std::invalid_argument( "Cut should be smaller than max, while cut is " + std::to_string( cut ) + " and max is " + std::to_string( max ) + "." );
    }

    unsigned numberOfBinsToUse = static_cast< unsigned >( ( max - cut )/max * numberOfBins );
    if( useMT ){
        return HistInfo( "mT", "m_{T} (GeV)", numberOfBinsToUse, cut, max );
    } else {
        return HistInfo( "met", "E_{T}^{miss} (GeV)", numberOfBinsToUse, cut, max );
    }
}



void fillPrescaleMeasurementHistograms( const std::string& year, const std::string& sampleDirectoryPath, const std::vector< std::string >& triggerVector, const bool useMT = true, const double metCut = 0, double mtCut = 0){

    analysisTools::checkYearString( year );

    static constexpr unsigned numberOfBins = 16;
    static constexpr double maxBin = 160;
    HistInfo histInfo;
    if( useMT ){
        histInfo = makeVarHistInfo( numberOfBins, mtCut, maxBin, true );
    } else {
        histInfo = makeVarHistInfo( numberOfBins, metCut, maxBin, false );
    }

    //histograms for data, prompt and nonprompt leptons 
    std::map< std::string, std::shared_ptr< TH1D > > prompt_map;
    std::map< std::string, std::shared_ptr< TH1D > > nonprompt_map;
    std::map< std::string, std::shared_ptr< TH1D > > data_map;
    
    //initialize histograms
    for( const auto& trigger : triggerVector ){
        prompt_map[trigger] = histInfo.makeHist( "prompt_mT_" + year + "_" + trigger );
        nonprompt_map[trigger] = histInfo.makeHist( "nonprompt_mT_" + year + "_" + trigger );
        data_map[trigger] = histInfo.makeHist( "data_mT_" + year + "_" + trigger );
    }

    //set up map of triggers to pt thresholds
    std::map< std::string, double > leptonPtCutMap = fakeRate::mapTriggerToLeptonPtThreshold( triggerVector );
    std::map< std::string, double > jetPtCutMap = fakeRate::mapTriggerToJetPtThreshold( triggerVector );

    //in this function we will loop over events and fill histograms for each trigger 
    TreeReader treeReader( "sampleLists/samples_fakeRateMeasurement_" + year + ".txt", sampleDirectoryPath);

    //build a reweighter for scaling MC events
    std::shared_ptr< ReweighterFactory >reweighterFactory( new EwkinoReweighterFactory() );
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( "../weights/", year, treeReader.sampleVector() );

    for( unsigned sampleIndex = 0; sampleIndex < treeReader.numberOfSamples(); ++sampleIndex ){
	std::cout<<"sample "<<sampleIndex+1<<" of "<<treeReader.numberOfSamples()<<std::endl;

        //load next sample
        treeReader.initSample();

	// reduce number of entries to run on for testing
	unsigned numberOfEntries = (unsigned) treeReader.numberOfEntries()/100.;
	if(treeReader.isData()) numberOfEntries = treeReader.numberOfEntries()/10;
        
	//loop over events in sample
        for( long unsigned entry = 0; entry < numberOfEntries; ++entry ){
	    if(entry%100000==0) std::cout<<"entry "<<entry+1<<" of "<<numberOfEntries<<std::endl; 
            Event event = treeReader.buildEvent( entry, true, false );

	    //apply event selection, note that event is implicitly modified (lepton selection etc)
            //cone-correction is applied
            //select tight leptons for prescale measurement : onlyMuons = false / onlyElectrons = false / onlyTightLeptons = true / requireJet = false
            if( !fakeRate::passFakeRateEventSelection( event, false, false, true, false ) ) continue;

            LightLepton& lepton = event.lightLepton(0);
            double mT = mt( lepton, event.met() );

            if( mT <= mtCut ) continue;
            if( event.metPt() <= metCut ) continue;

            //compute event weight
            double weight = event.weight();
            if( event.isMC() ){
                weight *= reweighter.totalWeight( event );
            }
	    // modification with respect to original code:
	    else{weight = 1;}

            for( const auto& trigger : triggerVector ){

                //event must pass trigger
                if( !event.passTrigger( trigger ) ) continue;

                //check lepton flavor corresponding to this trigger
		if( stringTools::stringContains( trigger, "Mu" ) ){
		    if( !lepton.isMuon() ) continue;
		} else if( stringTools::stringContains( trigger, "Ele" ) ){
		    if( !lepton.isElectron() ) continue;
		} else {
                    throw std::invalid_argument( "Can not measure prescale for trigger " + trigger + " since it is neither a muon nor electron trigger." );
                }

                //apply offline pT threshold to be on the trigger plateau
                if( lepton.uncorrectedPt() <= leptonPtCutMap[ trigger ] ) continue;

                //check if any trigger jet threshold must be applied and apply it
                if( !fakeRate::passTriggerJetSelection( event, trigger, jetPtCutMap ) ) continue;
                
                double valueToFill = ( useMT ? mT : event.metPt() );

                if( event.isData() ){
                    data_map[ trigger ]->Fill( std::min( valueToFill, histInfo.maxBinCenter() ), weight );
                } else {
                    if( lepton.isPrompt() ){
                        prompt_map[ trigger ]->Fill( std::min( valueToFill, histInfo.maxBinCenter() ), weight ); 
                    } else {
                        nonprompt_map[ trigger ]->Fill( std::min( valueToFill, histInfo.maxBinCenter() ), weight );
                    }
                }
            }
        }
    }

    //write histograms to TFile 
    TFile* histogram_file = TFile::Open( ( std::string("prescaleMeasurement_") + ( useMT ? "mT" : "met" ) + "_histograms_" + year + ".root" ).c_str(), "RECREATE" );
    for( const auto& trigger : triggerVector ){
	data_map[ trigger ]->Write();
	prompt_map[ trigger ]->Write();
	nonprompt_map[ trigger ]->Write();
    }
    histogram_file->Close();
}

RangedMap< RangedMap< std::shared_ptr< TH1D > > > build2DHistogramMap( const std::vector< double >& ptBinBorders, const std::vector< double >& etaBinBorders, const HistInfo& mtHistInfo, const std::string& name ){
    
    //ALSO MAKE THIS WORK FOR TH2D
    //construct 2D ranged map of pt/eta ranges to TH1D
    std::map< double, RangedMap< std::shared_ptr< TH1D > > >  histMap2DTemp;
    for( auto ptBinBorder : ptBinBorders ){
        std::map< double, std::shared_ptr< TH1D > > histMapTemp;

        for( auto etaBinBorder : etaBinBorders ){ 
            histMapTemp[etaBinBorder] = mtHistInfo.makeHist( name + "_pT_" + std::to_string( int( ptBinBorder ) ) + "_eta_" + stringTools::replace( stringTools::doubleToString( etaBinBorder, 2 ), ".", "p" ) );
        }
        histMap2DTemp.insert( { ptBinBorder, RangedMap< std::shared_ptr< TH1D > >( histMapTemp ) } );
    }

    return RangedMap< RangedMap< std::shared_ptr< TH1D > > >( histMap2DTemp );
}


void write2DHistogramMap( const RangedMap< RangedMap< std::shared_ptr< TH1D > > >& histMap ){
    for( auto& map_pair : histMap ){
        for( auto& hist_pair : map_pair.second ){
            hist_pair.second->Write();
        }
    }
}


//function to fill MT shape histogram for numerator and denominator
void fillFakeRateMeasurementHistograms( const std::string& leptonFlavor, const std::string& year, const std::string& sampleDirectory, const std::vector< std::string >& triggerVector, const std::map< std::string, Prescale >& prescaleMap, double maxMT, double maxMet){

    //make sure year and leptonFlavor are OK
    fakeRate::checkFlavorString( leptonFlavor );
    analysisTools::checkYearString( year );

    //make sure we have prescale values for each trigger 
    for( const auto& trigger : triggerVector ){
        if( prescaleMap.find( trigger ) == prescaleMap.end() ){
            throw std::invalid_argument( "Given vector of triggers contains triggers that are not present in the given prescale map." );
        }
    }

    bool isMuonMeasurement = ( leptonFlavor == "muon" );
    
    //define bin borders 
    std::vector< double > ptBinBorders;
    std::vector< double > etaBinBorders;

    //MODIFY THIS BINNING AS NEEDED
    if( isMuonMeasurement ){
        etaBinBorders = {0., 1.2, 2.1};
        ptBinBorders = { 10, 20, 30, 45, 65 };
    } else{
        etaBinBorders = {0., 0.8, 1.442};
        ptBinBorders = { 10, 20, 30, 45, 65 };
    }

    //MAKE THE NUMBER OF MT OR MET BINS ADAPTIVE
    unsigned numberOfMTBins = 100; 
    HistInfo mtHistInfo( "mT", "m_{T}( GeV )", numberOfMTBins, 0., 160. );

    //make 2D RangedMap mapping each bin to a histogram
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > prompt_numerator_map = build2DHistogramMap( ptBinBorders, etaBinBorders, mtHistInfo, "prompt_numerator_mT_" + year + "_" + leptonFlavor );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > prompt_denominator_map = build2DHistogramMap( ptBinBorders, etaBinBorders, mtHistInfo, "prompt_denominator_mT_" + year + "_" + leptonFlavor );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > nonprompt_numerator_map = build2DHistogramMap( ptBinBorders, etaBinBorders, mtHistInfo, "nonprompt_numerator_mT_" + year + "_" + leptonFlavor );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > nonprompt_denominator_map = build2DHistogramMap( ptBinBorders, etaBinBorders, mtHistInfo, "nonprompt_denominator_mT_" + year + "_" + leptonFlavor );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > data_numerator_map = build2DHistogramMap( ptBinBorders, etaBinBorders, mtHistInfo, "data_numerator_mT_" + year + "_" + leptonFlavor);
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > data_denominator_map = build2DHistogramMap( ptBinBorders, etaBinBorders, mtHistInfo, "data_denominator_mT_" + year + "_" + leptonFlavor );


    //map lepton pT to trigger 
    RangedMap< std::string > leptonPtToTriggerMap = fakeRate::mapLeptonPtToTriggerName( triggerVector, isMuonMeasurement );

    //map trigger to lepton and jet pT thresholds
    std::map< std::string, double > triggerToLeptonPtMap = fakeRate::mapTriggerToLeptonPtThreshold( triggerVector );
    std::map< std::string, double > triggerToJetPtMap = fakeRate::mapTriggerToJetPtThreshold( triggerVector );

    //initialize TreeReader
    TreeReader treeReader( "sampleLists/samples_fakeRateMeasurement_" + year + ".txt" , sampleDirectory );

    //build a reweighter for scaling MC events
    std::shared_ptr< ReweighterFactory >reweighterFactory( new EwkinoReweighterFactory() );
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( "../weights/", year, treeReader.sampleVector() );

    for( unsigned sampleIndex = 0; sampleIndex < treeReader.numberOfSamples(); ++sampleIndex ){
	std::cout<<"sample "<<sampleIndex+1<<" of "<<treeReader.numberOfSamples()<<std::endl;

        //load next sample
        treeReader.initSample();

	// reduce number of entries in loop for testing
	unsigned numberOfEntries = (unsigned) treeReader.numberOfEntries()/100.;
	if(treeReader.isData()) numberOfEntries = treeReader.numberOfEntries()/10.;
	
        //loop over events in sample
        for( long unsigned entry = 0; entry < numberOfEntries; ++entry ){
	    if(entry%100000==0) std::cout<<"entry "<<entry+1<<" of "<<numberOfEntries<<std::endl;

            Event event = treeReader.buildEvent( entry, true, false );

            //apply event selection, note that event is implicitly modified (lepton selection etc)
            if( !fakeRate::passFakeRateEventSelection( event, isMuonMeasurement, !isMuonMeasurement, false, true, 1 ) ) continue;

            LightLepton& lepton = event.lightLepton( 0 );
            
            //apply cone-corrected pT cut
            if( lepton.pt() < 10 ) continue;

            //compute mT for event binning
            double mT = mt( lepton, event.met() );

            //apply upper cuts on mT  and met 
            if( mT >= maxMT ) continue;
            if( event.metPt() >= maxMet ) continue;

            //IMPORTANT : apply correct trigger
            //if( !fakeRate::passFakeRateTrigger( event, leptonPtToTriggerMap ) ) continue;
            std::string triggerToUse = leptonPtToTriggerMap[ lepton.uncorrectedPt() ];
            if( !event.passTrigger( triggerToUse ) ) continue;

            //check if any trigger jet threshold must be applied and apply it
            if( !fakeRate::passTriggerJetSelection( event, triggerToUse, triggerToJetPtMap ) ) continue;

            //set correct weight corresponding to this trigger prescale for MC
            double weight = event.weight();
            if( event.isMC() ){
                const Prescale& prescale = prescaleMap.find( triggerToUse )->second;
                weight *= prescale.value();
                weight *= reweighter.totalWeight( event );
            }
	    // modification with respect to original code
	    else{ weight=1;}

            //fill numerator
            if( lepton.isTight() ){
                if( treeReader.isData() ){
                    data_numerator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( std::min( mT, mtHistInfo.maxBinCenter() ), weight );
                } else if( lepton.isPrompt() ){
                    prompt_numerator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( std::min( mT, mtHistInfo.maxBinCenter() ), weight );
                } else {
                    nonprompt_numerator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( std::min( mT, mtHistInfo.maxBinCenter() ), weight );
                }
            }

            //fill denominator
            if( treeReader.isData() ){
                data_denominator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( std::min( mT, mtHistInfo.maxBinCenter() ), weight );
            } else if( lepton.isPrompt() ){
                prompt_denominator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( std::min( mT, mtHistInfo.maxBinCenter() ), weight );
            } else {
                nonprompt_denominator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( std::min( mT, mtHistInfo.maxBinCenter() ), weight );
            }
        }
    }

    //write histograms to TFile 
    TFile* histogram_file = TFile::Open( ( "fakeRateMeasurement_" + leptonFlavor + "_" + year + "_mT_histograms.root" ).c_str(), "RECREATE" );
    
    write2DHistogramMap( prompt_numerator_map );
    write2DHistogramMap( prompt_denominator_map );
    write2DHistogramMap( nonprompt_numerator_map );
    write2DHistogramMap( nonprompt_denominator_map );
    write2DHistogramMap( data_numerator_map );
    write2DHistogramMap( data_denominator_map );

    histogram_file->Close();
}

int main( int argc, char* argv[] ){

    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    
    //either take 2 command line arguments, flavor and year, or none, in which case everything will be run
    if( !( argvStr.size() == 3 || argvStr.size() == 1 ) ){
        std::cerr << argc - 1 << " command line arguments given, while 2 or 0 are expected." << std::endl;
        std::cerr << "Usage ( to measure fake-rate for all flavors and years ): ./fakeRateMeasurement" << std::endl;
        std::cerr << "Or ( to measure fake-rate a single flavor and year ): ./fakeRateMeasurement flavor year" << std::endl;
        return 1;
    }

    //configuration
    const double metLowerCut_prescaleMeasurement = 20;
    const double mTLowerCut_prescaleMeasurement = 0;
    const double mTLowerCut_prescaleFit = 80;
    const double mTUpperCut_prescaleFit = 130;
    const double metUpperCut_fakeRateMeasurement = 20;
    const double mTUpperCut_fakeRateMeasurement = 160;
    const double maxFitValue = 20;
    const bool use_mT = true;
    const std::string& sampleDirectory = "/pnfs/iihe/cms/store/user/wverbeke/ntuples_ewkino_fakerate";

    std::vector< std::string > years;
    std::vector< std::string > flavors;

    //run for all flavors and years 
    if( argvStr.size() == 1 ){
        years = { "2016", "2017", "2018" };
        flavors = { "muon", "electron" };

    //run for one flavor and year
    } else{
        years = { argvStr[2] };
        flavors = { argvStr[1] };
    }

    //trigger to use in fake-rate measurement 
    std::map< std::string, std::vector< std::string > > triggerVectorMap = {
        { "2016", std::vector< std::string >( {"HLT_Mu3_PFJet40", "HLT_Mu8", "HLT_Mu17", "HLT_Mu20", "HLT_Mu27", "HLT_Ele8_CaloIdM_TrackIdM_PFJet30", "HLT_Ele12_CaloIdM_TrackIdM_PFJet30", "HLT_Ele17_CaloIdM_TrackIdM_PFJet30", "HLT_Ele23_CaloIdM_TrackIdM_PFJet30" } ) },
        { "2017", std::vector< std::string >( { "HLT_Mu3_PFJet40", "HLT_Mu8", "HLT_Mu17", "HLT_Mu20", "HLT_Mu27", "HLT_Ele8_CaloIdM_TrackIdM_PFJet30", "HLT_Ele17_CaloIdM_TrackIdM_PFJet30", "HLT_Ele23_CaloIdM_TrackIdM_PFJet30" } ) },
        { "2018", std::vector< std::string >( { "HLT_Mu3_PFJet40", "HLT_Mu8", "HLT_Mu17", "HLT_Mu20", "HLT_Mu27", "HLT_Ele8_CaloIdM_TrackIdM_PFJet30", "HLT_Ele17_CaloIdM_TrackIdM_PFJet30", "HLT_Ele23_CaloIdM_TrackIdM_PFJet30" } ) }
    };

	//make sure ROOT behaves itself when running multithreaded
    ROOT::EnableThreadSafety();

    //set histogram style
    setTDRStyle();

    //fill all prescale histograms in a multithreaded manner
    std::vector< std::thread > threadVector_prescale;
    threadVector_prescale.reserve( 3 );
    for( const auto& year : years ){
        threadVector_prescale.emplace_back( fillPrescaleMeasurementHistograms, year, sampleDirectory, triggerVectorMap[ year ], use_mT, metLowerCut_prescaleMeasurement, mTLowerCut_prescaleMeasurement );
    }

    //join the threads
    for( auto& t : threadVector_prescale ){
        t.join();
    }
    
    //map years to prescale measurement map
    std::map< std::string, std::map< std::string, Prescale > > prescaleMaps;
    for( const auto& year : years ){
        std::string file_name = std::string( "prescaleMeasurement_" ) + ( use_mT ? "mT" : "met" ) + "_histograms_" + year + ".root";
        TFile* prescale_filePtr = TFile::Open( file_name.c_str() );
        prescaleMaps[ year ] = fakeRate::fitTriggerPrescales_cut( prescale_filePtr, mTLowerCut_prescaleFit, mTUpperCut_prescaleFit );
        prescale_filePtr->Close();
    }

    //fill all fake-rate measurement histograms in a multithreaded manner 
    //use cuts on MET and MT ( though the cut on the fitting variable can also be set later on in the fit )
    std::vector< std::thread > threadVector_measurement;
    threadVector_measurement.reserve( 6 );
    for( const auto& year : years ){
        for( const auto& flavor : flavors ){
            threadVector_measurement.emplace_back( fillFakeRateMeasurementHistograms, flavor, year, sampleDirectory, triggerVectorMap[ year ], prescaleMaps[ year ], mTUpperCut_fakeRateMeasurement, metUpperCut_fakeRateMeasurement );
        }
    }

    //join the threads
    for( auto& t : threadVector_measurement ){
        t.join();
    }

    //make output directory if it did not already exist
    systemTools::makeDirectory( "fakeRateMaps" );

    //plot fake-rate maps and write them to a root file
    for( const auto& year : years ){
        for( const auto& flavor : flavors ){

            std::string instanceName = flavor + "_" + year + "_" + ( use_mT ? "mT" : "met" );
            
            //measure the fake-rate, using bins up to the specified cut in the fitting variable
            TFile* measurement_filePtr = TFile::Open( ( "fakeRateMeasurement_" + instanceName + "_histograms.root" ).c_str() );
            std::shared_ptr< TH2D > frMap = fakeRate::produceFakeRateMap_cut( measurement_filePtr, maxFitValue );
            measurement_filePtr->Close();

            //plot the fake-rate measurement
            plot2DHistogram( frMap.get(), ( "fakeRateMaps/fakeRateMap_" + instanceName + ".pdf").c_str() );

            //write fake-rate measurement to file
            TFile* writeFile = TFile::Open( ( "fakeRateMaps/fakeRateMap_data_" + instanceName + ".root" ).c_str(), "RECREATE" );
            frMap->Write( ("fakeRate_" + flavor + "_" + year ).c_str() );
            writeFile->Close();
        }
    }
    return 0;
}


