// include header
#include "../interface/eventFlattening.h"
// include constants (particle masses)
#include "../../../constants/particleMasses.h"

// definition of the variables used for filling the ouput TTree
// (and evaluating an TMVA::Reader if requested)
// warning: these are not accessible outside this file; use the varmap instead!
// event id variables
ULong_t _runNb = 0;
ULong_t _lumiBlock = 0;
ULong_t _eventNb = 0;
// event weight for simulation
Float_t _weight = 0; // generator weight
Float_t _scaledweight = 0; // generator weight scaled by cross section and lumi
Float_t _normweight = 0; // total weight, including reweighting and fake rate
Float_t _leptonreweight = 1; // lepton reweighting factor
Float_t _nonleptonreweight = 1; // all other reweighting factors
Float_t _fakerateweight = 0; // fake rate reweighting factor
// event BDT variables
Float_t _abs_eta_recoil = 0;
Float_t _Mjj_max = 0;
Float_t _lW_asymmetry = 0;
Float_t _deepCSV_max = 0;
Float_t _deepFlavor_max = 0;
Float_t _lT = 0;
Float_t _MT = 0;
Float_t _pTjj_max = 0;
Float_t _dRlb_min = 99.;
Float_t _dPhill_max = 0;
Float_t _HT = 0;
Float_t _nJets = 0;
Float_t _nBJets = 0;
Float_t _dRlWrecoil = 0;
Float_t _dRlWbtagged = 0;
Float_t _M3l = 0;
Float_t _abs_eta_max = 0;
// BDT output score
Float_t _eventBDT = 0.;
// other variables
Int_t _nMuons = 0;
Int_t _nElectrons = 0;
Float_t _leptonMVATOP_min = 1.;
Float_t _leptonMVAttH_min = 1.;
Float_t _yield = 0.5; // fixed value
Float_t _leptonPtLeading = 0.;
Float_t _leptonPtSubLeading = 0.;
Float_t _leptonPtTrailing = 0.;
Float_t _leptonEtaLeading = 0.;
Float_t _leptonEtaSubLeading = 0.;
Float_t _leptonEtaTrailing = 0.;
Float_t _numberOfVertices = 0.;
Int_t _fakeRateFlavour = -1;


void eventFlattening::setVariables(std::map<std::string,double> varmap){
    // copy (and parse) the values contained in varmap to the TTree variables
    // TODO: decent error handling when element is not present in the map

    _runNb = (unsigned long) varmap["_runNb"];
    _lumiBlock = (unsigned long) varmap["_lumiBlock"];
    _eventNb = (unsigned long) varmap["_eventNb"];

    _weight = varmap["_weight"];
    _scaledweight = varmap["_scaledweight"];
    _normweight = varmap["_normweight"];
    _leptonreweight = varmap["_leptonreweight"];
    _nonleptonreweight = varmap["_nonleptonreweight"];
    _fakerateweight = varmap["_fakerateweight"];

    _abs_eta_recoil = varmap["_abs_eta_recoil"];
    _Mjj_max = varmap["_Mjj_max"]; 
    _lW_asymmetry = varmap["_lW_asymmetry"];
    _deepCSV_max = varmap["_deepCSV_max"];
    _deepFlavor_max = varmap["_deepFlavor_max"];
    _lT = varmap["_lT"];
    _MT = varmap["_MT"];
    _pTjj_max = varmap["_pTjj_max"];
    _dRlb_min = varmap["_dRlb_min"];
    _dPhill_max = varmap["_dPhill_max"];
    _HT = varmap["_HT"];
    _nJets = varmap["_nJets"];
    _nBJets = varmap["_nBJets"];
    _dRlWrecoil = varmap["_dRlWrecoil"];
    _dRlWbtagged = varmap["_dRlWbtagged"];
    _M3l = varmap["_M3l"];
    _abs_eta_max = varmap["_abs_eta_max"];
    
    _eventBDT = varmap["_eventBDT"];

    _nMuons = (int) varmap["_nMuons"];
    _nElectrons = (int) varmap["_nElectrons"];
    _leptonMVATOP_min = varmap["_leptonMVATOP_min"];
    _leptonMVAttH_min = varmap["_leptonMVAttH_min"];
    _yield = varmap["_yield"];
    _leptonPtLeading = varmap["_leptonPtLeading"];
    _leptonPtSubLeading = varmap["_leptonPtSubLeading"];
    _leptonPtTrailing = varmap["_leptonPtTrailing"];
    _leptonEtaLeading = varmap["_leptonEtaLeading"];
    _leptonEtaSubLeading = varmap["_leptonEtaSubLeading"];
    _leptonEtaTrailing = varmap["_leptonEtaTrailing"];
    _numberOfVertices = varmap["_numberOfVertices"];
    _fakeRateFlavour = varmap["_fakeRateFlavour"];
}

