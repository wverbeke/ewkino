

//include c++ library classes 
#include <vector>
#include <utility>
#include <memory>
#include <string>

//include ROOT classes 
#include "TH1D.h"
#include "TFile.h"

//include parts of analysis framework
#include "../Event/interface/Event.h"
#include "../Tools/interface/RangedMap.h"
#include "../Tools/interface/HistInfo.h"
#include "../Tools/interface/stringTools.h"
#include "../Tools/interface/analysisTools.h"



//TAKE FR TRIGGERS INTO ACCOUNT 
bool passFakeRateTrigger( const Event& event ){
    return true;
}


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

//function to fill MT shape histogram for numerator and denominator
void makeFakeRateHistograms( const std::string& leptonFlavor, const std::string& year ){

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
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > prompt_numerator_map = build2DHistogramMap( ptBinBorders, etaBinBorders, mtHistInfo, leptonFlavor + "_prompt_numerator" );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > prompt_denominator_map = build2DHistogramMap( ptBinBorders, etaBinBorders, mtHistInfo, leptonFlavor + "_prompt_denominator" );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > nonprompt_numerator_map = build2DHistogramMap( ptBinBorders, etaBinBorders, mtHistInfo, leptonFlavor + "_nonprompt_numerator" );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > nonprompt_denominator_map = build2DHistogramMap( ptBinBorders, etaBinBorders, mtHistInfo, leptonFlavor + "_nonprompt_denominator" );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > data_numerator_map = build2DHistogramMap( ptBinBorders, etaBinBorders, mtHistInfo, leptonFlavor +  "_data_numerator" );
    RangedMap< RangedMap< std::shared_ptr< TH1D > > > data_denominator_map = build2DHistogramMap( ptBinBorders, etaBinBorders, mtHistInfo, leptonFlavor + "_data_denominator" );

    //initialize TreeReader
    //ADAPT SAMPLE LIST TO USE DATA FOR CORRECT YEAR
    TreeReader treeReader( "samples_fakeRateMeasurement.txt", "../test/testData");
    for( unsigned sampleIndex = 0; sampleIndex < treeReader.numberOfSamples(); ++sampleIndex ){

        //load next sample
        treeReader.initSample();

        //loop over events in sample
        for( long unsigned entry = 0; entry < treeReader.nEntries; ++entry ){
            Event event = treeReader.buildEvent( entry );


            //Require the presence of just one lepton, and veto a second loose lepton
            event.cleanElectronsFromLooseMuons();
            event.cleanTausFromLooseLightLeptons();
            if( event.numberOfLooseLeptons() != 1 ) continue;

            //Select FO leptons 
            event.selectFOLeptons();
            if( event.numberOfLeptons() != 1 ) continue;

            //IMPORTANT : apply cone correction
            event.applyLeptonConeCorrection();

            //Require the presence of at least one good jet
            event.selectGoodJets();
            event.cleanJetsFromLooseLeptons();
            if( event.numberOfJets() < 1 ) continue;

            Lepton& lepton = event.lepton( 0 );

            //select correct lepton flavor
            if( isMuonMeasurement ){
                if( !lepton.isMuon() ) continue;
            } else {
                if( !lepton.isElectron() ) continue;
            }

            //require deltaR of at least 1 between lepton and any of the selected jets 
            double maxDeltaR = 0; 
            for( auto jetPtr : event.jetCollection() ){
                double currentDeltaR = deltaR( lepton, *jetPtr );
                if( currentDeltaR > maxDeltaR ){
                    maxDeltaR = currentDeltaR;
                }
            }
            if( maxDeltaR < 1 ) continue;

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


void printDataCardsForBin( TFile* filePtr, const std::string& ptEtaBinName ){

	//GENERALIZE THE EXTRACTION OF THE NAMES HERE 

	for( auto& term : std::vector< std::string >( { "numerator", "denominator" } ) ){
		std::vector< std::string > processNames = { "data", "prompt", "nonprompt" };

		std::map< std::string, std::shared_ptr< TH1D > > processHistogramMap;
		for( auto& p : processNames ){
			processHistogramMap[p] = std::shared_ptr< TH1D >( (TH1D*) filePtr->Get( ("mTmuon_" + p + "_" + term + "_" + ptEtaBinName ).c_str() ) );
		}

		//prepare necessary arguments to print datacard 
		double observedYield = processHistogramMap["data"]->GetSumOfWeights();
		double signalYield = processHistogramMap["nonprompt"]->GetSumOfWeights();
		double bkgYield[1] = { processHistogramMap["prompt"]->GetSumOfWeights() };
		std::string bkgNames[1] = { "prompt" };

		//name of the file that will contain the histograms 
		std::string shapeFileName = "shapeFile_" + term + "_muon_" + ptEtaBinName + ".root";
		TFile* shapeFilePtr = TFile::Open( ( "datacards/" + shapeFileName ).c_str(), "RECREATE" );
		for( auto& p : processNames ){
			processHistogramMap[p]->Write( p.c_str() ); 
		}
		shapeFilePtr->Close();

		analysisTools::printDataCard( "datacards/datacard_" + term + "_muon_" + ptEtaBinName + ".txt", observedYield, signalYield, "nonprompt", bkgYield, 1, bkgNames, std::vector< std::vector< double > >(), 0, nullptr, nullptr, true, shapeFileName, true );

	}
}



//function to read MT histogram for numerator/denominator of each bin, produce a datacard
void printDataCards( const std::string& fileName ){
    
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


//function to run combine on datacards and measure nonprompt yield in each bin 


int main(){

    //makeFakeRateHistograms( "muon", "2016" );
    printDataCards( "muon_2016_fakeRate_mT_histograms.root" );
    return 0;
}
