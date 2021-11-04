/*
Custom analyzer class for investigating gen-level D meson decays.
*/
#include "heavyNeutrino/multilep/interface/DGenAnalyzer.h"

// constructor //
DGenAnalyzer::DGenAnalyzer(const edm::ParameterSet& iConfig, multilep* multilepAnalyzer):
    multilepAnalyzer(multilepAnalyzer){
};

// destructor //
DGenAnalyzer::~DGenAnalyzer(){
}

// beginJob //
void DGenAnalyzer::beginJob(TTree* outputTree){
    outputTree->Branch("_genDEventType", &_genDEventType, "_genDEventType/I");
}

// analyze (main method) //
void DGenAnalyzer::analyze(const edm::Event& iEvent){
    edm::Handle<std::vector<reco::GenParticle>> genParticles;
    genParticles = getHandle(iEvent, multilepAnalyzer->genParticleToken);
    if(!genParticles.isValid()){
	std::cout << "WARNING: genParticle collection not valid" << std::endl;
	return;
    }
    //find_DStar_To_D0Pi_To_KPiPi( *genParticles );
    std::map< std::string, const reco::GenParticle* > DGenParticles;
    DGenParticles = find_Ds_To_PhiPi_To_KKPi( *genParticles );
    // set the event type (maybe to be extended later)
    if( DGenParticles["Ds"]==nullptr ) _genDEventType = 0;
    else _genDEventType = 1;
}

void DGenAnalyzer::find_DStar_To_D0Pi_To_KPiPi(
	const std::vector<reco::GenParticle>& genParticles){
    // find the DStar
    int dstarindex = -1;
    for(unsigned i=0; i<genParticles.size(); ++i){
        int absId = std::abs( genParticles.at(i).pdgId() );
	if(absId == 413) dstarindex = i;
    }
    if( dstarindex<0 ){
	std::cout << "WARNING: no DStar found" << std::endl;
	return;
    }
    const reco::GenParticle dstar = genParticles.at(dstarindex);
    // find its daughters
    std::vector<const reco::GenParticle*> dstardaughters;
    for(unsigned int i=0; i<dstar.numberOfDaughters(); ++i){
        dstardaughters.push_back( &genParticles[dstar.daughterRef(i).key()] );
    }
    // find the pion and D0
    if( dstardaughters.size()!=2 ) return;
    const reco::GenParticle* pi;
    const reco::GenParticle* d0;
    if( std::abs(dstardaughters.at(0)->pdgId())==421 
	&& std::abs(dstardaughters.at(1)->pdgId())==211 ){
	d0 = dstardaughters.at(0);
	pi = dstardaughters.at(1);
    } else if( std::abs(dstardaughters.at(0)->pdgId())==211 
	&& std::abs(dstardaughters.at(1)->pdgId())==421 ){
	d0 = dstardaughters.at(1);
        pi = dstardaughters.at(0);
    } else return;
    // find the daughters of the D0
    std::vector<const reco::GenParticle*> d0daughters;
    for(unsigned int i=0; i<d0->numberOfDaughters(); ++i){
        d0daughters.push_back( &genParticles[d0->daughterRef(i).key()] );
    }
    //for( const reco::GenParticle* p: d0daughters ){ std::cout << p->pdgId() << " "; } 
    //std::cout << std::endl;
    // find the K and pi
    const reco::GenParticle* K;
    const reco::GenParticle* pi2;
    if( d0daughters.size()!=2 ) return;
    if( std::abs(d0daughters.at(0)->pdgId())==321
        && std::abs(d0daughters.at(1)->pdgId())==211 ){
	std::cout << "found decay to pi K pi!" << std::endl;
	K = d0daughters.at(0);
	pi2 = d0daughters.at(1);
    } else if( std::abs(dstardaughters.at(0)->pdgId())==211
        && std::abs(dstardaughters.at(1)->pdgId())==321 ){
	std::cout << "found decay to pi pi K!" << std::endl;
	K = d0daughters.at(0);    
        pi2 = d0daughters.at(1);
    } else return;

    // print kinematics
    std::cout << "DStar kinematics:" << std::endl;
    std::cout << dstar.pt() << " " << dstar.eta() << " " << dstar.phi() << std::endl;
    std::cout << "first pion kinematics:" << std::endl;
    std::cout << pi->pt() << " " << pi->eta() << " " << pi->phi() << std::endl;
    std::cout << "D0 kinematics:" << std::endl;
    std::cout << d0->pt() << " " << d0->eta() << " " << d0->phi() << std::endl;
    std::cout << "kaon kinematics:" << std::endl;
    std::cout << K->pt() << " " << K->eta() << " " << K->phi() << std::endl;
    std::cout << "second pion kinematics:" << std::endl;
    std::cout << pi2->pt() << " " << pi2->eta() << " " << pi2->phi() << std::endl;

    return;
}

