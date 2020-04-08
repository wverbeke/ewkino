// include header 
#include "../interface/eventSelections.h"

//include c++ library classes
#include <functional>

bool passES(Event& event, const std::string& eventselection, const bool isnpbackground){
    static std::map< std::string, std::function< bool(Event&, const bool) > > ESFunctionMap = {
        { "signalregion", pass_signalregion },
        { "wzcontrolregion", pass_wzcontrolregion },
        { "zzcontrolregion", pass_zzcontrolregion },
        { "zgcontrolregion", pass_zgcontrolregion },
        { "ttzcontrolregion", pass_ttzcontrolregion },
	{ "signalsideband_noossf", pass_signalsideband_noossf },
	{ "signalsideband_noz", pass_signalsideband_noz }
    };
    auto it = ESFunctionMap.find( eventselection );
    if( it == ESFunctionMap.cend() ){
        throw std::invalid_argument( "unknown event selection condition " + eventselection );
    } else {
        return (it->second)(event,isnpbackground);
    }
}

// help functions for signal and control region selections //

constexpr double halfwindow = 7.5;

void cleanleptoncollection(Event& event){
    event.selectLooseLeptons();
    event.cleanElectronsFromLooseMuons();
    event.cleanTausFromLooseLightLeptons();
    event.removeTaus();
}

void cleanjetcollection(Event& event){
    event.cleanJetsFromLooseLeptons();
    event.selectGoodJets(); // ttH jet selection
    event.selectGoodtZqJets(); // tZq jet selection
}

bool hasnFOLeptons(Event& event, int n){
    //event.selectFOLeptons(); // ttH lepton ID
    event.selectFOtZqLeptons(); // tZq lepton ID
    int nFOLeptons = event.numberOfLeptons();
    if(nFOLeptons == n) return true;
    return false;
}

bool hasnTightLeptons(Event& event, int n){
    //event.selectTightLeptons(); // ttH lepton ID
    event.selectTighttZqLeptons(); // tZq lepton ID
    int nTightLeptons = event.numberOfLeptons();
    if(nTightLeptons == n) return true;
    return false;
}

int eventCategory(Event& event){
    // determine the event category based on the number of jets and b-jets
    // note that it is assumed the event has been passed through a signal region selection!
    int njets = event.numberOfJets();
    int nbjets = event.numberOfMediumBTaggedJets();
    if(nbjets == 0 or (nbjets==1 and njets==1)) return -1;
    if(nbjets == 1 and (njets==2 or njets==3)) return 1;
    if(nbjets == 1) return 2;
    return 3;
}

// dedicated functions to check if event passes certain conditions //

bool pass_signalregion(Event& event, const bool isnpbackground){
    // clean jet collection (warning: need to check whether after or before lepton cleaning)
    cleanjetcollection(event);
    // clean lepton collections (see also ewkino/skimmer/src/skimSelections.cc)
    cleanleptoncollection(event);
    // select FO leptons
    if(!hasnFOLeptons(event,3)) return false;
    // select tight leptons
    if(isnpbackground && hasnTightLeptons(event,3)) return false;
    else if(!isnpbackground && !hasnTightLeptons(event,3)) return false;
    // Z boson candidate
    if(!event.hasOSSFLightLeptonPair()) return false;
    if(!event.hasZTollCandidate(halfwindow)) return false;
    // number of jets and b-jets
    if(eventCategory(event)==-1) return false;
    return true;
}

bool pass_signalsideband_noossf(Event& event, const bool isnpbackground){
    cleanjetcollection(event);
    cleanleptoncollection(event);
    if(!hasnFOLeptons(event,3)) return false;
    if(isnpbackground && hasnTightLeptons(event,3)) return false;
    else if(!isnpbackground && !hasnTightLeptons(event,3)) return false;
    // inverted cut on OSSF:
    if(event.hasOSSFLightLeptonPair()) return false;
    if(eventCategory(event)==-1) return false;
    return true;
}

