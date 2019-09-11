

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


RangedMap< RangedMap< std::shared_ptr< TH1D > > > build2DHistogramMap( const std::vector< double >& ptBinBorders, const std::vector< double >& etaBinBorders, const HistInfo& mtHistInfo, const std::string& name ){
    
    //ALSO MAKE THIS WORK FOR TH2D
    //construct 2D ranged map of pt/eta ranges to TH1D
    std::map< double, RangedMap< std::shared_ptr< TH1D > > >  histMap2DTemp;
    for( auto ptBinBorder : ptBinBorders ){
        std::map< double, std::shared_ptr< TH1D > > histMapTemp;

        for( auto etaBinBorder : etaBinBorders ){ 
            histMapTemp[etaBinBorder] = mtHistInfo.makeHist( name + "_pT_" + std::to_string( int( ptBinBorder ) ) + "_eta_" + stringTools::doubleToString( etaBinBorder, 2 ) );

            //make sure statistical uncertainties are correctly stored 
            histMapTemp[etaBinBorder]->Sumw2();
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


void checkLeptonFlavorString( const std::string& leptonFlavor ){
    if( !( ( leptonFlavor == "muon" ) || ( leptonFlavor == "electron" ) ) ){
        throw std::invalid_argument( "leptonFlavor string should be either 'muon' or 'electron'." );
    }
}


void checkYearString( const std::string& year ){
    if( !( ( year == "2016" ) || ( year == "2017" ) || ( year == "2018" ) ) ){
        throw std::invalid_argument( "year string should be either '2016', '2017' or '2018'." );
    }
}


//function to determine prescale of each trigger
//this will be done by determining the prompt normalization in a fit to mT
//std::map< std::string, double > measurePrescales(){

void fillPrescaleMeasurementHistograms(){
    static std::vector< std::string > triggerVector = { "HLT_Mu3_PFJet40", "HLT_Mu8", "HLT_Mu17", "HLT_Mu20", "HLT_Mu27", "HLT_Mu50", "HLT_Ele17_CaloIdM_TrackIdM_PFJet30", "HLT_Ele23_CaloIdM_TrackIdM_PFJet30"};

    unsigned numberOfMTBins = 100;
    HistInfo mtHistInfo( "mT", "m_{T}( GeV )", numberOfMTBins, 0, 200 );
    std::map< std::string, std::shared_ptr< TH1D > > prompt_map;
    std::map< std::string, std::shared_ptr< TH1D > > nonprompt_map;
    std::map< std::string, std::shared_ptr< TH1D > > data_map;
    
    //initialize histograms
    for( const auto& trigger : triggerVector ){
        prompt_map[trigger] = mtHistInfo.makeHist( "prompt_mT_" + trigger );
        nonprompt_map[trigger] = mtHistInfo.makeHist( "nonprompt_mT_" + trigger );
        data_map[trigger] = mtHistInfo.makeHist( "data_mT_" + trigger );
    }
    

	//loop over events and fill  histograms for each trigger 
   	TreeReader treeReader( "samples_fakeRateMeasurement.txt", "../test/testData");
    for( unsigned sampleIndex = 0; sampleIndex < treeReader.numberOfSamples(); ++sampleIndex ){

        //load next sample
        treeReader.initSample();

        //loop over events in sample
        for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){
            Event event = treeReader.buildEvent( entry, true, true);

			//apply event selection, note that event is implicitly modified (lepton selection etc)
            if( !passFakeRateEventSelection( event, true, false ) ) continue;

            Lepton& lepton = event.lepton(0);
            double mT = mt( lepton, event.met() );

            for( const auto& trigger : triggerVector ){

                //check lepton flavor corresponding to this trigger
				if( stringTools::stringContains( trigger, "Mu" ) ){
					if( !lepton.isMuon() ) continue;
				} else {
					if( !lepton.isElectron() ) continue;
				}

                //event must pass trigger
                if( !event.passTrigger( trigger ) ) continue;

                //apply offline pT threshold equal to that of the trigger 
                if( lepton.pt() <= triggerLeptonPtCut( trigger ) ) continue;

                //check if any trigger jet threshold must be applied
                if( stringTools::stringContains( trigger, "PFJet" ) ){
                    event.selectGoodJets();
                    if( event.numberOfJets() < 1 ) continue;
                    event.sortJetsByPt();
                    if( event.jet(0).pt() <= triggerJetPtCut( trigger ) ) continue;
                }
                
                if( treeReader.isData() ){
                    data_map[trigger]->Fill( std::min( mT, mtHistInfo.maxBinCenter() ), event.weight() );
                } else {
                    if( lepton.isPrompt() ){
                        prompt_map[trigger]->Fill( std::min( mT, mtHistInfo.maxBinCenter() ), event.weight() ); 
                    } else {
                        nonprompt_map[trigger]->Fill( std::min( mT, mtHistInfo.maxBinCenter() ), event.weight() );
                    }
                }
            }
        }
    }

	//write histograms to TFile 
    TFile* histogram_file = TFile::Open( ( std::string("prescaleMeasurement_mT_histograms.root") ).c_str(), "RECREATE" );
	for( const auto& trigger : triggerVector ){
		data_map[trigger]->Write();
		prompt_map[trigger]->Write();
		nonprompt_map[trigger]->Write();
	}
    histogram_file->Close();
}


//function to fill MT shape histogram for numerator and denominator
void fillFakeRateMeasurementHistograms( const std::string& leptonFlavor, const std::string& year ){

    //make sure year and leptonFlavor are OK
    checkLeptonFlavorString( leptonFlavor );
    checkYearString( year );

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

    //initialize TreeReader
    //ADAPT SAMPLE LIST TO USE DATA FOR CORRECT YEAR
    TreeReader treeReader( "samples_fakeRateMeasurement.txt", "../test/testData");
    for( unsigned sampleIndex = 0; sampleIndex < treeReader.numberOfSamples(); ++sampleIndex ){

        //load next sample
        treeReader.initSample();

        //loop over events in sample
        for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){
            Event event = treeReader.buildEvent( entry, true, true );

			//apply event selection, note that event is implicitly modified (lepton selection etc)
			if( !passFakeRateEventSelection( event, isMuonMeasurement, false, true, 1 ) ) continue;
			Lepton& lepton = event.lepton( 0 );

            //IMPORTANT : apply correct trigger
            if( !passFakeRateTrigger( event ) ) continue;

            //compute mT for event binning
            double mT = mt( lepton, event.met() );

            //fill numerator
            if( lepton.isTight() ){
                if( treeReader.isData() ){
                    data_numerator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( std::min( mT, mtHistInfo.maxBinCenter() ), event.weight() );
                } else if( lepton.isPrompt() ){
                    prompt_numerator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( std::min( mT, mtHistInfo.maxBinCenter() ), event.weight() );
                } else {
                    nonprompt_numerator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( std::min( mT, mtHistInfo.maxBinCenter() ), event.weight() );
                }
            }

            //fill denominator
            if( treeReader.isData() ){
                data_denominator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( std::min( mT, mtHistInfo.maxBinCenter() ), event.weight() );
            } else if( lepton.isPrompt() ){
                prompt_denominator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( std::min( mT, mtHistInfo.maxBinCenter() ), event.weight() );
            } else {
                nonprompt_denominator_map[ lepton.pt() ][ lepton.absEta() ]->Fill( std::min( mT, mtHistInfo.maxBinCenter() ), event.weight() );
            }
        }
    }

    //write histograms to TFile 
    TFile* histogram_file = TFile::Open( ( leptonFlavor + "_" + year + "_fakeRate_mT_histograms.root" ).c_str(), "RECREATE" );
    
    write2DHistogramMap( prompt_numerator_map );
    write2DHistogramMap( prompt_denominator_map );
    write2DHistogramMap( nonprompt_numerator_map );
    write2DHistogramMap( nonprompt_denominator_map );
    write2DHistogramMap( data_numerator_map );
    write2DHistogramMap( data_denominator_map );

    histogram_file->Close();
}


std::vector< std::string > listHistogramsInFile( const TFile* filePtr ){
    std::vector< std::string > ret;
    for( const auto & key : *filePtr->GetListOfKeys() ){
        ret.push_back( key->GetName() );
    }
    return ret;
}


std::string extractPtEtaString( const std::string& histName ){

    //string is expected to end in _pT_x_eta_y
    //find last occurence of pT and cut off string there
    auto pos = histName.find_last_of( "pT" );
    return histName.substr( pos - 1, histName.size() );
}


std::vector< std::string > histogramListToPtEtaBinList( const std::vector< std::string >& histogramNameList ){
    std::set< std::string > binList;
    for( const auto& histName : histogramNameList ){
       binList.insert( extractPtEtaString( histName ) );
    }
    return std::vector< std::string >( binList.cbegin(), binList.cend() );
}


//list all pt and eta bins that are present in a file with all the fake rate histograms
std::vector< std::string > listPtEtaBinsInFile( const TFile* filePtr ){
    std::vector< std::string > histogramNameList = listHistogramsInFile( filePtr );
    return histogramListToPtEtaBinList( histogramNameList );
}


void printDataCard( TFile* filePtr, const std::string& identifier, const bool promptSignal ){

	//each datacard should contain the three following processes
	static std::vector< std::string > processNames = { "data", "prompt", "nonprompt" };

	//map each process to a histogram
	std::map< std::string, std::shared_ptr< TH1D > > processHistogramMap;
	for( auto& p : processNames ){
	    processHistogramMap[p] = std::shared_ptr< TH1D >( (TH1D*) filePtr->Get( ( p + "_" + identifier ).c_str() ) );
	}

    //prompt or nonprompt can be considered the signal process 
    std::string signalProcess;
    std::string backgroundProcess;
    if( promptSignal ){
        signalProcess = "prompt";
        backgroundProcess = "nonprompt";
    } else {
        signalProcess = "nonprompt";
        backgroundProcess = "prompt";
    }
	
	//prepare necessary arguments to print datacard 
	double observedYield = processHistogramMap["data"]->GetSumOfWeights();
	double signalYield = processHistogramMap[signalProcess]->GetSumOfWeights();
	double bkgYield[1] = { processHistogramMap[backgroundProcess]->GetSumOfWeights() };
	std::string bkgNames[1] = { backgroundProcess };
	
	//name of the file that will contain the histograms 
	std::string shapeFileName = "shapeFile_" + identifier + ".root";
	TFile* shapeFilePtr = TFile::Open( ( "datacards/" + shapeFileName ).c_str(), "RECREATE" );
	for( auto& p : processNames ){
	    processHistogramMap[p]->Write( p.c_str() );
	}
	shapeFilePtr->Close();
	
	analysisTools::printDataCard( "datacards/datacard_" + identifier + ".txt", observedYield, signalYield, signalProcess , bkgYield, 1, bkgNames, std::vector< std::vector< double > >(), 0, nullptr, nullptr, true, shapeFileName, true );
}
	


void printDataCardsForBin( TFile* filePtr, const std::string& ptEtaBinName ){

	for( auto& term : std::vector< std::string >( { "numerator", "denominator" } ) ){
		std::string identifier = term + "_mT_muon_" + ptEtaBinName;
		printDataCard( filePtr, identifier, false );
	}
}

	
//function to read MT histogram for numerator/denominator of each bin, produce a datacard
void printDataCardsFakeRateMeasurement( const std::string& fileName ){
    
    //open file containing all histograms 
    TFile* histogram_file = TFile::Open( fileName.c_str() );

    //make a list of pt/eta bins 
    std::vector< std::string > bin_names = listPtEtaBinsInFile( histogram_file );

    //collect histograms for each pt/eta bin 
    for( auto& s : bin_names ){

        //make datacard with autoMCStats 
		printDataCardsForBin( histogram_file, s );
    }

    //close ROOT file
    histogram_file->Close();
}


std::vector< std::string > listTriggersInFile( const TFile* filePtr ){
    std::vector< std::string > histogramNameVector = listHistogramsInFile( filePtr );
    std::set< std::string > triggerSet;
    for( const auto& name : histogramNameVector ){
        if( stringTools::stringContains( name, "HLT" ) ){
            auto beginPos = name.find( "HLT" );
            triggerSet.insert( name.substr( beginPos ) );
        }
    }
    return std::vector< std::string >( triggerSet.cbegin(), triggerSet.cend() );
}


//function to submit combine express jobs 
void submitDataCardJob( const std::string& dataCardPath, const std::string& CMSSWDirectory ){
    std::string cardName = stringTools::fileNameWithoutExtension( stringTools::fileNameFromPath( dataCardPath ) );
    std::string scriptName = "job_" + cardName + ".sh";
    std::ofstream jobScript( scriptName );
    systemTools::initScript( jobScript, CMSSWDirectory );
    std::string outputFileName = "output_" + cardName + ".txt";
    jobScript << "combine -M FitDiagnostics " << dataCardPath << " > " << outputFileName << " 2>> " << outputFileName << "\n";
    jobScript.close();
    //systemTools::submitScript( scriptName, "00:20:00", "express" );
    systemTools::system( "cat " + scriptName );
}


void printDataCardsPrescaleMeasurement( const std::string& fileName ){
    
    //open file containing all histograms
    TFile* histogram_file = TFile::Open( fileName.c_str() );

    //print datacards for the measurement of each trigger 
    for( const auto& trigger : listTriggersInFile( histogram_file ) ){
        printDataCard( histogram_file, "mT_" + trigger, true );

        //measure the prescale 
        submitDataCardJob( "datacards/datacard_" + ( "mT_" + trigger ) + ".txt", "/user/wverbeke/CMSSW_8_1_0/" );
    }

    histogram_file->Close();
}

//run over datacards for fakerate measurement 


//function to run combine on datacards and measure nonprompt yield in each bin 


int main( int argc, char* argv[] ){
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    //fillFakeRateMeasurementHistograms( "muon", "2016" );
    //printDataCardsFakeRateMeasurement( "muon_2016_fakeRate_mT_histograms.root" );
	fillPrescaleMeasurementHistograms();
    printDataCardsPrescaleMeasurement( "prescaleMeasurement_mT_histograms.root" );
    return 0;
}
