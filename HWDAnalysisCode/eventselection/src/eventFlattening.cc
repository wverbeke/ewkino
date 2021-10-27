/*
Definition of event variables for H->W+Ds analysis
*/


// include header
#include "../interface/eventFlattening.h"


// definition of the variables used for filling the ouput TTree
// (and evaluating an TMVA::Reader if requested)
// note: need to use Float_t for any variable that might be used in TMVA::BDT

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

// event-level variables
Float_t _deepCSVMax = 0;
Float_t _deepFlavorMax = 0;
Float_t _lT = 0;
Float_t _MT = 0;
Float_t _HT = 0;
Float_t _nJets = 0;
Float_t _nBJets = 0;
Float_t _nMuons = 0;
Float_t _nElectrons = 0;
Float_t _leptonMVATOPMin = 1.;
Float_t _yield = 0.5; // fixed value
Float_t _numberOfVertices = 0.;
Float_t _fakeRateFlavour = -1;

// lepton variables
Float_t _leptonPt = 0.;
Float_t _leptonEta = 0.;
Float_t _leptonPhi = 0;
Float_t _leptonCharge = 0;
Float_t _leptonAsymmetry = 0;

// met/neutrino variables
Float_t _metPt = 0;
Float_t _metPhi = 0;
Float_t _nuPz = 0;

// Ds variables
Float_t _dsPt = 0;
Float_t _dsEta = 0;
Float_t _dsPhi = 0;
Float_t _dsMass = 0;
Int_t _dsHasFastGenMatch = -1;

// Higgs variables
Float_t _hPt = 0;
Float_t _hEta = 0;
Float_t _hPhi = 0;
Float_t _hMass = 0;

// BDT output score
Float_t _eventBDT = 0.;

// internal variables
Bool_t _hasValidDs = 0; // event has exactly one Ds
Bool_t _hasValidLepton = 0; // event has exactly one lepton
Bool_t _hasValidNu = 0; // neutrino solutions are valid


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

    // event id variables
    _runNb = (unsigned long) getVariable( varmap, "_runNb" );
    _lumiBlock = (unsigned long) getVariable( varmap, "_lumiBlock" );
    _eventNb = (unsigned long) getVariable( varmap, "_eventNb" );

    // event weight for simulation
    _weight = getVariable( varmap, "_weight" );
    _scaledWeight = getVariable( varmap, "_scaledWeight" );
    _totWeight = getVariable( varmap, "_totWeight" );
    _leptonReweight = getVariable( varmap, "_leptonReweight" );
    _nonLeptonReweight = getVariable( varmap, "_nonLeptonReweight" );
    _fakeRateWeight = getVariable( varmap, "_fakeRateWeight" );

    // event-level variables
    _deepCSVMax = getVariable( varmap, "_deepCSVMax" );
    _deepFlavorMax = getVariable( varmap, "_deepFlavorMax" );
    _lT = getVariable( varmap, "_lT" );
    _MT = getVariable( varmap, "_MT" );
    _HT = getVariable( varmap, "_HT" );
    _nJets = getVariable( varmap, "_nJets" );
    _nBJets = getVariable( varmap, "_nBJets" );
    _nMuons = getVariable( varmap, "_nMuons" );
    _nElectrons = getVariable( varmap, "_nElectrons" );
    _leptonMVATOPMin = getVariable( varmap, "_leptonMVATOPMin" );
    _yield = getVariable( varmap, "_yield" );
    _numberOfVertices = getVariable( varmap, "_numberOfVertices" );
    _fakeRateFlavour = getVariable( varmap, "_fakeRateFlavour" );
   
    // lepton variables
    _leptonPt = getVariable( varmap, "_leptonPt" );
    _leptonEta = getVariable( varmap, "_leptonEta" );
    _leptonPhi = getVariable( varmap, "_leptonPhi" );
    _leptonCharge = getVariable( varmap, "_leptonCharge" );
    _leptonAsymmetry = getVariable( varmap ,"_leptonAsymmetry" );

    // met/neutrino variables
    _metPt = getVariable( varmap, "_metPt" );
    _metPhi = getVariable( varmap, "_metPhi");
    _nuPz = getVariable( varmap, "_nuPz" );

    // Ds variables
    _dsPt = getVariable( varmap, "_dsPt" );
    _dsEta = getVariable( varmap, "_dsEta" );
    _dsPhi = getVariable( varmap, "_dsPhi" );
    _dsMass = getVariable( varmap, "_dsMass" );
    _dsHasFastGenMatch = (int) getVariable( varmap, "_dsHasFastGenMatch" );

    // Higgs variables
    _hPt = getVariable( varmap, "_hPt" );
    _hEta = getVariable( varmap, "_hEta" );
    _hPhi = getVariable( varmap, "_hPhi" );
    _hMass = getVariable( varmap, "_hMass" );

    // BDT output score 
    _eventBDT = getVariable( varmap, "_eventBDT" );

    // internal variables
    _hasValidDs = (bool) getVariable( varmap, "_hasValidDs" );
    _hasValidLepton = (bool) getVariable( varmap, "_hasValidLepton" );
    _hasValidNu = (bool) getVariable( varmap, "_hasValidNu" );
}


