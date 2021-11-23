/*
Custom analyzer class for investigating gen-level H->W+Ds decays.
*/
#include "heavyNeutrino/multilep/interface/HToWDGenAnalyzer.h"

// constructor //
HToWDGenAnalyzer::HToWDGenAnalyzer(const edm::ParameterSet& iConfig, multilep* multilepAnalyzer):
    multilepAnalyzer(multilepAnalyzer){
};

// destructor //
HToWDGenAnalyzer::~HToWDGenAnalyzer(){
}

// beginJob //
void HToWDGenAnalyzer::beginJob(TTree* outputTree){
    outputTree->Branch("_genhtowdEventType", &_genhtowdEventType, "_genhtowdEventType/I");
    // properties of Higgs boson
    outputTree->Branch("_genhtowdHPt", &_genhtowdHPt, "_genhtowdHPt/D");
    outputTree->Branch("_genhtowdHEta", &_genhtowdHEta, "_genhtowdHEta/D");
    outputTree->Branch("_genhtowdHPhi", &_genhtowdHPhi, "_genhtowdHPhi/D");
    outputTree->Branch("_genhtowdHE", &_genhtowdHE, "_genhtowdHE/D");
    // properties of W boson
    outputTree->Branch("_genhtowdWPt", &_genhtowdWPt, "_genhtowdWPt/D");
    outputTree->Branch("_genhtowdWEta", &_genhtowdWEta, "_genhtowdWEta/D");
    outputTree->Branch("_genhtowdWPhi", &_genhtowdWPhi, "_genhtowdWPhi/D");
    outputTree->Branch("_genhtowdWE", &_genhtowdWE, "_genhtowdWE/D");
    outputTree->Branch("_genhtowdWCharge", &_genhtowdWCharge, "_genhtowdWCharge/D");
    // properties of neutrino
    outputTree->Branch("_genhtowdNuPt", &_genhtowdNuPt, "_genhtowdNuPt/D");
    outputTree->Branch("_genhtowdNuEta", &_genhtowdNuEta, "_genhtowdNuEta/D");
    outputTree->Branch("_genhtowdNuPhi", &_genhtowdNuPhi, "_genhtowdNuPhi/D");
    outputTree->Branch("_genhtowdNuE", &_genhtowdNuE, "_genhtowdNuE/D");
    // properties of lepton
    outputTree->Branch("_genhtowdLPt", &_genhtowdLPt, "_genhtowdLPt/D");
    outputTree->Branch("_genhtowdLEta", &_genhtowdLEta, "_genhtowdLEta/D");
    outputTree->Branch("_genhtowdLPhi", &_genhtowdLPhi, "_genhtowdLPhi/D");
    outputTree->Branch("_genhtowdLE", &_genhtowdLE, "_genhtowdLE/D");
    outputTree->Branch("_genhtowdLCharge", &_genhtowdLCharge, "_genhtowdLCharge/D");
    outputTree->Branch("_genhtowdLFlavour", &_genhtowdLFlavour, "_genhtowdLFlavour/D");
    // properties of D meson
    outputTree->Branch("_genhtowdDPt", &_genhtowdDPt, "_genhtowdDPt/D");
    outputTree->Branch("_genhtowdDEta", &_genhtowdDEta, "_genhtowdDEta/D");
    outputTree->Branch("_genhtowdDPhi", &_genhtowdDPhi, "_genhtowdDPhi/D");
    outputTree->Branch("_genhtowdDE", &_genhtowdDE, "_genhtowdDE/D");
    outputTree->Branch("_genhtowdDCharge", &_genhtowdDCharge, "_genhtowdDCharge/D");
}