std::map< std::string, double > eventFlattening::initVarMap(){
    // initialize a map of variables set to their default values
    std::map< std::string, double> varmap = {
	{"_runNb", 0},{"_lumiBlock",0},{"_eventNb",0},

	{"_weight",0},{"_scaledweight",0},{"_normweight",0},
	{"_leptonreweight",1},{"_nonleptonreweight",1},{"_fakerateweight",0},

	{"_abs_eta_recoil",0},{"_Mjj_max",0},{"_lW_asymmetry",0},
	{"_deepCSV_max",0},{"_deepFlavor_max",0},{"_lT",0},
	{"_MT",0},{"_pTjj_max",0},{"_dRlb_min",99.},
	{"_dPhill_max",0},{"_HT",0},{"_nJets",0},
	{"_nBJets",0},{"_dRlWrecoil",0},{"_dRlWbtagged",0},
	{"_M3l",0},{"_abs_eta_max",0},

	{"_eventBDT",0},
	
	{"_nMuons",0},{"_nElectrons",0},
	
	{"_leptonMVATOP_min",1.},{"_leptonMVAttH_min",1.},
	
	{"_yield",0.5},
	
	{"_leptonPtLeading",0.}, {"_leptonPtSubLeading",0.}, {"_leptonPtTrailing",0.},
	{"_leptonEtaLeading",0.}, {"_leptonEtaSubLeading",0.}, {"_leptonEtaTrailing",0.},

	{"_numberOfVertices",0},
	
	{"_fakeRateFlavour",-1}
    };
    return varmap;    
}