std::map< std::string, double > eventFlattening::initVarMap(){
    // initialize a map of variables set to their default values
    std::map< std::string, double> varmap = {
    {"_runNb", 0},
    {"_lumiBlock",0},
    {"_eventNb",0},

    {"_weight",0},
    {"_scaledWeight",0},
    {"_totWeight",0},
    {"_leptonReweight",1},
    {"_nonLeptonReweight",1},
    {"_fakeRateWeight",0},

    {"_deepCSVMax",0},
    {"_deepFlavorMax",0},
    {"_lT",0},
    {"_MT",0},
    {"_HT",0},
    {"_nJets",0},
    {"_nBJets",0},
    {"_nMuons",0},
    {"_nElectrons",0},
    {"_leptonMVATOPMin",1.},
    {"_yield",0.5},
    {"_numberOfVertices",0},    
    {"_fakeRateFlavour",-1},

    {"_leptonPt",0.},
    {"_leptonEta",0.},
    {"_leptonPhi",0.},
    {"_leptonCharge",0.},
    {"_leptonAsymmetry",0.},
    
    {"_metPt",0.},
    {"_metPhi",0.},
    {"_nuPz",0.},

    {"_dsPt",0.},
    {"_dsEta",0.},
    {"_dsPhi",0.},
    {"_dsMass",0.},
    {"_dsHasFastGenMatch",-1},

    {"_hPt",0.},
    {"_hEta",0.},
    {"_hPhi",0.},
    {"_hMass",0.},

    {"_eventBDT",0},
    
    {"_hasValidDs",0}, 
    {"_hasValidLepton",0},
    {"_hasValidNu",0},
    
    };
    return varmap;    
}


