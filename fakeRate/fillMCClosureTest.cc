// include c++ library classes 
#include <memory>

//include other parts of framework
#include "../TreeReader/interface/TreeReader.h"
#include "../Event/interface/Event.h"
#include "../Tools/interface/HistInfo.h"
#include "interface/fakeRateTools.h"
#include "../plotting/plotCode.h"
#include "../plotting/tdrStyle.h"
#include "../Tools/interface/systemTools.h"
#include "../Tools/interface/stringTools.h"
#include "../Tools/interface/analysisTools.h"

std::vector< HistInfo > makeDistributionInfoDefault(){
    std::vector< HistInfo > histInfoVec = {
	HistInfo( "leptonPtLeading", "p_{T}^{leading lepton} (GeV)", 10, 10, 200 ),
	HistInfo( "leptonPtSubLeading", "p_{T}^{subleading lepton} (GeV)", 10, 10, 150 ),

	HistInfo( "leptonEtaLeading", "|#eta|^{leading lepton}", 10, 0, 2.5 ),
	HistInfo( "leptonEtaSubLeading", "|#eta|^{subleading lepton}", 10, 0, 2.5 ),

	HistInfo( "nLeptons", "number of leptons", 3, 1.5, 4.5 ),	
	HistInfo( "met", "E_{T}^{miss} (GeV)", 10, 0, 300 ),
	HistInfo( "ltmet", "L_{T} + E_{T}^{miss} (GeV)", 10, 0, 300 ),
	HistInfo( "ht", "H_{T} (GeV)", 10, 0, 600 ),

	HistInfo( "nJets", "number of jets", 8, 0, 8 ),
	HistInfo( "nBJets", "number of b-jets (medium deepFlavour)", 4, 0, 4 ),
	HistInfo( "nVertex", "number of vertices", 10, 0, 70 ),

	HistInfo( "nLightNonPrompt", "number of nonprompt with light origin", 4, 0, 4 ),
	HistInfo( "nCFlavorNonPrompt", "number of nonprompt with c-hadron origin", 4, 0, 4 ),
	HistInfo( "nBFlavorNonPrompt", "number of nonprompt with b-hadron origin", 4, 0, 4 )
    };
    return histInfoVec;
}

std::shared_ptr< TH2D > readFRMap( const std::string& flavor, const std::string& year, 
				    const bool isMCFRMap, const bool use_mT ){
    std::string file_name = "fakeRateMaps/fakeRateMap";
    file_name.append(isMCFRMap ? "_MC" : "_data");
    file_name.append("_"+flavor+"_"+year);
    file_name.append(use_mT ? "_mT" : "");
    file_name.append(".root");
    std::cout<<"fake rate map file name: "<<file_name<<std::endl;
    TFile* frFile = TFile::Open( file_name.c_str() );
    std::shared_ptr< TH2D > frMap( dynamic_cast< TH2D* >( frFile->Get( ( 
			    "fakeRate_" + flavor + "_" + year ).c_str() ) ) );
    frMap->SetDirectory( gROOT );
    frFile->Close();

    // printout for testing
    /*std::cout<<"values:"<<std::endl;
    for(unsigned xbin=1; xbin<=5; ++xbin){
        for(unsigned ybin=1; ybin<=3; ++ybin){
            std::cout<<"bin: "<<xbin<<" "<<ybin<<std::endl;
            std::cout<<frMap->GetBinContent(xbin,ybin)<<std::endl;
        }
    }*/

    return frMap;
}