// analyze (main method) //
void HToWDGenAnalyzer::analyze(const edm::Event& iEvent){
    edm::Handle<std::vector<reco::GenParticle>> genParticles;
    genParticles = getHandle(iEvent, multilepAnalyzer->genParticleToken);
    if(!genParticles.isValid()){
	std::cout << "WARNING: genParticle collection not valid" << std::endl;
	return;
    }
    std::map< std::string, const reco::GenParticle* > HToWDGenParticles;
    HToWDGenParticles = find_H_to_WD( *genParticles );
    if( HToWDGenParticles["H"]==nullptr ){
	_genhtowdEventType = 0;
	_genhtowdHPt = 0;
	_genhtowdHEta = 0;
	_genhtowdHPhi = 0;
	_genhtowdHE = 0;
	_genhtowdWPt = 0;
        _genhtowdWEta = 0;
        _genhtowdWPhi = 0;
        _genhtowdWE = 0;
	_genhtowdWCharge = 0;
	_genhtowdNuPt = 0;
        _genhtowdNuEta = 0;
        _genhtowdNuPhi = 0;
        _genhtowdNuE = 0;
	_genhtowdLPt = 0;
        _genhtowdLEta = 0;
        _genhtowdLPhi = 0;
        _genhtowdLE = 0;
	_genhtowdLCharge = 0;
	_genhtowdLFlavour = 0;
	_genhtowdDPt = 0;
        _genhtowdDEta = 0;
        _genhtowdDPhi = 0;
        _genhtowdDE = 0;
	_genhtowdDCharge = 0;
    }
    else{
	_genhtowdEventType = 1;
	_genhtowdHPt = HToWDGenParticles["H"]->pt();
        _genhtowdHEta = HToWDGenParticles["H"]->eta();
        _genhtowdHPhi = HToWDGenParticles["H"]->phi();
        _genhtowdHE = HToWDGenParticles["H"]->energy();
        _genhtowdWPt = HToWDGenParticles["W"]->pt();
        _genhtowdWEta = HToWDGenParticles["W"]->eta();
        _genhtowdWPhi = HToWDGenParticles["W"]->phi();
        _genhtowdWE = HToWDGenParticles["W"]->energy();
	_genhtowdWCharge = HToWDGenParticles["W"]->charge();
        _genhtowdNuPt = HToWDGenParticles["Nu"]->pt();
        _genhtowdNuEta = HToWDGenParticles["Nu"]->eta();
        _genhtowdNuPhi = HToWDGenParticles["Nu"]->phi();
        _genhtowdNuE = HToWDGenParticles["Nu"]->energy();
        _genhtowdLPt = HToWDGenParticles["L"]->pt();
        _genhtowdLEta = HToWDGenParticles["L"]->eta();
        _genhtowdLPhi = HToWDGenParticles["L"]->phi();
        _genhtowdLE = HToWDGenParticles["L"]->energy();
	_genhtowdLCharge = HToWDGenParticles["L"]->charge();
	_genhtowdLFlavour = (std::abs(HToWDGenParticles["L"]->pdgId())==11) ? 0 :
			    (std::abs(HToWDGenParticles["L"]->pdgId())==13) ? 1 : 2;
        _genhtowdDPt = HToWDGenParticles["D"]->pt();
        _genhtowdDEta = HToWDGenParticles["D"]->eta();
        _genhtowdDPhi = HToWDGenParticles["D"]->phi();
        _genhtowdDE = HToWDGenParticles["D"]->energy();
	_genhtowdDCharge = HToWDGenParticles["D"]->charge();
    }
}