void eventFlattening::initOutputTree(TTree* outputTree){
    // set branches for a flat output tree, to be used instead of ewkino/TreeReader/src/setOutputTree. 
    
    // event id variables
    outputTree->Branch("_runNb", &_runNb, "_runNb/l");
    outputTree->Branch("_lumiBlock", &_lumiBlock, "_lumiBlock/l");
    outputTree->Branch("_eventNb", &_eventNb, "_eventNb/l");

    // event weight for simulation (fill with ones for data)
    outputTree->Branch("_weight", &_weight, "_weight/F");
    outputTree->Branch("_scaledweight", &_scaledweight, "_scaledweight/F");
    outputTree->Branch("_normweight", &_normweight, "_normweight/F");
    outputTree->Branch("_leptonreweight", &_leptonreweight, "_leptonreweight/F");
    outputTree->Branch("_nonleptonreweight", &_nonleptonreweight, "_nonleptonreweight/F");
    outputTree->Branch("_fakerateweight", &_fakerateweight, "_fakerateweight/F"); 
   
    // event BDT variables
    outputTree->Branch("_abs_eta_recoil", &_abs_eta_recoil, "_abs_eta_recoil/F");
    outputTree->Branch("_Mjj_max", &_Mjj_max, "_Mjj_max/F");
    outputTree->Branch("_lW_asymmetry", &_lW_asymmetry, "_lW_asymmetry/F");
    outputTree->Branch("_deepCSV_max", &_deepCSV_max, "_deepCSV_max/F");
    outputTree->Branch("_deepFlavor_max", &_deepFlavor_max, "_deepFlavor_max/F");
    outputTree->Branch("_lT", &_lT, "_lT/F");
    outputTree->Branch("_MT", &_MT, "_MT/F");
    outputTree->Branch("_pTjj_max", &_pTjj_max, "_pTjj_max/F");
    outputTree->Branch("_dRlb_min", &_dRlb_min, "_dRlb_min/F");
    outputTree->Branch("_dPhill_max", &_dPhill_max, "_dPhill_max/F");
    outputTree->Branch("_HT", &_HT, "_HT/F");
    outputTree->Branch("_nJets", &_nJets, "_nJets/F");
    outputTree->Branch("_nBJets", &_nBJets, "_nBJets/F");
    outputTree->Branch("_dRlWrecoil", &_dRlWrecoil, "_dRlWrecoil/F");
    outputTree->Branch("_dRlWbtagged", &_dRlWbtagged, "_dRlWbtagged/F");
    outputTree->Branch("_M3l", &_M3l, "_M3l/F");
    outputTree->Branch("_abs_eta_max", &_abs_eta_max, "_abs_eta_max/F");

    // BDT output score (initialized here but filled in calling function!)
    outputTree->Branch("_eventBDT", &_eventBDT, "_eventBDT/F");

    // other variables
    outputTree->Branch("_nMuons", &_nMuons, "_nMuons/I");
    outputTree->Branch("_nElectrons", &_nElectrons, "_nElectrons/I");
    outputTree->Branch("_leptonMVATOP_min", &_leptonMVATOP_min, "_leptonMVATOP_min/F");
    outputTree->Branch("_leptonMVAttH_min", &_leptonMVAttH_min, "_leptonMVAttH_min/F");
    outputTree->Branch("_yield", &_yield, "_yield/F");
    outputTree->Branch("_leptonPtLeading", &_leptonPtLeading, "_leptonPtLeading/F");
    outputTree->Branch("_leptonPtSubLeading", &_leptonPtSubLeading, "_leptonPtSubLeading/F");
    outputTree->Branch("_leptonPtTrailing", &_leptonPtTrailing, "_leptonPtTrailing/F");
    outputTree->Branch("_leptonEtaLeading", &_leptonEtaLeading, "_leptonEtaLeading/F");
    outputTree->Branch("_leptonEtaSubLeading", &_leptonEtaSubLeading, "_leptonEtaSubLeading/F");
    outputTree->Branch("_leptonEtaTrailing", &_leptonEtaTrailing, "_leptonEtaTrailing/F");
    outputTree->Branch("_numberOfVertices", &_numberOfVertices, "_numberOfVertices/F");
    outputTree->Branch("_fakeRateFlavour",&_fakeRateFlavour, "_fakeRateFlavour/I");
}

TMVA::Reader* eventFlattening::initializeReader( TMVA::Reader* reader, 
		const std::string& pathToXMLFile, 
		const std::string& bdtCombineMode ){
    // make sure it is consistent with bdt training!
    reader->AddVariable("_abs_eta_recoil", &_abs_eta_recoil);
    reader->AddVariable("_Mjj_max", &_Mjj_max);
    reader->AddVariable("_lW_asymmetry", &_lW_asymmetry);
    reader->AddVariable("_deepCSV_max",&_deepCSV_max);
    reader->AddVariable("_lT", &_lT);
    reader->AddVariable("_MT", &_MT);
    reader->AddVariable("_dPhill_max", &_dPhill_max);
    reader->AddVariable("_pTjj_max", &_pTjj_max);
    reader->AddVariable("_dRlb_min", &_dRlb_min);
    reader->AddVariable("_HT", &_HT);
    reader->AddVariable("_dRlWrecoil", &_dRlWrecoil);
    reader->AddVariable("_dRlWbtagged", &_dRlWbtagged);
    reader->AddVariable("_M3l", &_M3l);
    reader->AddVariable("_abs_eta_max", &_abs_eta_max);
    if( bdtCombineMode=="all" or bdtCombineMode=="regions" ){
	// the variables nJets and nBJets are not present 
	// if trained on regions separately
	reader->AddVariable("_nJets", &_nJets);
	reader->AddVariable("_nBJets", &_nBJets);
    }

    reader->BookMVA("BDT", pathToXMLFile);
    return reader;
}

