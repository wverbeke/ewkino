

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
#include "../weights/interface/ConcreteLeptonReweighter.h"
#include "../weights/interface/ConcreteReweighterLeptons.h"
#include "../weights/interface/ConcreteSelection.h"




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

        //load next sample
        treeReader.initSample();

        //loop over events in sample
        for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){
            Event event = treeReader.buildEvent( entry, true, false );

	    //apply event selection, note that event is implicitly modified (lepton selection etc)
            //cone-correction is applied
            //select tight leptons for prescale measurement : onlyMuons = false / onlyElectrons = false / onlyTightLeptons = true / requireJet = false
            if( !fakeRate::passFakeRateEventSelection( event, false, false, true, true, 0.7, 40 ) ) continue;

            LightLepton& lepton = event.lightLepton(0);
            double mT = mt( lepton, event.met() );

            if( mT <= mtCut ) continue;
            if( event.metPt() <= metCut ) continue;

            if( mT > maxBin ) continue;

            //compute event weight
            double weight = event.weight();
            if( event.isMC() ){
                weight *= reweighter.totalWeight( event );
            }

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
                /*
                event.selectGoodJets();
                event.cleanJetsFromLooseLeptons();
                if( event.numberOfJets() < 1 ) continue;
                event.sortJetsByPt();
                if( event.jet(0).pt() < 40 ) continue;
                */
                
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


/*std::shared_ptr< Reweighter > makeLeptonReweighter( const std::string& year, const bool isMuon, const bool isFO){
    std::string flavorString = ( isMuon ? "m" : "e" );
    std::string wpString = ( isFO ? "FO" : "3lTight" );
    const std::string weightDirectory = "../weights/";
    TFile* leptonSFFile = TFile::Open( ( stringTools::formatDirectoryName( weightDirectory ) + "weightFiles/leptonSF/leptonSF_" + flavorString + "_" + year + "_" + wpString + ".root" ).c_str() );
    std::shared_ptr< TH2 > leptonSFHist( dynamic_cast< TH2* >( leptonSFFile->Get( "EGamma_SF2D" ) ) );
    leptonSFHist->SetDirectory( gROOT );
    leptonSFFile->Close();

    if( isMuon ){
        if( isFO ){
            MuonReweighter muonReweighter( leptonSFHist, new FOSelector );
            return std::make_shared< ReweighterMuons >( muonReweighter );
        } else {
            MuonReweighter muonReweighter( leptonSFHist, new TightSelector );
            return std::make_shared< ReweighterMuons >( muonReweighter );
        }
    } else {
        if( isFO ){
            ElectronIDReweighter electronIDReweighter( leptonSFHist, new FOSelector );
            return std::make_shared< ReweighterElectronsID >( electronIDReweighter );
        } else {
            ElectronIDReweighter electronIDReweighter( leptonSFHist, new TightSelector );
            return std::make_shared< ReweighterElectronsID >( electronIDReweighter );
        }
        return nullptr;
    }
}*/


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
    unsigned numberOfMTBins = 16; 
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


    //map cone pT to trigger
    std::map< double, std::string > conePtLowerBoundMap;
    for( auto it = leptonPtToTriggerMap.cbegin(); it != leptonPtToTriggerMap.cend(); ++it ){
        double conePtBound;
        if( it == leptonPtToTriggerMap.cbegin() ){
            conePtBound = it->first;
        } else {
            if( isMuonMeasurement ){
                conePtBound = 2*it->first;
            } else {
                conePtBound = 1.5*it->first;
            }
        }
        conePtLowerBoundMap[ conePtBound ] = it->second;
    }
    RangedMap< std::string > conePtToTriggerMap( conePtLowerBoundMap );


    //map trigger to lepton and jet pT thresholds
    //std::map< std::string, double > triggerToLeptonPtMap = fakeRate::mapTriggerToLeptonPtThreshold( triggerVector );
    std::map< std::string, double > triggerToJetPtMap = fakeRate::mapTriggerToJetPtThreshold( triggerVector );

    //initialize TreeReader
    TreeReader treeReader( "sampleLists/samples_fakeRateMeasurement_" + year + ".txt" , sampleDirectory );

    //build a reweighter for scaling MC events
    std::shared_ptr< ReweighterFactory >reweighterFactory( new EwkinoReweighterFactory() );
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( "../weights/", year, treeReader.sampleVector() );

    //remove lepton weights from reweighter so a separate FO and tight reweighter can be made
    //reweighter.eraseReweighter( "muonID" );
    //reweighter.eraseReweighter( "electronID" );

    //make FO and tight reweighters
    //std::shared_ptr< Reweighter > FOReweighter = makeLeptonReweighter( year, isMuonMeasurement, true );
    //std::shared_ptr< Reweighter > tightReweighter = makeLeptonReweighter( year, isMuonMeasurement, false );

    for( unsigned sampleIndex = 0; sampleIndex < treeReader.numberOfSamples(); ++sampleIndex ){

        //load next sample
        treeReader.initSample();

        //loop over events in sample
        for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){
            Event event = treeReader.buildEvent( entry, true, false );

            //apply event selection, note that event is implicitly modified (lepton selection etc)
            if( !fakeRate::passFakeRateEventSelection( event, isMuonMeasurement, !isMuonMeasurement, false, true, 0.7, 30 ) ) continue;

            LightLepton& lepton = event.lightLepton( 0 );
            
            //apply cone-corrected pT cut
            if( lepton.pt() < 10 ) continue;

            //compute mT for event binning
            //use fixed lepton pT 'mTFix to avoid correlation between mT and lepton pT when measuring the fake-rate
            //double mT = mt( lepton, event.met() );
            const double pTFix = 35.;
            PhysicsObject leptonFix( pTFix, lepton.eta(), lepton.phi(), lepton.energy() );
            double mT = mt( leptonFix, event.met() );

            //apply upper cuts on mT  and met 
            if( mT >= maxMT ) continue;
            if( event.metPt() >= maxMet ) continue;


            //IMPORTANT : apply correct trigger
            //if( !fakeRate::passFakeRateTrigger( event, leptonPtToTriggerMap ) ) continue;
            //std::string triggerToUse = leptonPtToTriggerMap[ lepton.uncorrectedPt() ];
            std::string triggerToUse = conePtToTriggerMap[ lepton.pt() ];
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

            //fill numerator
            if( lepton.isTight() ){
                double tightWeight = 1.;
                //if( event.isMC() ){
                //    tightWeight = tightReweighter->weight( event );
                //}
                if( treeReader.isData() ){
                    data_numerator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( std::min( mT, mtHistInfo.maxBinCenter() ), weight );
                } else if( lepton.isPrompt() ){
                    prompt_numerator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( std::min( mT, mtHistInfo.maxBinCenter() ), weight*tightWeight );
                } else {
                    nonprompt_numerator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( std::min( mT, mtHistInfo.maxBinCenter() ), weight*tightWeight );
                }
            }

            //add FO weight after numerator has been filled
            //if( event.isMC() ){
            //    weight *= FOReweighter->weight( event );    
            //}

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
    const double metLowerCut_prescaleMeasurement = 40;
    const double mTLowerCut_prescaleMeasurement = 0;
    const double mTLowerCut_prescaleFit = 90;
    const double mTUpperCut_prescaleFit = 130;
    const double metUpperCut_fakeRateMeasurement = 20;
    const double mTUpperCut_fakeRateMeasurement = 160;
    const double maxFitValue = 40;
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