std::map< std::string, const reco::GenParticle* > DGenAnalyzer::find_Ds_To_PhiPi_To_KKPi(
	const std::vector<reco::GenParticle>& genParticles){
    // initialize output
    std::map< std::string, const reco::GenParticle* > res = {
        {"Ds", nullptr},
        {"Phi", nullptr},
        {"Pi", nullptr},
        {"K1", nullptr},
        {"K2", nullptr},
    };
    // find the Ds
    int dsindex = -1;
    for(unsigned i=0; i<genParticles.size(); ++i){
        int absId = std::abs( genParticles.at(i).pdgId() );
        if(absId == 431) dsindex = i;
    }
    if( dsindex<0 ){
        std::cout << "WARNING: no Ds found" << std::endl;
        return res;
    }
    const reco::GenParticle ds = genParticles.at(dsindex);
    // find its daughters
    std::vector<const reco::GenParticle*> dsdaughters;
    for(unsigned int i=0; i<ds.numberOfDaughters(); ++i){
        dsdaughters.push_back( &genParticles[ds.daughterRef(i).key()] );
    }
    // printouts
    /*std::cout << "ds daughters:" << std::endl;
    for( const reco::GenParticle* p: dsdaughters ){ std::cout << p->pdgId() << " "; }
    std::cout << std::endl;*/
    // find the pion and phi
    if( dsdaughters.size()!=2 ) return res;
    const reco::GenParticle* pi;
    const reco::GenParticle* phi;
    if( std::abs(dsdaughters.at(0)->pdgId())==333
        && std::abs(dsdaughters.at(1)->pdgId())==211 ){
        phi = dsdaughters.at(0);
        pi = dsdaughters.at(1);
    } else if( std::abs(dsdaughters.at(0)->pdgId())==211
        && std::abs(dsdaughters.at(1)->pdgId())==333 ){
        phi = dsdaughters.at(1);
        pi = dsdaughters.at(0);
    } else return res;
    // printouts
    //std::cout << "  -> found Ds -> phi + pi" << std::endl;
    // find the daughters of the phi
    std::vector<const reco::GenParticle*> phidaughters;
    for(unsigned int i=0; i<phi->numberOfDaughters(); ++i){
        phidaughters.push_back( &genParticles[phi->daughterRef(i).key()] );
    }
    // printouts
    /*std::cout << "phi daughters" << std::endl;
    for( const reco::GenParticle* p: phidaughters ){ std::cout << p->pdgId() << " "; } 
    std::cout << std::endl;*/
    // find the kaons
    const reco::GenParticle* K1;
    const reco::GenParticle* K2;
    if( phidaughters.size()!=2 ) return res;
    if( std::abs(phidaughters.at(0)->pdgId())==321
        && std::abs(phidaughters.at(1)->pdgId())==321 ){
        //std::cout << "found decay to K K pi!" << std::endl;
        K1 = phidaughters.at(0);
        K2 = phidaughters.at(1);
    } else return res;

    // print kinematics
    /*std::cout << "Ds kinematics:" << std::endl;
    std::cout << ds.pt() << " " << ds.eta() << " " << ds.phi() << std::endl;
    std::cout << "pion kinematics:" << std::endl;
    std::cout << pi->pt() << " " << pi->eta() << " " << pi->phi() << std::endl;
    std::cout << "phi kinematics:" << std::endl;
    std::cout << phi->pt() << " " << phi->eta() << " " << phi->phi() << std::endl;
    std::cout << "kaon1 kinematics:" << std::endl;
    std::cout << K1->pt() << " " << K1->eta() << " " << K1->phi() << std::endl;
    std::cout << "kaon2 kinematics:" << std::endl;
    std::cout << K2->pt() << " " << K2->eta() << " " << K2->phi() << std::endl;*/

    // set the particles
    res["Ds"] = &ds;
    res["Phi"] = phi;
    res["Pi"] = pi;
    res["K1"] = K1;
    res["K2"] = K2;
    return res;
}