bool passClosureTestEventSelection( Event& event, const bool requireMuon = false, 
				    const bool requireElectron = false ){
    // do basic selections
    event.removeTaus();
    event.applyLeptonConeCorrection();
    event.cleanElectronsFromLooseMuons();
    event.selectFOLeptons();
    if( event.numberOfLightLeptons() < 2 ) return false;
    if( event.numberOfLightLeptons() == 2 
	&& event.lightLeptonCollection()[0].charge() 
	   != event.lightLeptonCollection()[1].charge() ) return false;
    event.selectGoodJets();
    event.cleanJetsFromFOLeptons();
    event.sortLeptonsByPt();
   
    // determine number of leptons in this event
    size_t numberOfLeptons = event.numberOfLightLeptons();

    // check which ones are prompt and which ones nonprompt
    size_t numberOfNonPromptLeptons = 0;
    size_t numberOfPromptLeptons = 0;
    for( auto& leptonPtr : event.lightLeptonCollection() ){
        if( !leptonPtr->isPrompt() ){
	    if( requireMuon && !leptonPtr->isMuon() ) continue;
            if( requireElectron && !leptonPtr->isElectron() ) continue;
            ++numberOfNonPromptLeptons;
        }
	else if( leptonPtr->isPrompt() ) ++numberOfPromptLeptons;
    }

    // require at least one nonprompt lepton of correct flavour
    if( numberOfNonPromptLeptons < 1 ) return false;
    if( ( numberOfNonPromptLeptons + numberOfPromptLeptons ) != numberOfLeptons ) return false;
    return true;
}

double fakeRateWeight( const Event& event, const std::shared_ptr< TH2D >& frMap_muon,  
			const std::shared_ptr< TH2D >& frMap_electron ){
    double weight = -1.;
    for( const auto& leptonPtr : event.lightLeptonCollection() ){
        if( leptonPtr->isFO() && !leptonPtr->isTight() ){

            //double croppedPt = std::min( leptonPtr->pt(), 99. );
	    double croppedPt = std::min( leptonPtr->pt(), 44.9 );
	    // (test, to be more consistent with fake rate application in data)
            double croppedAbsEta = std::min( leptonPtr->absEta(), (leptonPtr->isMuon() ? 2.4 : 2.5) );

            double fr;
            if( leptonPtr->isMuon() ){
                fr = frMap_muon->GetBinContent( frMap_muon->FindBin( croppedPt, croppedAbsEta ) );
            } else {
                fr = frMap_electron->GetBinContent( frMap_electron->FindBin( croppedPt, croppedAbsEta ) );
            }

	    //std::cout<<"isMuon: "<<leptonPtr->isMuon()<<", pt: "<<leptonPtr->pt()<<std::endl;
	    //std::cout<<fr<<std::endl;

            weight *= ( - fr / ( 1. - fr ) );
        }
    }
    return weight;
}

std::tuple<int,int,int> eventOriginFlavour( const Event& event ){
    // retrieve flavour composition of nonprompt leptons
    unsigned nlight = 0;
    unsigned ncflavor = 0;
    unsigned nbflavor = 0;
    for( auto& leptonPtr : event.lightLeptonCollection() ){
        if( !leptonPtr->isPrompt() ){
	    if( leptonPtr->provenanceCompressed()==1 ) nbflavor++;
	    else if( leptonPtr->provenanceCompressed()==2 ) ncflavor++;
	    else nlight++;
	}
    }
    return std::make_tuple<int,int,int>(nlight,ncflavor,nbflavor);
}

