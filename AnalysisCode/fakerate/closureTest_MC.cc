// include c++ library classes 
#include <memory>

//include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"
#include "../../Tools/interface/HistInfo.h"
#include "../../fakeRate/interface/fakeRateTools.h"
#include "../../plotting/plotCode.h"
#include "../../plotting/tdrStyle.h"
#include "../../Tools/interface/systemTools.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/analysisTools.h"
#include "../eventselection/interface/eventFlattening.h"

std::vector< HistInfo > makeDistributionInfo(){
    std::vector< HistInfo > histInfoVec = {
	HistInfo( "leptonPtLeading", "p_{T}^{leading lepton} (GeV)", 10, 10, 200 ),
	HistInfo( "leptonPtSubLeading", "p_{T}^{subleading lepton} (GeV)", 10, 10, 150 ),
	HistInfo( "leptonPtTrailing", "P_{T}^{trailing lepton} (GeV)", 10, 10, 100 ),

	HistInfo( "leptonEtaLeading", "|#eta|^{leading lepton}", 10, 0, 2.5 ),
	HistInfo( "leptonEtaSubLeading", "|#eta|^{subleading lepton}", 10, 0, 2.5 ),
	HistInfo( "leptonEtaTrailing", "|#eta|^{trailing lepton}", 10, 0, 2.5 ),
	
	HistInfo( "met", "E_{T}^{miss} (GeV)", 10, 0, 300 ),
        HistInfo( "mt", "M_{T}^{W} (GeV)", 10, 0, 300 ),
	HistInfo( "mll", "M_{ll} (GeV)", 10, 0, 200 ),
	HistInfo( "ltmet", "L_{T} + E_{T}^{miss} (GeV)", 10, 0, 300 ),
	HistInfo( "ht", "H_{T} (GeV)", 10, 0, 600 ),
	HistInfo( "m3l", "M_{3l} (GeV)", 10, 0, 300 ), 
	HistInfo( "mt3l", "M_{T}^{3l} (GeV)", 10, 0, 300 ),

	HistInfo( "nJets", "number of jets", 8, 0, 8 ),
	HistInfo( "nBJets", "number of b-jets (medium deepFlavour)", 4, 0, 4 ),
	HistInfo( "nVertex", "number of vertices", 10, 0, 70 )
    };
    return histInfoVec;
}