// help functions for getting the fake rate map and fake rate weight // 

std::shared_ptr< TH2D > eventFlattening::readFRMap( const std::string& pathToFile, 
			    const std::string& flavor, const std::string& year ){
    // note: this function was copied (with slight modifications)
    // from ewkino/AnalysisCode/fakerate/closureTest_MC.cc
    TFile* frFile = TFile::Open( pathToFile.c_str() );
    std::shared_ptr< TH2D > frMap( dynamic_cast< TH2D* >( frFile->Get( (
                            "fakeRate_" + flavor + "_" + year ).c_str() ) ) );
    frMap->SetDirectory( gROOT );
    frFile->Close();

    /* // printouts for testing
    std::cout<<"values:"<<std::endl;
    for(unsigned xbin=1; xbin<=5; ++xbin){
        for(unsigned ybin=1; ybin<=3; ++ybin){
            std::cout<<"bin: "<<xbin<<" "<<ybin<<std::endl;
            std::cout<<frMap->GetBinContent(xbin,ybin)<<std::endl;
        }
    }*/

    return frMap;
}

double eventFlattening::fakeRateWeight( const Event& event,
			const std::shared_ptr< TH2D >& frMap_muon,
                        const std::shared_ptr< TH2D >& frMap_electron ){
    // note: this function was copied (with slight modifications)
    // from ewkino/AnalysisCode/fakerate/closureTest_MC.cc
    
    double weight = -1.;
    //std::cout<<"--- event ---"<<std::endl;
    for( const auto& leptonPtr : event.lightLeptonCollection() ){
	if( !(leptonPtr->isFO() && !leptonPtr->isTight()) ) continue;
            
	double ptMax = 44.9; // limit to bin up to 45 GeV
	//double ptMax = 29.9; // limit to bin up to 30 GeV 
	double croppedPt = std::min( leptonPtr->pt(), ptMax );
        double croppedAbsEta = std::min( leptonPtr->absEta(), (leptonPtr->isMuon() ? 2.4 : 2.5) );

        double fr;
        if( leptonPtr->isMuon() ){
            fr = frMap_muon->GetBinContent( frMap_muon->FindBin( croppedPt, croppedAbsEta ) );
        } else {
            fr = frMap_electron->GetBinContent( frMap_electron->FindBin( croppedPt, croppedAbsEta ) );
        }
	// printouts for testing:
	//std::cout<<"--- lepton ---"<<std::endl;
	//std::cout<<"isMuon: "<<leptonPtr->isMuon()<<std::endl;
	//std::cout<<"cropped pt: "<<croppedPt<<std::endl;
	//std::cout<<"cropped eta: "<<croppedAbsEta<<std::endl;
	//std::cout<<"fake rate: "<<fr<<std::endl;

	// calculate weight
        weight *= ( - fr / ( 1. - fr ) );
    }
    //std::cout<<"weight: "<<weight<<std::endl;
    return weight;
}

int eventFlattening::fakeRateFlavour( const Event& event ){
    // return flavour of failing lepton
    // (-1 if none, 2 if more than one)
    int frflav = -1;
    for( const auto& leptonPtr : event.lightLeptonCollection() ){
        if( !(leptonPtr->isFO() && !leptonPtr->isTight()) ) continue;
        if( frflav==-1 && leptonPtr->isMuon() ){
	    frflav = 0;
        } else if( frflav==-1 && leptonPtr->isElectron() ) {
	    frflav = 1;
        } else{ frflav = 2; }
    }
    return frflav;
}
 
// main function //

