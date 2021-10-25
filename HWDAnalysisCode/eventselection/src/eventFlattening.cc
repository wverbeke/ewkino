/*
Definition of event variables for H->W+Ds analysis
*/


// include header
#include "../interface/eventFlattening.h"


// definition of the variables used for filling the ouput TTree
// (and evaluating an TMVA::Reader if requested)
// event id variables
ULong_t _runNb = 0;
ULong_t _lumiBlock = 0;
ULong_t _eventNb = 0;
// event weight for simulation
Float_t _weight = 0; // generator weight
Float_t _scaledWeight = 0; // generator weight scaled by cross section and lumi
Float_t _totWeight = 0; // total weight, including scaled weight, reweighting and fake rate
Float_t _leptonReweight = 1; // lepton reweighting factor
Float_t _nonLeptonReweight = 1; // all other reweighting factors
Float_t _fakeRateWeight = 0; // fake rate reweighting factor
// event BDT variables
// NOTE: BDT still to be designed for this analysis so not sure yet...
Float_t _leptonAsymmetry = 0;
Float_t _deepCSVMax = 0;
Float_t _deepFlavorMax = 0;
Float_t _lT = 0;
Float_t _MT = 0;
Float_t _HT = 0;
Float_t _nJets = 0;
Float_t _nBJets = 0;
// BDT output score
Float_t _eventBDT = 0.;
// other variables
Int_t _nMuons = 0;
Int_t _nElectrons = 0;
Float_t _leptonMVATOPMin = 1.;
Float_t _yield = 0.5; // fixed value
Float_t _leptonPtLeading = 0.;
Float_t _leptonEtaLeading = 0.;
Int_t _leptonChargeLeading = 0;
Float_t _numberOfVertices = 0.;
Int_t _fakeRateFlavour = -1;


double eventFlattening::getVariable( const std::map<std::string,double>& varmap, 
				     const std::string& varname ){
    // get a variable from the varmap
    std::map<std::string,double>::const_iterator it = varmap.find(varname);
    if(it == varmap.end()){
	std::string msg = "ERROR in eventFlattening:getVariable:";
	msg += " variable "+varname+" not found in map.";
	throw std::invalid_argument(msg);
    }
    return it->second; 
}


void eventFlattening::setVariables(std::map<std::string,double> varmap){
    // copy (and parse) the values contained in varmap to the TTree variables

    _runNb = (unsigned long) getVariable( varmap, "_runNb" );
    _lumiBlock = (unsigned long) getVariable( varmap, "_lumiBlock" );
    _eventNb = (unsigned long) getVariable( varmap, "_eventNb" );

    _weight = getVariable( varmap, "_weight" );
    _scaledWeight = getVariable( varmap, "_scaledWeight" );
    _totWeight = getVariable( varmap, "_totWeight" );
    _leptonReweight = getVariable( varmap, "_leptonReweight" );
    _nonLeptonReweight = getVariable( varmap, "_nonLeptonReweight" );
    _fakeRateWeight = getVariable( varmap, "_fakeRateWeight" );

    _leptonAsymmetry = getVariable( varmap ,"_leptonAsymmetry" );
    _deepCSVMax = getVariable( varmap, "_deepCSVMax" );
    _deepFlavorMax = getVariable( varmap, "_deepFlavorMax" );
    _lT = getVariable( varmap, "_lT" );
    _MT = getVariable( varmap, "_MT" );
    _HT = getVariable( varmap, "_HT" );
    _nJets = getVariable( varmap, "_nJets" );
    _nBJets = getVariable( varmap, "_nBJets" );
    
    _eventBDT = getVariable( varmap, "_eventBDT" );

    _nMuons = (int) getVariable( varmap, "_nMuons" );
    _nElectrons = (int) getVariable( varmap, "_nElectrons" );
    _leptonMVATOPMin = getVariable( varmap, "_leptonMVATOPMin" );
    _yield = getVariable( varmap, "_yield" );
    _leptonPtLeading = getVariable( varmap, "_leptonPtLeading" );
    _leptonEtaLeading = getVariable( varmap, "_leptonEtaLeading" );
    _leptonChargeLeading = getVariable( varmap, "_leptonChargeLeading" );
    _numberOfVertices = getVariable( varmap, "_numberOfVertices" );
    _fakeRateFlavour = (int) getVariable( varmap, "_fakeRateFlavour" );
}