std::map< std::string, const reco::GenParticle* > HToWDGenAnalyzer::find_H_to_WD(
	const std::vector<reco::GenParticle>& genParticles){
    // initialize output
    std::map< std::string, const reco::GenParticle* > res = {
        {"H", nullptr},
        {"W", nullptr},
        {"Nu", nullptr},
        {"L", nullptr},
        {"D", nullptr},
    };
    // find the H
    int hindex = -1;
    for(unsigned i=0; i<genParticles.size(); ++i){
        int absId = std::abs( genParticles.at(i).pdgId() );
	int status = genParticles.at(i).status();
        if(absId == 25 && status==62) hindex = i;
    }
    // return if no suitable H found
    if( hindex<0 ){
        std::cout << "WARNING: no H found" << std::endl;
        return res;
    }
    const reco::GenParticle H = genParticles.at(hindex);
    // find its daughters
    std::vector<const reco::GenParticle*> hdaughters;
    for(unsigned int i=0; i<H.numberOfDaughters(); ++i){
        hdaughters.push_back( &genParticles[H.daughterRef(i).key()] );
    }
    // find the W boson and D meson
    if( hdaughters.size()!=2 ){
	std::cout << "WARNING: unexpected number of H daughters: ";
	for( auto gp: hdaughters ) std::cout << gp->pdgId() << " ";
	std::cout << std::endl;
	return res;
    }
    const reco::GenParticle* W;
    const reco::GenParticle* D;
    if( std::abs(hdaughters.at(0)->pdgId())==24
        && std::abs(hdaughters.at(1)->pdgId())==431 ){
        W = hdaughters.at(0);
        D = hdaughters.at(1);
    } else if( std::abs(hdaughters.at(0)->pdgId())==431
        && std::abs(hdaughters.at(1)->pdgId())==24 ){
        W = hdaughters.at(1);
        D = hdaughters.at(0);
    } else{
	std::cout << "WARNING: unexpected H daughters: ";
        for( auto gp: hdaughters ) std::cout << gp->pdgId() << " ";
        std::cout << std::endl;
	return res;
    }
    // find the daughters of the W
    std::vector<const reco::GenParticle*> wdaughters;
    for(unsigned int i=0; i<W->numberOfDaughters(); ++i){
        wdaughters.push_back( &genParticles[W->daughterRef(i).key()] );
    }
    // special case: foton emission from W boson
    bool iterate = false;
    for( unsigned int i=0; i<wdaughters.size(); ++i ){ 
	if( std::abs(wdaughters.at(i)->pdgId())==24 ){ 
	    iterate = true; 
	    W = wdaughters.at(i);
	    wdaughters.clear();
	    for(unsigned int i=0; i<W->numberOfDaughters(); ++i){
		wdaughters.push_back( &genParticles[W->daughterRef(i).key()] );
	    }
	}
    }
    while( iterate ){
	iterate = false;
	for( unsigned int i=0; i<wdaughters.size(); ++i ){
	    if( std::abs(wdaughters.at(i)->pdgId())==24 ){
		iterate = true; 
		W = wdaughters.at(i);
		wdaughters.clear();
		for(unsigned int i=0; i<W->numberOfDaughters(); ++i){
		    wdaughters.push_back( &genParticles[W->daughterRef(i).key()] );
		}
	    }
	}	
    }
    // find the lepton and neutrino
    if( wdaughters.size()!=2 ){
	std::cout << "WARNING: unexpected number of W daughters: ";
        for( auto gp: wdaughters ) std::cout << gp->pdgId() << " ";
        std::cout << std::endl;
	return res;
    }
    const reco::GenParticle* L;
    const reco::GenParticle* Nu;
    if( (std::abs(wdaughters.at(0)->pdgId())==11
	 && std::abs(wdaughters.at(1)->pdgId())==12 )
        || (std::abs(wdaughters.at(0)->pdgId())==13
	 && std::abs(wdaughters.at(1)->pdgId())==14 )
	|| (std::abs(wdaughters.at(0)->pdgId())==15
         && std::abs(wdaughters.at(1)->pdgId())==16) ){
        L = wdaughters.at(0);
        Nu = wdaughters.at(1);
    } else if( (std::abs(wdaughters.at(0)->pdgId())==12
         && std::abs(wdaughters.at(1)->pdgId())==11 )
        || (std::abs(wdaughters.at(0)->pdgId())==14 
         && std::abs(wdaughters.at(1)->pdgId())==13 )
	|| (std::abs(wdaughters.at(0)->pdgId())==16
         && std::abs(wdaughters.at(1)->pdgId())==15 ) ){
        L = wdaughters.at(1);
        Nu = wdaughters.at(0);
    } else{
	std::cout << "WARNING: unexpected W daughters: ";
        for( auto gp: wdaughters ) std::cout << gp->pdgId() << " ";
        std::cout << std::endl;
	return res;
    }

    // set the particles
    res["H"] = &H;
    res["W"] = W;
    res["D"] = D;
    res["L"] = L;
    res["Nu"] = Nu;
    return res;
}