std::map< std::string, double > eventFlattening::eventToEntry(Event& event, const double norm,
				const CombinedReweighter& reweighter,
				const std::string& selection_type, 
				const std::shared_ptr< TH2D>& frMap_muon, 
				const std::shared_ptr< TH2D>& frMap_electron,
				const std::string& variation,
				const bool doMVA,
				TMVA::Reader* reader){
    // fill one entry in outputTree (initialized with initOutputTree), based on the info of one event.
    // note that the event must be cleaned and processed by an event selection function first!

    //std::cout<<"----- new event -----"<<std::endl;   

    // re-initialize all variables in the map
    std::map< std::string, double > varmap = initVarMap();
 
    // sort leptons and jets by pt
    event.sortJetsByPt();
    event.sortLeptonsByPt();

    // event id variables 
    varmap["_runNb"] = event.runNumber();
    varmap["_lumiBlock"] = event.luminosityBlock();
    varmap["_eventNb"] = event.eventNumber();

    // other global precomputed event variables
    varmap["_numberOfVertices"] = event.numberOfVertices();

    // event weight (note: 0 for data!)
    varmap["_weight"] = event.weight();
    varmap["_scaledweight"] = event.weight()*norm;
    varmap["_normweight"] = event.weight()*norm;
    if(event.isData()){ 
	varmap["_scaledweight"] = 1;
	varmap["_normweight"] = 1;
    }
    if(event.isMC()){ 
	varmap["_normweight"] *= reweighter.totalWeight(event);
	varmap["_leptonreweight"] = reweighter["muonID"]->weight(event) 
				    * reweighter["electronID"]->weight(event);
	varmap["_nonleptonreweight"] = reweighter.totalWeight(event)/varmap["_leptonreweight"];
    }

    // in case of running in mode "fakerate", take into account fake rate weight
    if(selection_type=="fakerate"){
	double frweight = fakeRateWeight(event,frMap_muon,frMap_electron);
	if(event.isMC()) frweight *= -1;
	varmap["_normweight"] *= frweight;
	varmap["_fakerateweight"] = frweight;
	varmap["_fakeRateFlavour"] = fakeRateFlavour(event);
    }

    // get correct jet collection and met (defined in eventSelections.cc!)
    JetCollection jetcollection = getjetcollection(event, variation);
    JetCollection bjetcollection = jetcollection.mediumBTagCollection();
    Met met = getmet(event, variation);
    // get lepton collection as well (warning: a lot of event methods work on this collection implicitly,
    // so changing the definition here is not enough to consistently use another collection of leptons!)
    LeptonCollection lepcollection = event.leptonCollection();

    // number of muons and electrons
    varmap["_nMuons"] = lepcollection.numberOfMuons();
    varmap["_nElectrons"] = lepcollection.numberOfElectrons();

    // lepton pt and eta
    if(lepcollection.numberOfLightLeptons()==3 && lepcollection.size()==3){
	varmap["_leptonPtLeading"] = lepcollection[0].pt();
	varmap["_leptonPtSubLeading"] = lepcollection[1].pt();
	varmap["_leptonPtTrailing"] = lepcollection[2].pt();
	varmap["_leptonEtaLeading"] = lepcollection[0].eta();
        varmap["_leptonEtaSubLeading"] = lepcollection[1].eta();
        varmap["_leptonEtaTrailing"] = lepcollection[2].eta();
    }

    // other more or less precomputed event variables
    varmap["_lT"] = lepcollection.scalarPtSum() + met.pt();
    varmap["_HT"] = jetcollection.scalarPtSum();
    varmap["_nJets"] = jetcollection.size();
    varmap["_nBJets"] = bjetcollection.size();
    for(LeptonCollection::const_iterator lIt = lepcollection.cbegin();
	    lIt != lepcollection.cend(); lIt++){
        std::shared_ptr<Lepton> lep = *lIt;
        if(lep->isElectron()){
	    std::shared_ptr<Electron> ele = std::static_pointer_cast<Electron>(lep);
            if(ele->leptonMVAttH() < varmap["_leptonMVAttH_min"]){
		varmap["_leptonMVAttH_min"] = ele->leptonMVAttH();
	    }
            if(ele->leptonMVATOP() < varmap["_leptonMVATOP_min"]){
		varmap["_leptonMVATOP_min"] = ele->leptonMVATOP();
	    }
        }
        else if(lep->isMuon()){
	    std::shared_ptr<Muon> mu = std::static_pointer_cast<Muon>(lep);
            if(mu->leptonMVAttH() < varmap["_leptonMVAttH_min"]){
		 varmap["_leptonMVAttH_min"] = mu->leptonMVAttH();
	    }
            if(mu->leptonMVATOP() < varmap["_leptonMVATOP_min"]){
		 varmap["_leptonMVATOP_min"] = mu->leptonMVATOP();
	    }
        }
    }   

    int lWindex = 0;
    // set mT
    if(event.hasOSSFLightLeptonPair()){ 
	varmap["_MT"] = event.mtW(); 
    }
    //std::cout<<"checkpoint 1"<<std::endl;

    // find lepton from W and set its properties
    // (will this work on e.g. the ZZ control region where in principle no lepton from W is present?)
    if(event.hasOSSFLightLeptonPair()){ lWindex = event.WLeptonIndex(); }
    LeptonCollection::const_iterator lIt = lepcollection.cbegin();
    for(int i=0; i<lWindex; i++){++lIt;}
    Lepton& lW = **lIt;
    varmap["_lW_asymmetry"] = fabs(lW.eta())*lW.charge();
    //std::cout<<"checkpoint 2"<<std::endl;

    // find leptons from Z
    // NOT YET SAFE WHEN NO OSSF PAIR IS PRESENT
    //std::pair< int, int > zbosonresults = event.bestZBosonCandidateIndices();
    //int lZ1index = zbosonresults.first; int lZ2index = zbosonresults.second;
    //lIt = event.leptonCollection().cbegin();
    //for(int i=0; i<lZ1index; i++){++lIt;}
    //Lepton& lZ1 = **lIt;
    //lIt = event.leptonCollection().cbegin();
    //for(int i=0; i<lZ2index; i++){++lIt;}
    //Lepton& lZ2 = **lIt;
    //std::cout<<"___________"<<std::endl;
    //std::cout<<lW<<std::endl;
    //std::cout<<lZ1<<std::endl;
    //std::cout<<lZ2<<std::endl;
    //std::cout<<""<<std::endl;
    //std::cout<<"checkpoint 4"<<std::endl;
    
    // top reconstruction
    std::pair< double, double > pmz = pmzcandidates(lW, met);
    //std::cout<<"pmz: "<<pmz.first<<" "<<pmz.second<<std::endl;
    std::pair< double, int > topresults = besttopcandidate(jetcollection,
					    lW, met, pmz.first, pmz.second);
    //std::cout<<"top mass: "<<topresults.first<<std::endl;
    //std::cout<<"b-tagged jet: "<<topresults.second<<std::endl;
    int taggedbindex = topresults.second;
    if(jetcollection.numberOfMediumBTaggedJets()==0) taggedbindex = 0;
    //std::cout<<topresults.first<<std::endl;
    //std::cout<<"checkpoint 5"<<std::endl;

    // find index of recoiling jet
    int recoilindex = -1;
    for(JetCollection::const_iterator jIt = jetcollection.cbegin();
	    jIt != jetcollection.cend(); jIt++){
	if(jIt-jetcollection.cbegin() != taggedbindex){
	    recoilindex = jIt-jetcollection.cbegin();
	    break;
	}
    }
    //std::cout<<taggedbindex<<std::endl;
    //std::cout<<recoilindex<<std::endl;
    //std::cout<<"checkpoint 6"<<std::endl;

    // loop over jets and find relevant quantities
    for(JetCollection::const_iterator jIt = jetcollection.cbegin();
        jIt != jetcollection.cend(); jIt++){
	Jet& jet = **jIt;
        // if abs(eta) is larger than max, modify max
        if(fabs(jet.eta())>varmap["_abs_eta_max"]) varmap["_abs_eta_max"] = fabs(jet.eta());
        // if deepCSV is higher than maximum, modify max
        if(jet.deepCSV()>varmap["_deepCSV_max"] and jet.inBTagAcceptance()){
	     varmap["_deepCSV_max"] = jet.deepCSV();
	}
	if(jet.deepFlavor()>varmap["_deepFlavor_max"] and jet.inBTagAcceptance()){
	    varmap["_deepFlavor_max"] = jet.deepFlavor();
	}
        // find maximum dijet mass and pt
        for(JetCollection::const_iterator jIt2 = jIt+1; jIt2 != jetcollection.cend(); jIt2++){
            Jet& jet2 = **jIt2;
            if((jet+jet2).mass()>varmap["_Mjj_max"]) varmap["_Mjj_max"] = (jet+jet2).mass();
            if((jet+jet2).pt()>varmap["_pTjj_max"]) varmap["_pTjj_max"] = (jet+jet2).pt();
        } 
    }
    //std::cout<<"checkpoint 7"<<std::endl;
    if(recoilindex>=0 and jetcollection.size()>0){
	JetCollection::const_iterator jIt = jetcollection.cbegin();
	for(int i=0; i<recoilindex; i++){jIt++;}
	Jet& recoiljet = **jIt;
	varmap["_dRlWrecoil"] = deltaR(lW,recoiljet);
	varmap["_abs_eta_recoil"] = fabs(recoiljet.eta());
    }
    if(taggedbindex>=0 and jetcollection.size()>0){
	JetCollection::const_iterator tbjIt = jetcollection.cbegin();
	for(int i=0; i<taggedbindex; i++){tbjIt++;}
	Jet& taggedbjet = **tbjIt;
	varmap["_dRlWbtagged"] = deltaR(lW,taggedbjet);
    }
    //std::cout<<"checkpoint 8"<<std::endl;

    // loop over leptons and find some kinematic properties
    PhysicsObject l3vec;
    for(LeptonCollection::const_iterator lIt = lepcollection.cbegin();
        lIt != lepcollection.cend(); lIt++){
        Lepton& lep = **lIt;
        for(LeptonCollection::const_iterator lIt2 = lIt+1; lIt2 != lepcollection.cend(); lIt2++){
            Lepton& lep2 = **lIt2;
            if(deltaPhi(lep,lep2)>varmap["_dPhill_max"]) varmap["_dPhill_max"] = deltaPhi(lep,lep2);
        }
        for(JetCollection::const_iterator jIt = bjetcollection.cbegin(); 
	    jIt != bjetcollection.cend(); jIt++){
            Jet& bjet = **jIt;
            if(deltaR(lep,bjet)<varmap["_dRlb_min"]) varmap["_dRlb_min"] = deltaR(lep,bjet);
        }
    }
    varmap["_M3l"] = event.leptonSystem().mass();
    //std::cout<<"checkpoint 9"<<std::endl;

    // at this point all variables have been calculated; now set TTree and TReader variables
    setVariables(varmap);
    // based on these variables: evaluate the BDT output if requested
    if(doMVA){
	/*std::cout << "some variables: " << std::endl;
	std::cout << _abs_eta_recoil << std::endl; 
	std::cout << _Mjj_max << std::endl;
	std::cout << _lW_asymmetry << std::endl;
	std::cout << _deepCSV_max;
	std::cout << _lT << std::endl;
	std::cout << _MT << std::endl;;
	std::cout << _dPhill_max << std::endl;
	std::cout << _pTjj_max << std::endl;
	std::cout << _dRlb_min << std::endl;
	std::cout << _HT << std::endl;
	std::cout << _dRlWrecoil << std::endl;
	std::cout << _dRlWbtagged << std::endl;
	std::cout << _M3l << std::endl;
	std::cout << _abs_eta_max << std::endl;
	std::cout << _nJets << std::endl;
	std::cout << _nBJets << std::endl;*/
	varmap["_eventBDT"] = reader->EvaluateMVA( "BDT" );
	// also reset the TTree variable _eventBDT!
	_eventBDT = varmap["_eventBDT"];
    }
    // now return the varmap (e.g. to fill histograms)
    return varmap;
}