void eventFlattening::initOutputTree(TTree* outputTree){
    // set branches for a flat output tree 
    
    // event id variables
    outputTree->Branch("_runNb", &_runNb, "_runNb/l");
    outputTree->Branch("_lumiBlock", &_lumiBlock, "_lumiBlock/l");
    outputTree->Branch("_eventNb", &_eventNb, "_eventNb/l");

    // event weight for simulation
    outputTree->Branch("_weight", &_weight, "_weight/F");
    outputTree->Branch("_scaledWeight", &_scaledWeight, "_scaledWeight/F");
    outputTree->Branch("_totWeight", &_totWeight, "_totWeight/F");
    outputTree->Branch("_leptonReweight", &_leptonReweight, "_leptonReweight/F");
    outputTree->Branch("_nonLeptonReweight", &_nonLeptonReweight, "_nonLeptonReweight/F");
    outputTree->Branch("_fakeRateWeight", &_fakeRateWeight, "_fakeRateWeight/F"); 
   
    // event-level
    outputTree->Branch("_deepCSVMax", &_deepCSVMax, "_deepCSVMax/F");
    outputTree->Branch("_deepFlavorMax", &_deepFlavorMax, "_deepFlavorMax/F");
    outputTree->Branch("_lT", &_lT, "_lT/F");
    outputTree->Branch("_MT", &_MT, "_MT/F");
    outputTree->Branch("_HT", &_HT, "_HT/F");
    outputTree->Branch("_nJets", &_nJets, "_nJets/F");
    outputTree->Branch("_nBJets", &_nBJets, "_nBJets/F");
    outputTree->Branch("_nMuons", &_nMuons, "_nMuons/F");
    outputTree->Branch("_nElectrons", &_nElectrons, "_nElectrons/F");
    outputTree->Branch("_leptonMVATOPMin", &_leptonMVATOPMin, "_leptonMVATOPMin/F");
    outputTree->Branch("_yield", &_yield, "_yield/F");
    outputTree->Branch("_numberOfVertices", &_numberOfVertices, "_numberOfVertices/F");
    outputTree->Branch("_fakeRateFlavour", &_fakeRateFlavour, "_fakeRateFlavour/F");

    // lepton variables
    outputTree->Branch("_leptonPt", &_leptonPt, "_leptonPt/F");
    outputTree->Branch("_leptonEta", &_leptonEta, "_leptonEta/F");
    outputTree->Branch("_leptonPhi", &_leptonPhi, "_leptonPhi/F");
    outputTree->Branch("_leptonAsymmetry", &_leptonAsymmetry, "_leptonAsymmetry/F");

    // met/neutrino variables
    outputTree->Branch("_metPt", &_metPt, "_metPt/F");
    outputTree->Branch("_metPhi", &_metPhi, "_metPhi/F");
    outputTree->Branch("_nuPz", &_nuPz, "_nuPz/F");

    // Ds variables
    outputTree->Branch("_dsPt", &_dsPt, "_dsPt/F");
    outputTree->Branch("_dsEta", &_dsEta, "_dsEta/F");
    outputTree->Branch("_dsPhi", &_dsPhi, "_dsPhi/F");
    outputTree->Branch("_dsMass", &_dsMass, "_dsMass/F");
    outputTree->Branch("_dsHasFastGenMatch", &_dsHasFastGenMatch, "_dsHasFastGenMatch/I");
 
    // Higgs variables
    outputTree->Branch("_hPt", &_hPt, "_hPt/F");
    outputTree->Branch("_hEta", &_hEta, "_hEta/F");
    outputTree->Branch("_hPhi", &_hPhi, "_hPhi/F");
    outputTree->Branch("_hMass", &_hMass, "_hMass/F");

    // BDT output score
    outputTree->Branch("_eventBDT", &_eventBDT, "_eventBDT/F");

    // internal variables
    outputTree->Branch("_hasValidDs", &_hasValidDs, "_hasValidDs/O");
    outputTree->Branch("_hasValidLepton", &_hasValidLepton, "_hasValidLepton/O");
    outputTree->Branch("_hasValidNu", &_hasValidNu, "_hasValidNu/O");
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

std::map< std::string, double > eventFlattening::eventToEntry(
		Event& event, 
		const double norm,
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

    // initializations
    bool hasvalidds = (event.dmesonCollection().size()==1);
    if( hasvalidds ) varmap["_hasValidDs"] = 1;
    // (note: to decide whether it is best to put ==1 or >0)
    bool hasvalidlepton = (event.leptonCollection().size()==1);
    if( hasvalidlepton ) varmap["_hasValidLepton"] = 1;
    // (note: to decide whether it is best to put ==1 or >0)

    // sort leptons and jets by pt
    event.sortJetsByPt();
    event.sortLeptonsByPt();

    // event id variables 
    varmap["_runNb"] = event.runNumber();
    varmap["_lumiBlock"] = event.luminosityBlock();
    varmap["_eventNb"] = event.eventNumber();

    // event weights
    varmap["_weight"] = event.weight();
    varmap["_scaledWeight"] = event.weight()*norm;
    varmap["_totWeight"] = event.weight()*norm;
    if(event.isData()){ 
	// set event weights for data specifically
	// note that event.weight() gives 0 for data!
	varmap["_weight"] = 1;
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
    if(selection_type=="fakerate"){
	// in case of running in mode "fakerate", take into account fake rate weight
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
    // get lepton collection 
    // (warning: a lot of event methods work on this collection implicitly,
    // so changing the definition here is not enough!)
    LeptonCollection lepcollection = event.leptonCollection();
    // get dmeson collection
    DMesonCollection dmesoncollection = event.dmesonCollection();

    // directly available lepton properties
    if( hasvalidlepton ){
        varmap["_leptonPt"] = lepcollection[0].pt();
        varmap["_leptonEta"] = lepcollection[0].eta();
	varmap["_leptonPhi"] = lepcollection[0].phi();
        varmap["_leptonCharge"] = lepcollection[0].charge();
        varmap["_leptonAsymmetry"] = std::fabs(lepcollection[0].eta())*lepcollection[0].charge();
        varmap["_MT"] = mt( lepcollection[0], met );
        // note: cannot use event.mtW() as it is only defined for events with a Z candidate...
        //       instead, use mt() as defined in the PhysicsObject class
    }

    // directly available MET properties
    varmap["_metPt"] = met.pt();
    varmap["_metPhi"] = met.phi();

    // directly available Ds meson properties
    if( hasvalidds ){
	varmap["_dsPt"] = dmesoncollection[0].pt();
	varmap["_dsEta"] = dmesoncollection[0].eta();
	varmap["_dsPhi"] = dmesoncollection[0].phi();
	varmap["_dsMass"] = dmesoncollection[0].invMass();
	varmap["_dsHasFastGenMatch"] = (double) dmesoncollection[0].hasFastGenMatch();
    }

    // other more or less precomputed event variables
    varmap["_lT"] = lepcollection.scalarPtSum() + met.pt();
    varmap["_HT"] = jetcollection.scalarPtSum();
    varmap["_nJets"] = jetcollection.size();
    varmap["_nBJets"] = bjetcollection.size();
    varmap["_nMuons"] = lepcollection.numberOfMuons();
    varmap["_nElectrons"] = lepcollection.numberOfElectrons();
    varmap["_numberOfVertices"] = event.numberOfVertices();

    // loop over leptons and find maximum/minimum quantities
    for(LeptonCollection::const_iterator lIt = lepcollection.cbegin();
	    lIt != lepcollection.cend(); lIt++){
        std::shared_ptr<Lepton> lep = *lIt;
	if(lep->isLightLepton()){
	    std::shared_ptr<LightLepton> llep = std::static_pointer_cast<LightLepton>(lep);
	    if(llep->leptonMVATOP() < getVariable( varmap, "_leptonMVATOPMin" ) ){
                varmap["_leptonMVATOPMin"] = llep->leptonMVATOP();
            }
	}
    }

    // loop over jets and find maximum/miinimum quantities
    for(JetCollection::const_iterator jIt = jetcollection.cbegin();
	    jIt != jetcollection.cend(); jIt++){
	Jet& jet = **jIt;
        if(jet.deepCSV()>getVariable(varmap, "_deepCSVMax") and jet.inBTagAcceptance()){
	    varmap["_deepCSVMax"] = jet.deepCSV();
	}
	if(jet.deepFlavor()>getVariable(varmap, "_deepFlavorMax") and jet.inBTagAcceptance()){
	    varmap["_deepFlavorMax"] = jet.deepFlavor();
	}
    }

    // do neutrino reconstruction
    if( hasvalidlepton ){
	std::tuple<bool, std::pair<double,double>> nusol;
	nusol = nuReco::pNuZCandidates( 
	    lepcollection[0], 0, met, particle::mW );
	bool hasvalidnu = std::get<0>(nusol);
	varmap["_hasValidNu"] = (double) hasvalidnu;

	// do higgs reconstruction
	if( hasvalidds ){
	    std::pair<PhysicsObject,PhysicsObject> temp;
	    temp = bestHiggsAndNeutrinoCandidate( lepcollection[0], met, 
						  std::get<1>(nusol), dmesoncollection[0] );
	    PhysicsObject hCand = temp.first;
	    PhysicsObject nuCand = temp.second;
	    varmap["_hPt"] = hCand.pt();
	    varmap["_hEta"] = hCand.eta();
	    varmap["_hPhi"] = hCand.phi();
	    varmap["_hMass"] = hCand.mass();
	    varmap["_nuPz"] = nuCand.pz();
	    
	    // printouts for testing
	    if( hCand.mass()<0 ){
		std::cout << "found higgs with negative mass" << std::endl;
		std::cout << "higgs: " << hCand.px() << ", " << hCand.py() << ", ";
		std::cout << hCand.pz() << ", " << hCand.energy() << ", ";
		std::cout << hCand.mass() << std::endl;
		std::cout << "lepton: " << lepcollection[0].px() << ", ";
		std::cout << lepcollection[0].py() << ", ";
		std::cout << lepcollection[0].pz() << ", ";
		std::cout << lepcollection[0].energy() << ", ";
		std::cout << lepcollection[0].mass() << std::endl;
		std::cout << "dmeson: " << dmesoncollection[0].px() << ", ";
		std::cout << dmesoncollection[0].py() << ", ";
                std::cout << dmesoncollection[0].pz() << ", ";
                std::cout << dmesoncollection[0].energy() << ", ";
		std::cout << dmesoncollection[0].mass() << std::endl;
		std::cout << "neutrino: " << nuCand.px() << ", " << nuCand.py() << ", ";
                std::cout << nuCand.pz() << ", " << nuCand.energy() << ", ";
		std::cout << nuCand.mass() << std::endl;
	    }
	}
    }

    // at this point all variables have been calculated; now set TTree and TReader variables
    setVariables(varmap);

    // printouts for testing
    /*std::cout << "some variables: " << std::endl;
    std::cout << _leptonAsymmetry << std::endl;
    std::cout << _deepCSVMax;
    std::cout << _lT << std::endl;
    std::cout << _MT << std::endl;;
    std::cout << _HT << std::endl;
    std::cout << _nJets << std::endl;
    std::cout << _nBJets << std::endl; */

    // based on these variables: evaluate the BDT output if requested
    if(doMVA){
	varmap["_eventBDT"] = reader->EvaluateMVA( "BDT" );
	// also reset the TTree variable _eventBDT!
	_eventBDT = getVariable( varmap, "_eventBDT" );
    }
    
    // now return the varmap (e.g. to fill histograms)
    return varmap;
}


// internal helper functions

std::pair<PhysicsObject,PhysicsObject> eventFlattening::makeHiggsAndNeutrinoCandidate(
	const Lepton& lepton,
	const Met& met,
	double nupz,
	const DMeson& dmeson){
    
    // make the neutrino object
    double nuenergy = std::sqrt( met.pt()*met.pt() + nupz*nupz );
    LorentzVector nuvec = lorentzVectorPxPyPzEnergy( met.px(), met.py(), nupz, nuenergy );
    PhysicsObject nuCand = PhysicsObject( nuvec.pt(), nuvec.eta(), nuvec.phi(), nuvec.energy() );
    // make the higgs candidate
    PhysicsObject hCand = lepton + nuCand + dmeson;
    return std::make_pair( hCand, nuCand );
}


std::pair<PhysicsObject,PhysicsObject> eventFlattening::bestHiggsAndNeutrinoCandidate( 
	const Lepton& lepton, 
	const Met& met,
	std::pair<double,double> nupzcands,
	const DMeson& dmeson){
    
    // make first candidate
    std::pair<PhysicsObject,PhysicsObject> cand1;
    cand1 = makeHiggsAndNeutrinoCandidate(lepton, met, nupzcands.first, dmeson);
    // make second candidate
    std::pair<PhysicsObject,PhysicsObject> cand2;
    cand2 = makeHiggsAndNeutrinoCandidate(lepton, met, nupzcands.second, dmeson);
    // return the best one
    if(std::abs(cand1.first.mass()-particle::mH) < std::abs(cand2.first.mass()-particle::mH)){
	return cand1;
    }
    return cand2;
}
