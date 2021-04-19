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

	HistInfo( "leptonEtaLeadingFine", "|#eta|^{leading lepton}", 30, 0, 2.5 ),
        HistInfo( "leptonEtaSubLeadingFine", "|#eta|^{subleading lepton}", 30, 0, 2.5 ),
        HistInfo( "leptonEtaTrailingFine", "|#eta|^{trailing lepton}", 30, 0, 2.5 ),

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

bool passBasicEventSelection( Event& event ){
    event.removeTaus();
    event.applyLeptonConeCorrection();
    event.cleanElectronsFromLooseMuons();
    event.selectFOLeptons();
    if( event.numberOfLightLeptons() != 3 ) return false;
    event.selectGoodJets();
    event.cleanJetsFromFOLeptons();
    event.sortLeptonsByPt();
    return true;
}

bool hasAtLeastOneNonPrompt( Event& event, const bool requireMuon = false,
			    const bool requireElectron = false ){
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

bool passClosureTestEventSelection( Event& event, const std::string& selection,
				    const bool requireMuon = false, 
				    const bool requireElectron = false ){
    
    // do basic selection (also includes cleaning)
    if( !passBasicEventSelection( event ) ){ return false; }
    if( !hasAtLeastOneNonPrompt( event, requireMuon, requireElectron ) ){ return false; }
    // now continue depending on the exact selection. default: no extra selections  
    if( selection=="default" ){
	return true;
    }
    // alternative: same as in data control regions (but without jet requirements)!
    if( selection=="noossf"){
	if( event.hasOSSFLightLeptonPair() ){ return false; }
	return true;
    }
    if( selection=="noz"){
        if( !event.hasOSSFLightLeptonPair() ){ return false; }
	if( event.hasZTollCandidate(7.5) ){ return false; }
        return true;
    }
    // alternative: same as default but with additional jet requirements
    JetCollection jetc = event.jetCollection().goodJetCollection();
    int njets = jetc.size();
    int nbjets = jetc.numberOfMediumBTaggedJets();
    if( selection=="signaljets" ){
	if( !passClosureTestEventSelection( event, "default", requireMuon, requireElectron) ){
            return false; }
	if( njets<2 || nbjets<1 ){ return false; }
	return true;
    }
    if( selection=="twojets"){
	if( !passClosureTestEventSelection( event, "default", requireMuon, requireElectron) ){ 
	    return false; }
	if( njets<2 ){ return false; }
	return true;
    }
    if( selection=="onebjet" ){
	if( !passClosureTestEventSelection( event, "default", requireMuon, requireElectron) ){ 
            return false; }
        if( nbjets<1 ){ return false; }
        return true;
    }
    if( selection=="signalregion" ){
	if( !passClosureTestEventSelection( event, "default", requireMuon, requireElectron) ){
	    return false; }
	if( njets<2 || nbjets<1 ){ return false; }
	if( !event.hasOSSFLightLeptonPair() ){ return false; }
        if( !event.hasZTollCandidate(15) ){ return false; }
	return true;
    }
    return false;
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

std::string eventOriginFlavour( const Event& event ){
    // retrieve composition of nonprompt leptons
    unsigned nlight = 0;
    unsigned nheavy = 0;
    for( auto& leptonPtr : event.lightLeptonCollection() ){
        if( !leptonPtr->isPrompt() ){
	    if( leptonPtr->provenanceCompressed()==1 
		|| leptonPtr->provenanceCompressed()==2) nheavy++;
	    else nlight++;
	}
    }
    return std::to_string(nlight)+"light"+std::to_string(nheavy)+"heavy";
}

int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;
    
    // check command line arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    if( !(argvStr.size() == 7 || argvStr.size() == 8) ){
        std::cerr<<"found "<<argc - 1<<" command line args, while 6 or 7 are needed."<<std::endl;
        std::cerr<<"usage: ./closureTest_MC isMCFR use_mT selection process year flavour [path_to_xml]"<<std::endl;
        return 1;
    }
    
    // parse command line arguments
    const bool isMCFR = (argvStr[1]=="True" or argvStr[1]=="true");
    const bool use_mT = (argvStr[2]=="True" or argvStr[2]=="true");
    std::string selection = argvStr[3];
    std::string process = argvStr[4];
    std::string year = argvStr[5];
    std::string flavor = argvStr[6];
    if( flavor!="electron" && flavor!="muon" ){ flavor=""; }
    bool doBDT = false;
    std::string path_to_xml = "";
    if( argvStr.size()==8 ){
	doBDT = true;
	path_to_xml = argvStr[7];
    }

    // check year string
    if( !( year == "2016" || year == "2017" || year == "2018" || year == "all") ){
        throw std::invalid_argument( "Year string "+year+" not valid");
    }
    std::vector<std::string> years = {year};
    if( year=="all" ){ years = {"2016","2017","2018"}; }

    // check process string
    if( ! (process == "TT" || process == "DY" || process=="all") ){
        std::string errorm = "Given closure test process argument is '" + process;
        errorm.append("' while it should be DY, TT or all.");
        throw std::invalid_argument( errorm );
    }
    std::vector<std::string> processes = {process};
    if( process=="all" ){ processes = {"TT","DY" }; }

    // make TMVA reader
    TMVA::Reader* reader = new TMVA::Reader();
    if(doBDT) reader = eventFlattening::initializeReader(reader, path_to_xml, "all");

    setTDRStyle();

    // check if only muons, only electrons, or both
    bool onlyMuonFakes = ( flavor == "muon" );
    bool onlyElectronFakes = ( flavor == "electron" );

    //make collection of histograms (OLD)
    //std::vector< std::shared_ptr< TH1D > > observedHists; 
    //std::vector< std::shared_ptr< TH1D > > predictedHists;
    //std::vector< HistInfo > histInfoVec = makeDistributionInfo();

    // make collection of histograms (NEW)
    // vector of observed histograms (one element per process and per variable)
    std::map<std::string, std::map< std::string, std::shared_ptr< TH1D > > > observedHists; 
    // vector of total predicted histograms (one element per process and per variable)
    std::map<std::string, std::map< std::string, std::shared_ptr< TH1D > > > predictedHists;
    // structure of split predicted histograms 
    // (one map per process and variable, mapping split sources of prediction)
    std::vector< std::string > predictionCategories = {"light","heavy","other"};
    std::map<std::string, std::map< std::string, std::map< std::string, std::shared_ptr<TH1D> > > > predictedHistsSplit;
    std::vector< HistInfo > histInfoVec = makeDistributionInfoNew();
    std::map<std::string,double> varmap = eventFlattening::initVarMap();

    for( auto process: processes ){
	for( const auto& histInfo : histInfoVec ){
	    observedHists[process][histInfo.name()] = histInfo.makeHist( 
		histInfo.name()+ "_observed_"+process+"_"+year);
	    predictedHists[process][histInfo.name()] = histInfo.makeHist( 
		histInfo.name()+"_predicted_"+process+"_"+year);
	    for( std::string cat: predictionCategories ){
		predictedHistsSplit[process][histInfo.name()][cat] = histInfo.makeHist(
		    histInfo.name()+"_predicted_"+process+"_"+cat+"_"+year);
	    }
	}
    }

    // make additional collection of histograms containing some diagnostics
    HistInfo failPtInfo = HistInfo( "leptonPtFailing", "P_{T}^{failing leptons} (GeV)", 10, 10, 100 );
    std::shared_ptr<TH1D> failPtHist = failPtInfo.makeHist( failPtInfo.name()+"_"+process+"_"+year );
   
    for( auto thisyear: years ){
	for( auto thisprocess: processes ){

	    // set samples
	    const std::string sampleDirectory = "/pnfs/iihe/cms/store/user/llambrec/trileptonskim/fakerate/";
	    std::string sampleListBase = "../../fakeRate/sampleLists/samples_closureTest";
            std::string sampleListFile = sampleListBase+"_"+thisprocess+"_"+thisyear+".txt";
 
	    // read fake-rate map corresponding to this year and flavor 
	    std::shared_ptr< TH2D > fakeRateMap_muon = readFRMap( "muon", thisyear, isMCFR, use_mT );
	    std::shared_ptr< TH2D > fakeRateMap_electron = readFRMap( "electron", thisyear, isMCFR, use_mT );

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
			                        "../../weights/", thisyear, thissample );

		long unsigned numberOfEntries = treeReader.numberOfEntries();
		//long unsigned numberOfEntries = 1000; // temp for testing
		std::cout<<"starting event loop for "<<numberOfEntries<<" events"<<std::endl;
		for( long unsigned entry = 0; entry < numberOfEntries; ++entry ){
		    Event event = treeReader.buildEvent( entry );

		    // apply event selection
		    if( !passClosureTestEventSelection( event, selection, 
			    onlyMuonFakes, onlyElectronFakes ) ) continue;
		    LightLeptonCollection lightLeptons = event.lightLeptonCollection();

		    // prints for testing
		    /*JetCollection jetc = event.jetCollection().goodJetCollection();
		    int njets = jetc.size();
		    int nbjets = jetc.numberOfMediumBTaggedJets();
		    std::cout << "after selection: " << njets << ", " << nbjets << std::endl;*/

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
			lightLeptons[0].absEta(), lightLeptons[1].absEta(), lightLeptons[2].absEta(),
			lightLeptons[0].absEta(), lightLeptons[1].absEta(), lightLeptons[2].absEta()
		    };
		    for( unsigned j=9; j<histInfoVec.size(); ++j){
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
			    observedHists[thisprocess][histInfoVec[v].name()]->Fill( 
				std::min( variables[v],  histInfoVec[v].maxBinCenter() ), weight );
			}
		    } else {
			//compute event weight with fake-rate
			weight = weight*fakeRateWeight( event, fakeRateMap_muon, fakeRateMap_electron );
			// find origin of nonprompt leptons
			std::string raworigin = eventOriginFlavour( event );
			std::string origin = (raworigin=="1light0heavy") ? "light" :
						(raworigin=="0light1heavy") ? "heavy" :
						"other";
			// fill histograms
			for( std::vector< double >::size_type v = 0; v < variables.size(); ++v ){
			    predictedHists[thisprocess][histInfoVec[v].name()]->Fill( 
				std::min( variables[v],  histInfoVec[v].maxBinCenter() ), weight );

			    predictedHistsSplit[thisprocess][histInfoVec[v].name()][origin]->Fill( 
				std::min( variables[v], histInfoVec[v].maxBinCenter() ), weight);
			}

			// fill histogram with pt of failing leptons
			for( const auto& leptonPtr : event.lightLeptonCollection() ){
			    if( leptonPtr->isFO() && !leptonPtr->isTight() ){
				failPtHist->Fill( leptonPtr->pt(), event.scaledWeight() );
			    }
			}
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
    }
    /*std::cout<<"--- histogram ---"<<std::endl;
    for(int i=1; i<observedHists[0]->GetNbinsX()+1; ++i){
	std::cout<<"bin: "<<i<<std::endl;
	std::cout<<"observed: "<<observedHists[0]->GetBinContent(i)<<std::endl;
	std::cout<<"predicted: "<<predictedHists[0]->GetBinContent(i)<<std::endl;
    }*/

    delete reader;

    // write file
    std::string flavorInterpendix = (flavor=="")? "": "_"+flavor;
    std::string file_name = "closurePlots_MC_" + selection + "_" + process + "_" + year;
    file_name += flavorInterpendix+".root";
    TFile* histogram_file = TFile::Open( file_name.c_str(), "RECREATE" );

    for( auto process: processes ){
        for( const auto& histInfo : histInfoVec ){
            observedHists[process][histInfo.name()]->Write();
            predictedHists[process][histInfo.name()]->Write();
            for( std::string cat: predictionCategories ){
                predictedHistsSplit[process][histInfo.name()][cat]->Write();
            }
        }
    }

    histogram_file->Close();
    std::cerr << "###done###" << std::endl;
    return 0;
    
    // older part of the code (filling and plotting not separated)
    /*
    // make plot output directory
    std::string outputDirectory_name = "closurePlots_MC_" + selection + "_" + process + "_" + year; 
    if( flavor != "" ){
        outputDirectory_name += ( "_" + flavor );
    }
    systemTools::makeDirectory( outputDirectory_name );
    
    // make plots 
    for( std::vector< HistInfo >::size_type v = 0; v < histInfoVec.size(); ++v ){
	std::string names[2] = {"MC observed", "fake-rate prediction"};
        std::vector< TH1D* > predicted = { predictedHists[v].get() };
	std::string namesSplit[1+predictionCategories.size()];
	TH1D* predictedSplit[predictionCategories.size()];
	namesSplit[0] = "MC observed";
	for( unsigned i=0; i<predictionCategories.size(); i++){
	    namesSplit[i+1] = predictionCategories[i];
	    predictedSplit[i] = predictedHistsSplit[v][predictionCategories[i]].get();
	}
        std::string header;
        if( year == "2016" ){
            header = "35.9 fb^{-1} (13 TeV)";
        } else if( year == "2017" ){
            header = "41.5 fb^{-1} (13 TeV)";
        } else if( year== "2018" ){
            header = "59.7 fb^{-1} (13 TeV)";
	} else if( year == "all" ){
	    header = "137 fb^{-1} (13 TeV)";
	} else{
	    header = "";
        }
        TH1D* systUnc = dynamic_cast< TH1D* >( predictedHists[v]->Clone() );
        for( int b = 1; b < systUnc->GetNbinsX() + 1; ++b ){
            systUnc->SetBinContent( b , systUnc->GetBinContent(b)*0.3 );
        }
	// make standard plot
	plotDataVSMC( observedHists[v].get(), &predicted[0], names, 1, 
			stringTools::formatDirectoryName( outputDirectory_name ) 
			+ histInfoVec[v].name() + "_" + selection + "_" + process + "_" + year 
			+ flavorInterpendix + ".pdf", 
			"", false, false, header, systUnc);
	// make plot with split heavy and light
	plotDataVSMC( observedHists[v].get(), predictedSplit, namesSplit, 
			predictionCategories.size(),
                        stringTools::formatDirectoryName( outputDirectory_name )
                        + histInfoVec[v].name() + "_" + selection + "_" + process + "_" + year 
			+ flavorSuffix + "_split.pdf",
                        "", false, false, header, systUnc);
	// make log-scale plot
	plotDataVSMC( observedHists[v].get(), &predicted[0], names, 1,  
                        stringTools::formatDirectoryName( outputDirectory_name )
                        + histInfoVec[v].name() + "_" + selection + "_" + process + "_" + year
                        + flavorInterpendix + "_log.pdf",
                        "", true, false, header, systUnc);
    }

    // make addtional plots
    std::string flavorInterpendix = (flavor=="")? "": "_"+flavor;
    std::string names[1] = {""};
    TH1D* hists[1] = {failPtHist.get()};
    plotHistograms( hists, 1, names, 
			stringTools::formatDirectoryName( outputDirectory_name )
                        + failPtInfo.name() + "_" + selection + "_" + process + "_" + year 
			+ flavorInterpendix + ".pdf", false, false);

    std::cerr << "###done###" << std::endl;
    return 0; */
}