std::pair<double,double> eventFlattening::pmzcandidates(Lepton& lW, Met& met){
    // this method returns two candidates for longitudinal component of missing momentum,
    // by imposing the W mass constraint on the system (lW,pmiss).
    
    // first define all four-vector quantities
    double El = lW.energy(); double plx = lW.px(); double ply = lW.py(); double plz = lW.pz();
    double pmx = met.px(); double pmy = met.py(); 
    double mW = particle::mW;
    std::pair<double,double> pmz = {0,0};
    
    //std::cout<<"lepton: "<<El<<" "<<plx<<" "<<ply<<" "<<plz<<std::endl;
    //std::cout<<"ptmiss: "<<pmx<<" "<<pmy<<std::endl;
    
    // then solve quadratic equation
    double A = El*El - plz*plz;
    double B = -plz*(2*(plx*pmx+ply*pmy)+mW*mW);
    double C = El*El*(pmx*pmx+pmy*pmy) - std::pow(mW*mW/2,2) - std::pow(plx*pmx+ply*pmy,2);
    C += -mW*mW*(plx*pmx+ply*pmy);
    double discr = B*B - 4*A*C;
    if(discr<0){
        //std::cout<<"### WARNING ###: negative discriminant found."<<std::endl;
        discr = 0;
    }
    pmz.first = (-B + std::sqrt(discr))/(2*A);
    pmz.second = (-B - std::sqrt(discr))/(2*A);
    return pmz;
}

