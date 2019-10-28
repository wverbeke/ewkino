

//include c++ library classes 
#include <vector>
#include <utility>
#include <memory>
#include <string>
#include <iterator>
#include <fstream>

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

    fakeRate::checkYearString( year );

    static constexpr unsigned numberOfBins = 80;
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
        prompt_map[trigger] = histInfo.makeHist( "prompt_mT_" + trigger );
        nonprompt_map[trigger] = histInfo.makeHist( "nonprompt_mT_" + trigger );
        data_map[trigger] = histInfo.makeHist( "data_mT_" + trigger );
    }

    //set up map of triggers to pt thresholds
    std::map< std::string, double > leptonPtCutMap = fakeRate::mapTriggerToLeptonPtThreshold( triggerVector );
    std::map< std::string, double > jetPtCutMap = fakeRate::mapTriggerToJetPtThreshold( triggerVector );

	//in this function we will loop over events and fill histograms for each trigger 
   	TreeReader treeReader( "samples_fakeRateMeasurement_" + year + ".txt", sampleDirectoryPath);

    for( unsigned sampleIndex = 0; sampleIndex < treeReader.numberOfSamples(); ++sampleIndex ){

        //load next sample
        treeReader.initSample();

        //loop over events in sample
        for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){
            Event event = treeReader.buildEvent( entry, true, true);

			//apply event selection, note that event is implicitly modified (lepton selection etc)
            //cone-correction is applied
            if( !fakeRate::passFakeRateEventSelection( event, false, false, false, false) ) continue;

            LightLepton& lepton = event.lightLepton(0);
            double mT = mt( lepton, event.met() );

            if( mT < mtCut ) continue;
            if( event.metPt() < metCut ) continue;

            for( const auto& trigger : triggerVector ){

                //check lepton flavor corresponding to this trigger
				if( stringTools::stringContains( trigger, "Mu" ) ){
					if( !lepton.isMuon() ) continue;
				} else if( stringTools::stringContains( trigger, "Ele" ) ){
					if( !lepton.isElectron() ) continue;
				} else {
                    throw std::invalid_argument( "Can not measure prescale for trigger " + trigger + " since it is neither a muon nor electron trigger." );
                }

                //event must pass trigger
                if( !event.passTrigger( trigger ) ) continue;

                //apply offline pT threshold to be on the trigger plateau
                if( lepton.uncorrectedPt() <= leptonPtCutMap[ trigger ] ) continue;

                //check if any trigger jet threshold must be applied and apply it
                if( !fakeRate::passTriggerJetSelection( event, trigger, jetPtCutMap ) ) continue;
                
                double valueToFill = ( useMT ? mT : event.metPt() );
                if( treeReader.isData() ){
                    data_map[trigger]->Fill( std::min( valueToFill, histInfo.maxBinCenter() ), event.weight() );
                } else {
                    if( lepton.isPrompt() ){
                        prompt_map[trigger]->Fill( std::min( valueToFill, histInfo.maxBinCenter() ), event.weight() ); 
                    } else {
                        nonprompt_map[trigger]->Fill( std::min( valueToFill, histInfo.maxBinCenter() ), event.weight() );
                    }
                }
            }
        }
    }

	//write histograms to TFile 
    TFile* histogram_file = TFile::Open( ( std::string("prescaleMeasurement_") + ( useMT ? "mT" : "met" ) + "_histograms_" + year + ".root" ).c_str(), "RECREATE" );
	for( const auto& trigger : triggerVector ){
		data_map[trigger]->Write();
		prompt_map[trigger]->Write();
		nonprompt_map[trigger]->Write();
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
            histMapTemp[etaBinBorder] = mtHistInfo.makeHist( name + "_pT_" + std::to_string( int( ptBinBorder ) ) + "_eta_" + stringTools::doubleToString( etaBinBorder, 2 ) );
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
void fillFakeRateMeasurementHistograms( const std::string& leptonFlavor, const std::string& year, const std::string& sampleDirectory, const std::vector< std::string >& triggerVector, std::map< std::string, Prescale >& prescaleMap, double maxMT, double maxMet){

    //make sure year and leptonFlavor are OK
    fakeRate::checkFlavorString( leptonFlavor );
    fakeRate::checkYearString( year );

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
        ptBinBorders = { 0, 10, 20, 30, 45, 65 };
    } else{
        etaBinBorders = {0., 0.8, 1.442};
        ptBinBorders = { 0, 10, 20, 30, 45, 65 };
    }

    //MAKE THE NUMBER OF MT OR MET BINS ADAPTIVE
    unsigned numberOfMTBins = 100; 
    HistInfo mtHistInfo( "mT", "m_{T}( GeV )", numberOfMTBins, 0, 200 );

    //make 2D RangedMap mapping each bin to a histogram
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > prompt_numerator_map = build2DHistogramMap( ptBinBorders, etaBinBorders, mtHistInfo, "prompt_numerator_mT_" + leptonFlavor );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > prompt_denominator_map = build2DHistogramMap( ptBinBorders, etaBinBorders, mtHistInfo, "prompt_denominator_mT_" + leptonFlavor );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > nonprompt_numerator_map = build2DHistogramMap( ptBinBorders, etaBinBorders, mtHistInfo, "nonprompt_numerator_mT_" + leptonFlavor );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > nonprompt_denominator_map = build2DHistogramMap( ptBinBorders, etaBinBorders, mtHistInfo, "nonprompt_denominator_mT_" + leptonFlavor );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > data_numerator_map = build2DHistogramMap( ptBinBorders, etaBinBorders, mtHistInfo, "data_numerator_mT_" + leptonFlavor);
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > data_denominator_map = build2DHistogramMap( ptBinBorders, etaBinBorders, mtHistInfo, "data_denominator_mT_" + leptonFlavor );


    //map lepton pT to trigger 
    RangedMap< std::string > leptonPtToTriggerMap = fakeRate::mapLeptonPtToTriggerName( triggerVector, isMuonMeasurement );

    //map trigger to lepton and jet pT thresholds
    std::map< std::string, double > triggerToLeptonPtMap = fakeRate::mapTriggerToLeptonPtThreshold( triggerVector );
    std::map< std::string, double > triggerToJetPtMap = fakeRate::mapTriggerToJetPtThreshold( triggerVector );

    //initialize TreeReader
    //ADAPT SAMPLE LIST TO USE DATA FOR CORRECT YEAR
    TreeReader treeReader( "samples_fakeRateMeasurement_" + year + ".txt" , sampleDirectory );
    for( unsigned sampleIndex = 0; sampleIndex < treeReader.numberOfSamples(); ++sampleIndex ){

        //load next sample
        treeReader.initSample();

        //loop over events in sample
        for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){
            Event event = treeReader.buildEvent( entry, true, true );

            //apply event selection, note that event is implicitly modified (lepton selection etc)
            if( !fakeRate::passFakeRateEventSelection( event, isMuonMeasurement, !isMuonMeasurement, false, true, 1 ) ) continue;

            LightLepton& lepton = event.lightLepton( 0 );

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

            //set correct weight corresponding to this trigger prescale 
            double weight = event.weight()*prescaleMap[ triggerToUse ].value();

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

    std::vector< std::string > triggerVector = { "HLT_Mu3_PFJet40", "HLT_Mu8", "HLT_Mu17", "HLT_Mu20", "HLT_Mu27", "HLT_Mu50", "HLT_Ele8_CaloIdL_TrackIdL_IsoVL_PFJet30", "HLT_Ele17_CaloIdM_TrackIdM_PFJet30", "HLT_Ele23_CaloIdM_TrackIdM_PFJet30"};

    //fillPrescaleMeasurementHistograms( "2016", "../test/testData/", triggerVector, true, 0, 20 );
    //fillPrescaleMeasurementHistograms( "2016", "../test/testData/", triggerVector, false, 0, 20 );

    TFile* filePtr = TFile::Open( "prescaleMeasurement_mT_histograms_2016.root" );

    std::map< std::string, Prescale > prescaleMap = fakeRate::fitTriggerPrescales_cut( filePtr, 80, 160);
    filePtr->Close();

    /*
    fillFakeRateMeasurementHistograms( "muon", "2016", "../test/testData/", triggerVector, prescaleMap, 20, 20 );
    fillFakeRateMeasurementHistograms( "electron", "2016", "../test/testData/", triggerVector, prescaleMap, 20, 20 );


    filePtr = TFile::Open( "fakeRateMeasurement_muon_2016_mT_histograms.root" );
    std::shared_ptr< TH2D > frMap = fakeRate::produceFakeRateMap_cut( filePtr );
    filePtr->Close();

    TFile* writeFile = TFile::Open( "test.root", "RECREATE" );
    frMap->Write("fake-rate-test" );
    writeFile->Close();
    */
    
    


    return 0;
}


