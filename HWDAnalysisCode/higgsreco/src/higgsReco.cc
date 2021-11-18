/*
Definition of Higgs reconstruction techniques
*/

// include header
#include "../interface/higgsReco.h"


// internal helper functions

std::pair<PhysicsObject,PhysicsObject> higgsReco::makeHiggsAndNeutrinoCandidate(
        const Lepton& lepton,
        const Met& met,
        double nupz,
        const DMeson& dmeson){
    // make a higgs candidate and neutrino candidate as a pair of PhysicsObjects,
    // with given lepton, met, neutrino z-momentum and a D meson.
    // make the neutrino object
    double nuenergy = std::sqrt( met.pt()*met.pt() + nupz*nupz );
    LorentzVector nuvec = lorentzVectorPxPyPzEnergy( met.px(), met.py(), nupz, nuenergy );
    PhysicsObject nuCand = PhysicsObject( nuvec.pt(), nuvec.eta(), nuvec.phi(), nuvec.energy() );
    // make the higgs candidate
    PhysicsObject hCand = lepton + nuCand + dmeson;
    return std::make_pair( hCand, nuCand );
}

std::vector<std::pair<PhysicsObject,PhysicsObject>> higgsReco::makeHiggsAndNeutrinoCandidates(
        const Lepton& lepton,
        const Met& met,
        std::pair<double,double> nupzcands,
        const DMeson& dmeson){
    // same as makeHiggsAndNeutrinoCandidate, but return both possible solutions
    // for the case when two neutrino z-momentum candidates are given
    std::vector<std::pair<PhysicsObject,PhysicsObject>> res;
    // make first candidate
    std::pair<PhysicsObject,PhysicsObject> cand1;
    cand1 = makeHiggsAndNeutrinoCandidate(lepton, met, nupzcands.first, dmeson);
    // make second candidate
    std::pair<PhysicsObject,PhysicsObject> cand2;
    cand2 = makeHiggsAndNeutrinoCandidate(lepton, met, nupzcands.second, dmeson);
    // return both
    res.push_back(cand1);
    res.push_back(cand2);
    return res;
}


// higgs reconstruction functions

std::vector<std::string> higgsReco::genericHiggsRecoMethods(){
    std::vector<std::string> methods = {    "maxnupz", "minnupz",
					    "maxhpz", "minhpz",
					    "bestmass" };
    return methods;
}

std::pair<PhysicsObject,PhysicsObject> higgsReco::genericHiggsReco( const Event& event,
								    const std::string& method ){
    // reconstruct the higgs boson from a lepton, met and Ds meson.
    // all submethods are based on a simple solving of a quadratic equation to find nu_p_z.
    // different submethods can be chosen based on which nu_p_z candidate is chosen.
    // note: make sure to keep in sync with the functions in nureco.
    //       for now, there is only one method implemented there,
    //       but if that is extended, probably some names should change here as well.
    // note: the event must be cleaned and processed by an event selection function first!
    //	     specifically, the event is assumed to contain exactly one good lepton 
    //       and one good Ds meson.

    // initializations
    bool hasvalidds = (event.dmesonCollection().size()==1);
    bool hasvalidlepton = (event.leptonCollection().size()==1);
    if(!hasvalidds or !hasvalidlepton){
	std::string msg = "ERROR in higgsReco::genericHiggsReco:";
	msg += " event is not valid, which is currently not supported";
	throw std::runtime_error(msg);
    }
    Met met = event.getMet("nominal");
    LeptonCollection lepcollection = event.leptonCollection();
    DMesonCollection dmesoncollection = event.dmesonCollection();

    // do neutrino reconstruction
    std::tuple<bool, std::pair<double,double>> nusol;
    nusol = nuReco::pNuZCandidates( lepcollection[0], 0, met, particle::mW );
    std::pair<double,double> nupzcands = std::get<1>( nusol );

    // get both options for the higgs boson
    std::vector<std::pair<PhysicsObject,PhysicsObject>> hoptions = makeHiggsAndNeutrinoCandidates(
	lepcollection[0], met, nupzcands, dmesoncollection[0] );

    // printouts for testing
    /*std::cout << "option 1:" << std::endl;
    std::cout << "higgs candidate: " << hoptions[0].first << std::endl;
    std::cout << "neutrino candidate: " << hoptions[0].second << std::endl;
    std::cout << "option 2:" << std::endl;
    std::cout << "higgs candidate: " << hoptions[1].first << std::endl;
    std::cout << "neutrino candidate: " << hoptions[1].second << std::endl;*/

    // choose one of both options based on different criteria
    if( method=="maxnupz" ){
	// choose neutrino solution with largest z-momentum
	if( std::abs(nupzcands.first) > std::abs(nupzcands.second) ) return hoptions[0];
	else return hoptions[1];
    } else if( method=="minnupz" ){
	// choose neutrino solution with smallest z-momentum
	if( std::abs(nupzcands.first) > std::abs(nupzcands.second) ) return hoptions[1];
	else return hoptions[0];
    } else if( method=="maxhpz" ){
	// choose higgs boson solution with largest z-momentum
	double firsthpz = hoptions[0].first.pt();
	double secondhpz = hoptions[1].first.pt();
	if( std::abs(firsthpz) > std::abs(secondhpz) ) return hoptions[0];
	else return hoptions[1];
    } else if( method=="minhpz"){
	// choos higgs boson solution with smallest z-momentum
	double firsthpz = hoptions[0].first.pt();
        double secondhpz = hoptions[1].first.pt();
        if( std::abs(firsthpz) > std::abs(secondhpz) ) return hoptions[1];
	else return hoptions[0];
    } else if( method=="bestmass" ){
	// choose candidate with best nominal higgs boson mass
	double firsthmass = hoptions[0].first.mass();
	double secondhmass = hoptions[1].first.mass();
	if(std::abs(firsthmass-particle::mH) < std::abs(secondhmass-particle::mH)){
	    return hoptions[0]; }
	else return hoptions[1];
    } 
    std::string msg = "ERROR in higgsReco::genericHiggsReco:";
    msg += " method '"+method+"' not recognized.";
    throw std::runtime_error(msg);
}