std::map< std::string, double > eventFlattening::initVarMap(){
    // initialize a map of variables set to their default values
    std::map< std::string, double> varmap = {
    {"_runNb", 0},{"_lumiBlock",0},{"_eventNb",0},

    {"_weight",0},{"_scaledWeight",0},{"_totWeight",0},
    {"_leptonReweight",1},{"_nonLeptonReweight",1},{"_fakeRateWeight",0},

    {"_leptonAsymmetry",0},
    {"_deepCSVMax",0},{"_deepFlavorMax",0},
    {"_lT",0},{"_MT",0},{"_HT",0},
    {"_nJets",0},{"_nBJets",0},

    {"_eventBDT",0},
    
    {"_nMuons",0},{"_nElectrons",0},
    
    {"_leptonMVATOPMin",1.},
    
    {"_yield",0.5},
    
    {"_leptonPtLeading",0.},
    {"_leptonEtaLeading",0.},
    {"_leptonChargeLeading",0.},

    {"_numberOfVertices",0},
    
    {"_fakeRateFlavour",-1},
    
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
    outputTree->Branch("_scaledWeight", &_scaledWeight, "_scaledWeight/F");
    outputTree->Branch("_totWeight", &_totWeight, "_totWeight/F");
    outputTree->Branch("_leptonReweight", &_leptonReweight, "_leptonReweight/F");
    outputTree->Branch("_nonLeptonReweight", &_nonLeptonReweight, "_nonLeptonReweight/F");
    outputTree->Branch("_fakeRateWeight", &_fakeRateWeight, "_fakeRateWeight/F"); 
   
    // event BDT variables
    outputTree->Branch("_leptonAsymmetry", &_leptonAsymmetry, "_leptonAsymmetry/F");
    outputTree->Branch("_deepCSVMax", &_deepCSVMax, "_deepCSVMax/F");
    outputTree->Branch("_deepFlavorMax", &_deepFlavorMax, "_deepFlavorMax/F");
    outputTree->Branch("_lT", &_lT, "_lT/F");
    outputTree->Branch("_MT", &_MT, "_MT/F");
    outputTree->Branch("_HT", &_HT, "_HT/F");
    outputTree->Branch("_nJets", &_nJets, "_nJets/F");
    outputTree->Branch("_nBJets", &_nBJets, "_nBJets/F");

    // BDT output score (initialized here but filled in calling function!)
    outputTree->Branch("_eventBDT", &_eventBDT, "_eventBDT/F");

    // other variables
    outputTree->Branch("_nMuons", &_nMuons, "_nMuons/I");
    outputTree->Branch("_nElectrons", &_nElectrons, "_nElectrons/I");
    outputTree->Branch("_leptonMVATOPMin", &_leptonMVATOPMin, "_leptonMVATOPMin/F");
    outputTree->Branch("_yield", &_yield, "_yield/F");
    outputTree->Branch("_leptonPtLeading", &_leptonPtLeading, "_leptonPtLeading/F");
    outputTree->Branch("_leptonEtaLeading", &_leptonEtaLeading, "_leptonEtaLeading/F");
    outputTree->Branch("_leptonChargeLeading", &_leptonChargeLeading, "_leptonChargeLeading/F");
    outputTree->Branch("_numberOfVertices", &_numberOfVertices, "_numberOfVertices/F");
    outputTree->Branch("_fakeRateFlavour", &_fakeRateFlavour, "_fakeRateFlavour/I");
}

TMVA::Reader* eventFlattening::initializeReader( TMVA::Reader* reader, 
	const std::string& pathToXMLFile ){
    // make sure it is consistent with bdt training!
    // NOTE: BDT still to be designed for this analysis, so will probably change...
    reader->AddVariable("_leptonAsymmetry", &_leptonAsymmetry);
    reader->AddVariable("_deepCSVMax",&_deepCSVMax);
    reader->AddVariable("_lT", &_lT);
    reader->AddVariable("_MT", &_MT);
    reader->AddVariable("_HT", &_HT);
    reader->AddVariable("_nJets", &_nJets);
    reader->AddVariable("_nBJets", &_nBJets);

    reader->BookMVA("BDT", pathToXMLFile);
    return reader;
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
    // fill one entry in outputTree (initialized with initOutputTree), 
    // based on the info of one event.
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

    // event weights
    varmap["_weight"] = event.weight();
    varmap["_scaledWeight"] = event.weight()*norm;
    varmap["_totWeight"] = event.weight()*norm;
    if(event.isData()){ 
	// set event weights for data specifically
	// note that event.weight() gives 0 for data!
	varmap["_scaledWeight"] = 1;
	varmap["_totWeight"] = 1;
    }
    if(event.isMC()){ 
	// set event weights for simulation specifically
	varmap["_totWeight"] *= reweighter.totalWeight(event);
	varmap["_leptonReweight"] = reweighter["muonID"]->weight(event) 
				    * reweighter["electronID"]->weight(event);
	if(std::fabs(varmap["_leptonReweight"])>0){
	    varmap["_nonLeptonReweight"] = reweighter.totalWeight(event)/varmap["_leptonReweight"];
	}
    }

    // in case of running in mode "fakerate", take into account fake rate weight
    if(selection_type=="fakerate"){
	double frweight = fakeRateTools::fakeRateWeight(event,frMap_muon,frMap_electron);
	if(event.isMC()) frweight *= -1;
	varmap["_totWeight"] *= frweight;
	varmap["_fakeRateWeight"] = frweight;
	varmap["_fakeRateFlavour"] = fakeRateTools::fakeRateFlavour(event);
    }

    // get correct jet collection and MET
    JetCollection jetcollection = event.getJetCollection(variation);
    JetCollection bjetcollection = jetcollection.mediumBTagCollection();
    Met met = event.getMet(variation);
    // get lepton collection as well 
    // (warning: a lot of event methods work on this collection implicitly,
    // so changing the definition here is not enough!)
    LeptonCollection lepcollection = event.leptonCollection();

    // number of muons and electrons
    varmap["_nMuons"] = lepcollection.numberOfMuons();
    varmap["_nElectrons"] = lepcollection.numberOfElectrons();

    // lepton pt, eta and charge
    varmap["_leptonPtLeading"] = lepcollection[0].pt();
    varmap["_leptonEtaLeading"] = lepcollection[0].eta();
    varmap["_leptonChargeLeading"] = lepcollection[0].charge();
    varmap["_leptonAsymmetry"] = std::fabs(lepcollection[0].eta())*lepcollection[0].charge();
    
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
	    if(ele->leptonMVATOP() < varmap["_leptonMVATOP_min"]){
	    varmap["_leptonMVATOP_min"] = ele->leptonMVATOP();
	    }
        }
        else if(lep->isMuon()){
	    std::shared_ptr<Muon> mu = std::static_pointer_cast<Muon>(lep);
	    if(mu->leptonMVATOP() < varmap["_leptonMVATOP_min"]){
	    varmap["_leptonMVATOP_min"] = mu->leptonMVATOP();
	    }
        }
    }
    varmap["_MT"] = event.mtW();

    // loop over jets and find relevant quantities
    for(JetCollection::const_iterator jIt = jetcollection.cbegin();
	    jIt != jetcollection.cend(); jIt++){
	Jet& jet = **jIt;
        // if deepCSV is higher than maximum, modify max
        if(jet.deepCSV()>varmap["_deepCSVMax"] and jet.inBTagAcceptance()){
	    varmap["_deepCSVMax"] = jet.deepCSV();
	}
	if(jet.deepFlavor()>varmap["_deepFlavorMax"] and jet.inBTagAcceptance()){
	    varmap["_deepFlavorMax"] = jet.deepFlavor();
	}
    }

    // at this point all variables have been calculated; now set TTree and TReader variables
    setVariables(varmap);
    // based on these variables: evaluate the BDT output if requested
    if(doMVA){

	// printouts for testing
	/*std::cout << "some variables: " << std::endl;
	std::cout << _leptonAsymmetry << std::endl;
	std::cout << _deepCSVMax;
	std::cout << _lT << std::endl;
	std::cout << _MT << std::endl;;
	std::cout << _HT << std::endl;
	std::cout << _nJets << std::endl;
	std::cout << _nBJets << std::endl;*/
	
	varmap["_eventBDT"] = reader->EvaluateMVA( "BDT" );
	// also reset the TTree variable _eventBDT!
	_eventBDT = varmap["_eventBDT"];
    }
    
    // now return the varmap (e.g. to fill histograms)
    return varmap;
}