// Willems equivalent method:
/*std::pair<double,double> pmzcandidates(Event& event, Lepton& lW){
    double pl = lW.energy(); double plt = lW.pt(); double plz = lW.pz();
    double pmt = event.met().pt();
    double mW = particle::mW;
    std::pair<double,double> pmz = {0,0};

    double a = 0.5*mW*mW + lW.px()*event.met().px() + lW.py()*event.met().py();
    double D = 1-plt*plt*pmt*pmt/(a*a);
    double sqrtD = 0;
    if(D>0) sqrtD = std::sqrt(D);
    else std::cout<<"### WARNING ###"<<std::endl;
    pmz.first = a*(plz + pl*sqrtD)/(plt*plt);
    pmz.second = a*(plz - pl*sqrtD)/(plt*plt);
    std::cout<<pmz.first<<std::endl;
    std::cout<<pmz.second<<std::endl;
    return pmz;
}*/

std::pair<double,int> eventFlattening::besttopcandidate(JetCollection& alljets, Lepton& lW, 
					Met& met, double pmz1, double pmz2){
    // This method returns the reconstruced top mass closest to its nominal mass,
    // by combining the four-vectors of the lepton from W, the two missing momentum candidates,
    // and all medium b-jets in the event.
    // The index of the jet that gives the best top mass is returned as well.
    
    std::pair<double,int> res = {0.,0};
    double bestmass = 0.;
    double massdiff = fabs(particle::mTop-bestmass);
    int bindex = -1;
    // set lorentz vectors for lepton and neutrino
    double metpx = met.px();
    double metpy = met.py();  
    LorentzVector nu1;
    nu1.setPxPyPzE(metpx,metpy,pmz1,std::sqrt(metpx*metpx+metpy*metpy+pmz1*pmz1));
    LorentzVector nu2;
    nu2.setPxPyPzE(metpx,metpy,pmz2,std::sqrt(metpx*metpx+metpy*metpy+pmz2*pmz2));
    LorentzVector lep;
    lep.setPxPyPzE(lW.px(),lW.py(),lW.pz(),lW.energy());
    // loop over jets
    LorentzVector jet;
    double mass = 0.;
    for(JetCollection::const_iterator jIt = alljets.cbegin(); jIt != alljets.cend(); jIt++){
        Jet& jetobject = **jIt;
	// consider only medium tagged b-jets
	if(!jetobject.isBTaggedMedium()) continue;
        jet.setPxPyPzE(jetobject.px(),jetobject.py(),jetobject.pz(),jetobject.energy());
	// try neutrino hypothesis one
        mass = (nu1 + lep + jet).mass();
        if(fabs(particle::mTop-mass)<massdiff){
            bestmass = mass;
            massdiff = fabs(particle::mTop-mass);
            bindex = jIt - alljets.cbegin();
        }
	// try neutrino hypothesis two
        mass = (nu2 + lep + jet).mass();
        if(fabs(particle::mTop-mass)<massdiff){
            bestmass = mass;
            massdiff = fabs(particle::mTop-mass);
            bindex = jIt - alljets.cbegin();
        }
    }
    res.first = bestmass;
    res.second = bindex;
    return res;   
}   