int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;
    
    // check command line arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    if( !(argvStr.size() == 9) ){
        std::cerr<<"found "<<argc - 1<<" command line args, while 8 are needed."<<std::endl;
        std::cerr<<"usage: ./closureTest_MC isMCFR use_mT process year flavour";
	std::cerr<<" sampleDirectory sampleList isTestRun"<<std::endl;
        return 1;
    }
    
    // parse command line arguments
    const bool isMCFR = (argvStr[1]=="True" or argvStr[1]=="true");
    const bool use_mT = (argvStr[2]=="True" or argvStr[2]=="true");
    std::string process = argvStr[3];
    std::string year = argvStr[4];
    std::string flavor = argvStr[5];
    std::string sampleDirectory = argvStr[6];
    std::string sampleListFile = argvStr[7];
    const bool isTestRun = (argvStr[8]=="True" or argvStr[8]=="true");
    if( flavor!="electron" && flavor!="muon" ){ flavor=""; }
    setTDRStyle();

    // check year string
    analysisTools::checkYearString( year );
    // check process string
    if( ! (process == "TT" || process == "DY" ) ){
	std::string errorm = "Given closure test process argument is '" + process;
	errorm.append("' while it should be DY or TT.");
        throw std::invalid_argument( errorm );
    }
    // check if only muons, only electrons, or both
    bool onlyMuonFakes = ( flavor == "muon" );
    bool onlyElectronFakes = ( flavor == "electron" );

    // make collection of histograms
    std::vector< std::shared_ptr< TH1D > > observedHists; 
    std::vector< std::shared_ptr< TH1D > > predictedHists;
    std::vector< HistInfo > histInfoVec = makeDistributionInfoDefault();
    for( const auto& histInfo : histInfoVec ){
        observedHists.push_back( histInfo.makeHist( histInfo.name()+ "_observed_"+process+"_"+year) );
        predictedHists.push_back( histInfo.makeHist( histInfo.name()+"_predicted_"+process+"_"+year) );
    }

    
    
    // read fake-rate map corresponding to this year and flavor 
    std::shared_ptr< TH2D > fakeRateMap_muon = readFRMap( "muon", year, isMCFR, use_mT );
    std::shared_ptr< TH2D > fakeRateMap_electron = readFRMap( "electron", year, isMCFR, use_mT );

    // loop over samples to fill histograms
    TreeReader treeReader( sampleListFile, sampleDirectory );

    unsigned numberOfSamples = treeReader.numberOfSamples();
    if( isTestRun ) numberOfSamples = 1;
    for( unsigned i = 0; i < numberOfSamples; ++i ){
	std::cout<<"start processing sample n. "<<i+1<<" of "<<numberOfSamples<<std::endl;
        treeReader.initSample();

	long unsigned numberOfEntries = treeReader.numberOfEntries();
	if( isTestRun ) numberOfEntries = 500000;
        std::cout<<"starting event loop for "<<numberOfEntries<<" events"<<std::endl;
	for( long unsigned entry = 0; entry < numberOfEntries; ++entry ){
            Event event = treeReader.buildEvent( entry );

            // apply event selection
            if( !passClosureTestEventSelection( event, onlyMuonFakes, onlyElectronFakes ) ) continue;
	    LightLeptonCollection lightLeptons = event.lightLeptonCollection();

            //compute plotting variables 
	    std::tuple<int,int,int> temp = eventOriginFlavour( event );
            std::vector< double > variables = { 
		lightLeptons[0].pt(), lightLeptons[1].pt(),
                lightLeptons[0].absEta(), lightLeptons[1].absEta(),
		static_cast< double >( lightLeptons.size() ),
                event.metPt(),
                lightLeptons.scalarPtSum() + event.metPt(),
                event.HT(),
                static_cast< double >( event.numberOfJets() ),
                static_cast< double >( event.numberOfMediumBTaggedJets() ),
                static_cast< double >( event.numberOfVertices() ),
		static_cast< double >( std::get<0>(temp) ),
		static_cast< double >( std::get<1>(temp) ),
		static_cast< double >( std::get<2>(temp) )
            };

            // event is 'observed' if all leptons are tight 
            bool isObserved = true;
	    double weight = event.weight();
            for( const auto& leptonPtr : lightLeptons ){
                if( !leptonPtr->isTight() ){
                    isObserved = false;
                }
            }

            if( isObserved ){
                for( std::vector< double >::size_type v = 0; v < variables.size(); ++v ){
                    observedHists[v]->Fill( std::min( variables[v],  histInfoVec[v].maxBinCenter() ), 
					    weight );
                }
            } else {

		//compute event weight with fake-rate
		weight = weight*fakeRateWeight( event, fakeRateMap_muon, fakeRateMap_electron );
		for( std::vector< double >::size_type v = 0; v < variables.size(); ++v ){
                    predictedHists[v]->Fill( std::min( variables[v],  histInfoVec[v].maxBinCenter() ), 
						weight );
                }
            }
	    // printouts for testing
	    /*std::cout<<"----- event ------"<<std::endl;
	    for(const auto& leptonPtr : lightLeptons ){
		std::cout<<*leptonPtr<<std::endl;
		std::cout<<"is prompt"<<leptonPtr->isPrompt()<<std::endl;
		std::cout<<"is tight: "<<leptonPtr->isTight()<<std::endl;
	    }
	    std::cout<<"is observed: "<<isObserved<<std::endl;
	    std::cout<<"(scaled) event weight: "<<event.weight()<<std::endl;
	    std::cout<<"final event weight: "<<weight<<std::endl;*/
        }
    }
    // printouts for testing
    /*std::cout<<"--- histogram ---"<<std::endl;
    for(int i=1; i<observedHists[0]->GetNbinsX()+1; ++i){
	std::cout<<"bin: "<<i<<std::endl;
	std::cout<<"observed: "<<observedHists[0]->GetBinContent(i)<<std::endl;
	std::cout<<"predicted: "<<predictedHists[0]->GetBinContent(i)<<std::endl;
    }*/

    // write file
    std::string flavorInterpendix = (flavor=="")? "": "_"+flavor;
    std::string fileName = "closurePlots_MC_" + process + "_" + year;
    fileName += flavorInterpendix + ".root";
    TFile* outputFilePtr = TFile::Open( fileName.c_str(), "RECREATE" );
    outputFilePtr->cd();

    for( std::vector< HistInfo >::size_type v = 0; v < histInfoVec.size(); ++v ){
        TH1D* predicted = predictedHists[v].get();
        TH1D* observed = observedHists[v].get();
	std::string pName = histInfoVec[v].name() + "_" + process + "_" + year 
	        + "_" + flavor + "_predicted";
	std::string oName = histInfoVec[v].name() + "_" + process + "_" + year 
	        + "_" + flavor + "_observed";
	predicted->SetName( pName.c_str() );
	observed->SetName( oName.c_str() );
	predicted->Write();
	observed->Write();
    }

    outputFilePtr->Close();
    std::cerr << "###done###" << std::endl;
    return 0;

    // the part of the code below is deprecated.
    // it creates a directory and makes the closure plots.
    // however, the plotting functionality has been moved out of this script
    // in order to allow quick aesthetic changes without having to rerun over the ntuples.
    // also the C++ plotting functions have been replaced with more versatile Python equivalents.
    // keept the code below however (in comments) for reference

    /*//make plot output directory
    std::string outputDirectory_name = "closurePlots_MC_" + process + "_" + year; 
    if( flavor != "" ){
        outputDirectory_name += ( "_" + flavor );
    }
    systemTools::makeDirectory( outputDirectory_name );
    
    //make plots 
    for( std::vector< HistInfo >::size_type v = 0; v < histInfoVec.size(); ++v ){
	std::string names[2] = {"MC observed", "fake-rate prediction"};
        std::vector< TH1D* > predicted = { predictedHists[v].get() };
        std::string header;
        if( year == "2016" ){
            header = "35.9 fb^{-1}";
        } else if( year == "2017" ){
            header = "41.5 fb^{-1}";
        } else{
            header = "59.7 fb^{-1}";
        }
        TH1D* systUnc = dynamic_cast< TH1D* >( predictedHists[v]->Clone() );
        for( int b = 1; b < systUnc->GetNbinsX() + 1; ++b ){
            systUnc->SetBinContent( b , systUnc->GetBinContent(b)*0.3 );
        }
        if( flavor == "" ){
	    plotDataVSMC( observedHists[v].get(), &predicted[0], names, 1, 
			    stringTools::formatDirectoryName( outputDirectory_name ) 
			    + histInfoVec[v].name() + "_" + process + "_" + year + ".pdf", 
			    "", false, false, header, systUnc);
        } else {
	    plotDataVSMC( observedHists[v].get(), &predicted[0], names, 1, 
			stringTools::formatDirectoryName( outputDirectory_name ) + histInfoVec[v].name() 
			+ "_" + process + "_" + year + "_" + flavor + ".pdf", 
			"", false, false, header, systUnc);
        }
    }

    std::cerr << "###done###" << std::endl;
    return 0;*/
}