bool pass_signalsideband_noz(Event& event, const bool isnpbackground){
    cleanjetcollection(event);
    cleanleptoncollection(event);
    if(!hasnFOLeptons(event,3)) return false;
    if(isnpbackground && hasnTightLeptons(event,3)) return false;
    else if(!isnpbackground && !hasnTightLeptons(event,3)) return false;
    if(!event.hasOSSFLightLeptonPair()) return false;
    // inverted cut on Z mass
    if(event.hasZTollCandidate(halfwindow)) return false;
    if(eventCategory(event)==-1) return false;
    return true;
}

bool pass_wzcontrolregion(Event& event, const bool isnpbackground){
    // very similar to signal region but b-jet veto and other specificities
    // cleaning and selecting leptons is done implicitly in pass_signalregion
    cleanjetcollection(event);
    cleanleptoncollection(event);
    if(!hasnFOLeptons(event,3)) return false;
    if(isnpbackground && hasnTightLeptons(event,3)) return false;
    else if(!isnpbackground && !hasnTightLeptons(event,3)) return false;
    if(!event.hasOSSFLightLeptonPair()) return false;
    if(!event.hasZTollCandidate(halfwindow)) return false;
    //for(JetCollection::const_iterator jIt = event.jetCollection().cbegin();
    //	jIt != event.jetCollection().cend(); jIt++){
    //	Jet& jet = **jIt;
    //}
    if(event.numberOfMediumBTaggedJets()>0) return false;
    if(event.metPt()<50.) return false;
    // calculate mass of 3-lepton system and veto mass close to Z mass
    if(fabs(event.leptonSystem().mass()-particle::mZ)<halfwindow) return false;
    return true;
}

bool pass_zzcontrolregion(Event& event, const bool isnpbackground){
    cleanjetcollection(event);
    cleanleptoncollection(event);
    if(!hasnFOLeptons(event,4)) return false;
    if(isnpbackground && hasnTightLeptons(event,4)) return false;
    else if(!isnpbackground && !hasnTightLeptons(event,4)) return false;
    if(!event.hasOSSFLeptonPair()) return false;
    if(!(event.numberOfUniqueOSSFLeptonPairs()==2)) return false;
    // first Z candidate
    std::pair< std::pair< int, int >, double > temp;
    temp = event.bestZBosonCandidateIndicesAndMass();
    if(fabs(temp.second-particle::mZ)>halfwindow) return false;
    // second Z candidate
    PhysicsObject lvec;
    for(LeptonCollection::const_iterator lIt = event.leptonCollection().cbegin();
        lIt != event.leptonCollection().cend(); lIt++){
        Lepton& lep = **lIt;
        if(lIt-event.leptonCollection().cbegin()==temp.first.first
                or lIt-event.leptonCollection().cbegin()==temp.first.second) continue;
        lvec += lep;
    }
    double llmass = lvec.mass();
    if(fabs(llmass-particle::mZ)>halfwindow) return false;
    return true;
}

bool pass_zgcontrolregion(Event& event, const bool isnpbackground){
    cleanjetcollection(event);
    cleanleptoncollection(event);
    if(!hasnFOLeptons(event,3)) return false;
    if(isnpbackground && hasnTightLeptons(event,3)) return false;
    else if(!isnpbackground && !hasnTightLeptons(event,3)) return false;
    if(!event.hasOSSFLightLeptonPair()) return false;
    if(fabs(event.leptonSystem().mass()-particle::mZ)>halfwindow) return false;
    bool pairZmass = false;
    for(LeptonCollection::const_iterator lIt1 = event.leptonCollection().cbegin();
        lIt1 != event.leptonCollection().cend(); lIt1++){
        Lepton& lep1 = **lIt1;
	for(LeptonCollection::const_iterator lIt2 = lIt1+1; lIt2!=event.leptonCollection().cend(); lIt2++){
	    Lepton& lep2 = **lIt2;
	    if(fabs((lep1+lep2).mass()-particle::mZ)<halfwindow) pairZmass = true;
	}
    }
    if(pairZmass) return false;
    return true;
}

bool pass_ttzcontrolregion(Event& event, const bool isnpbackground){
    // no dedicated ttz control region yet (see analysis note)
    // put dummy code here to avoid compilation warnings
    if(isnpbackground) cleanjetcollection(event);
    return false;
}