std::vector< HistInfo > makeDistributionInfoNew(){
    std::vector< HistInfo > histInfoVec = {
	HistInfo( "leptonPtLeading", "p_{T}^{leading lepton} (GeV)", 10, 10, 200 ),
        HistInfo( "leptonPtSubLeading", "p_{T}^{subleading lepton} (GeV)", 10, 10, 150 ),
	HistInfo( "leptonPtTrailing", "P_{T}^{trailing lepton} (GeV)", 10, 10, 100 ),

	HistInfo( "leptonEtaLeading", "|#eta|^{leading lepton}", 10, 0, 2.5 ),
	HistInfo( "leptonEtaSubLeading", "|#eta|^{subleading lepton}", 10, 0, 2.5 ),
	HistInfo( "leptonEtaTrailing", "|#eta|^{trailing lepton}", 10, 0, 2.5 ),

	HistInfo("_abs_eta_recoil","#||{#eta}_{recoil}",20,0.,5.),
	HistInfo("_Mjj_max","M_{jet+jet}^{max}",20,0.,1200.),
	HistInfo("_lW_asymmetry","asymmetry (lepton from W)",20,-2.5,2.5),
	HistInfo("_deepCSV_max","highest deepCSV",20,0.,1.),
	HistInfo("_lT","L_{T}",20,0.,800.),
	HistInfo("_MT","M_{T}",20,0.,300.),
	HistInfo("_pTjj_max","p_{T}^{max}(jet+jet)",20,0.,300.),
	HistInfo("_dRlb_min","#Delta R(lep,bjet)_{min}",20,0.,3.15),
	HistInfo("_dPhill_max","#Delta #Phi (lep,lep)_{max}",20,0.,3.15),
	HistInfo("_HT","H_{T}",20,0.,800.),
	HistInfo("_nJets","number of jets",11,-0.5,10.5),
	HistInfo("_dRlWrecoil","#Delta R(lep_{W},jet_{recoil})",20,0.,10.),
	HistInfo("_dRlWbtagged","#Delta R(lep_{W},jet_{b-tagged})",20,0.,7.),
	HistInfo("_M3l","M_{3l}",20,0.,600.),
	HistInfo("_abs_eta_max","#||{#eta}_{max}",20,0.,5.),
	HistInfo("_eventBDT","event BDT output score",15,-1,1)
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
    event.removeTaus();
    event.applyLeptonConeCorrection();
    event.cleanElectronsFromLooseMuons();
    event.selectFOLeptons();
    if( event.numberOfLightLeptons() != 3 ) return false;
    event.selectGoodJets();
    event.cleanJetsFromFOLeptons();
    event.sortLeptonsByPt();
    
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
    if( numberOfNonPromptLeptons < 1 ) return false;
    if( ( numberOfNonPromptLeptons + numberOfPromptLeptons ) != 3 ) return false;
    return true;
}

double fakeRateWeight( const Event& event, const std::shared_ptr< TH2D >& frMap_muon,  
			const std::shared_ptr< TH2D >& frMap_electron ){
    double weight = -1.;
    for( const auto& leptonPtr : event.lightLeptonCollection() ){
        if( leptonPtr->isFO() && !leptonPtr->isTight() ){

            double croppedPt = std::min( leptonPtr->pt(), 99. );
	    //double croppedPt = std::min( leptonPtr->pt(), 44.9 );
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

int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;
    
    // check command line arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    if( !(argvStr.size() == 6 || argvStr.size() == 7) ){
        std::cerr<<"found "<<argc - 1<<" command line args, while 5 or 6 are needed."<<std::endl;
        std::cerr<<"usage: ./closureTest_MC isMCFR use_mT process year flavour [path_to_xml]"<<std::endl;
        return 1;
    }
    
    // parse command line arguments
    const bool isMCFR = (argvStr[1]=="True" or argvStr[1]=="true");
    const bool use_mT = (argvStr[2]=="True" or argvStr[2]=="true");
    std::string process = argvStr[3];
    std::string year = argvStr[4];
    std::string flavor = argvStr[5];
    if( flavor!="electron" && flavor!="muon" ){ flavor=""; }
    bool doBDT = false;
    std::string path_to_xml = "";
    if( argvStr.size()==7 ){
	doBDT = true;
	path_to_xml = argvStr[6];
    }

    // make TMVA reader
    TMVA::Reader* reader = new TMVA::Reader();
    if(doBDT) reader = eventFlattening::initializeReader(reader, path_to_xml, "all");

    
    const std::string sampleDirectory = "/pnfs/iihe/cms/store/user/llambrec/trileptonskim/fakerate/"; 
    std::string sampleListFile = "../../fakeRate/sampleLists/samples_closureTest_"+process+"_"+year+".txt";

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

    //make collection of histograms (OLD)
    //std::vector< std::shared_ptr< TH1D > > observedHists; 
    //std::vector< std::shared_ptr< TH1D > > predictedHists;
    //std::vector< HistInfo > histInfoVec = makeDistributionInfo();

    // make collection of histograms (NEW)
    std::vector< std::shared_ptr< TH1D > > observedHists; 
    std::vector< std::shared_ptr< TH1D > > predictedHists;
    std::vector< HistInfo > histInfoVec = makeDistributionInfoNew();
    std::map<std::string,double> varmap = eventFlattening::initVarMap();

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
    for( unsigned i = 0; i < numberOfSamples; ++i ){
	std::cout<<"start processing sample n. "<<i+1<<" of "<<numberOfSamples<<std::endl;
        treeReader.initSample();

	// make dummy reweighter (needed in syntax but result is not used)
        std::shared_ptr< ReweighterFactory >reweighterFactory( new tZqReweighterFactory() );
	std::vector<Sample> thissample;
	thissample.push_back(treeReader.currentSample());
	CombinedReweighter reweighter = reweighterFactory->buildReweighter(
                                        "../../weights/", year, thissample );

	long unsigned numberOfEntries = treeReader.numberOfEntries();
	//long unsigned numberOfEntries = 1000; // temp for testing
        std::cout<<"starting event loop for "<<numberOfEntries<<" events"<<std::endl;
	for( long unsigned entry = 0; entry < numberOfEntries; ++entry ){
            Event event = treeReader.buildEvent( entry );

            // apply event selection
            if( !passClosureTestEventSelection( event, onlyMuonFakes, onlyElectronFakes ) ) continue;
	    LightLeptonCollection lightLeptons = event.lightLeptonCollection();

	    // fill variables (OLD)
            /*double mll = 0, mtW = 0;
            if( event.hasOSSFLightLeptonPair() ){
                mll = event.bestZBosonCandidateMass();
                mtW = event.mtW();
            } else{
                mll = ( lightLeptons[0] + lightLeptons[1] ).mass();
                mtW = mt( lightLeptons[2], event.met() );
            }
            //compute plotting variables 
            std::vector< double > variables = { 
		lightLeptons[0].pt(), lightLeptons[1].pt(), lightLeptons[2].pt(),
                lightLeptons[0].absEta(), lightLeptons[1].absEta(), lightLeptons[2].absEta(),
                event.metPt(),
                mtW,
                mll,
                lightLeptons.scalarPtSum() + event.metPt(),
                event.HT(),
                lightLeptons.mass(),
                mt( lightLeptons.objectSum(), event.met() ),
                static_cast< double >( event.numberOfJets() ),
                static_cast< double >( event.numberOfMediumBTaggedJets() ),
                static_cast< double >( event.numberOfVertices() )
            };*/

	    // fill variables (NEW)
	    // use 1 for norm as the weight used in eventToEntry is not used anyway
	    // use dummy reweighter and "" for selection_type for the same reason
	    varmap = eventFlattening::eventToEntry(event, 1. , reweighter, "",
				    fakeRateMap_muon, fakeRateMap_electron, "nominal", doBDT, reader);
	    std::vector< double > variables = {
		lightLeptons[0].pt(), lightLeptons[1].pt(), lightLeptons[2].pt(),
		lightLeptons[0].absEta(), lightLeptons[1].absEta(), lightLeptons[2].absEta()
	    };
	    for( unsigned j=6; j<histInfoVec.size(); ++j){
		variables.push_back( varmap[histInfoVec[j].name()] );
	    }

            // event is 'observed' if all leptons are tight 
            bool isObserved = true;
	    double weight = event.scaledWeight();
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
	    /*std::cout<<"----- event ------"<<std::endl;
	    for(const auto& leptonPtr : lightLeptons ){
		std::cout<<*leptonPtr<<std::endl;
		std::cout<<"is prompt"<<leptonPtr->isPrompt()<<std::endl;
		std::cout<<"is tight: "<<leptonPtr->isTight()<<std::endl;
	    }
	    std::cout<<"is observed: "<<isObserved<<std::endl;
	    std::cout<<"scaled event weight: "<<event.scaledWeight()<<std::endl;
	    std::cout<<"final event weight: "<<weight<<std::endl;*/
        }
    }
    /*std::cout<<"--- histogram ---"<<std::endl;
    for(int i=1; i<observedHists[0]->GetNbinsX()+1; ++i){
	std::cout<<"bin: "<<i<<std::endl;
	std::cout<<"observed: "<<observedHists[0]->GetBinContent(i)<<std::endl;
	std::cout<<"predicted: "<<predictedHists[0]->GetBinContent(i)<<std::endl;
    }*/

    //make plot output directory
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
    return 0;
}
