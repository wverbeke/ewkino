// include header 
#include "../interface/eventSelections.h"

//include c++ library classes
#include <functional>

bool passES(Event& event, const std::string& eventselection, const std::string& leptonID,
	    const bool isdataforbackground){
    // allowed values of eventselection: see list below
    // allowed values of leptoniD: "tth" and "tzq"
    static std::map< std::string, std::function<    bool(Event&, const std::string, const bool) > > 
    ESFunctionMap = {
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
        return (it->second)(event, leptonID, isdataforbackground);
    }
}

// help functions for event cleaning //

constexpr double halfwindow = 7.5;

void cleanleptoncollection(Event& event){
    event.selectLooseLeptons();
    event.cleanElectronsFromLooseMuons();
    event.cleanTausFromLooseLightLeptons();
    event.removeTaus();
}

void cleanjetcollection(Event& event){
    event.cleanJetsFromLooseLeptons(); // or from FO leptons? -> check!
    //event.jetCollection().selectGoodAnyVariationJets(); // ttH jet selection
    event.jetCollection().selectGoodAnyVariationtZqJets(); // tZq jet selection
}

// help functions for determining the number of leptons with correct ID //

bool hasnFOLeptons(Event& event, int n, const std::string& leptonID, bool select){
    int nFO = 0;
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( leptonID=="tth" && leptonPtr->isFO() ){ ++nFO; }
        else if( leptonID=="tzq" && leptonPtr->isFOtZq() ){ ++nFO; }
    }
    if( n!=nFO ){ return false; }
    if( select && leptonID=="tth" ){ event.selectFOLeptons(); }
    else if( select && leptonID=="tzq" ){ event.selectFOtZqLeptons(); }
    return true;
}

bool hasnTightLeptons(Event& event, int n, const std::string& leptonID, bool select){
    int nTight = 0;
    for( const auto& leptonPtr : event.lightLeptonCollection() ){
        if( leptonID=="tth" && leptonPtr->isTight() ){ ++nTight; }
        else if( leptonID=="tzq" && leptonPtr->isTighttZq() ){ ++nTight; }
    }
    if( n!=nTight ){ return false; }
    if( select && leptonID=="tth" ){ event.selectTightLeptons(); }
    else if( select && leptonID=="tzq" ){ event.selectTighttZqLeptons(); }
    return true;
}

// dedicated functions to check if event passes certain conditions //

bool pass_signalregion(Event& event, const std::string& leptonID, 
			const bool isdataforbackground){
    // clean jet collection (warning: need to check whether after or before lepton cleaning)
    cleanjetcollection(event);
    // clean lepton collections (see also ewkino/skimmer/src/skimSelections.cc)
    cleanleptoncollection(event);
    // select FO leptons
    if(!hasnFOLeptons(event,3,leptonID)) return false;
    // select tight leptons
    if(isdataforbackground && hasnTightLeptons(event,3,leptonID,false)) return false;
    else if(!isdataforbackground && !hasnTightLeptons(event,3,leptonID)) return false;
    // Z boson candidate
    if(!event.hasOSSFLightLeptonPair()) return false;
    if(!event.hasZTollCandidate(halfwindow)) return false;
    // number of jets and b-jets -> move to event flattening
    return true;
}

bool pass_signalsideband_noossf(Event& event, const std::string& leptonID, 
			const bool isdataforbackground){
    cleanjetcollection(event);
    cleanleptoncollection(event);
    if(!hasnFOLeptons(event,3,leptonID)) return false;
    if(isdataforbackground && hasnTightLeptons(event,3,leptonID,false)) return false;
    else if(!isdataforbackground && !hasnTightLeptons(event,3,leptonID)) return false;
    // inverted cut on OSSF:
    if(event.hasOSSFLightLeptonPair()) return false;
    // number of jets and b-jets -> move to event flattening
    return true;
}

bool pass_signalsideband_noz(Event& event, const std::string& leptonID, 
			const bool isdataforbackground){
    cleanjetcollection(event);
    cleanleptoncollection(event);
    if(!hasnFOLeptons(event,3,leptonID)) return false;
    if(isdataforbackground && hasnTightLeptons(event,3,leptonID,false)) return false;
    else if(!isdataforbackground && !hasnTightLeptons(event,3,leptonID)) return false;
    if(!event.hasOSSFLightLeptonPair()) return false;
    // inverted cut on Z mass
    if(event.hasZTollCandidate(halfwindow)) return false;
    // number of ets and b-jets -> move to event flattening
    return true;
}

bool pass_wzcontrolregion(Event& event, const std::string& leptonID, 
			const bool isdataforbackground){
    // very similar to signal region but b-jet veto and other specificities
    // cleaning and selecting leptons is done implicitly in pass_signalregion
    cleanjetcollection(event);
    cleanleptoncollection(event);
    if(!hasnFOLeptons(event,3,leptonID)) return false;
    if(isdataforbackground && hasnTightLeptons(event,3,leptonID,false)) return false;
    else if(!isdataforbackground && !hasnTightLeptons(event,3,leptonID)) return false;
    if(!event.hasOSSFLightLeptonPair()) return false;
    if(!event.hasZTollCandidate(halfwindow)) return false;
    if(event.numberOfMediumBTaggedJets()>0) return false;
    if(event.metPt()<50.) return false;
    // calculate mass of 3-lepton system and veto mass close to Z mass
    if(fabs(event.leptonSystem().mass()-particle::mZ)<halfwindow) return false;
    return true;
}

bool pass_zzcontrolregion(Event& event, const std::string& leptonID, 
			const bool isdataforbackground){
    cleanjetcollection(event);
    cleanleptoncollection(event);
    if(!hasnFOLeptons(event,4,leptonID)) return false;
    if(isdataforbackground && hasnTightLeptons(event,4,leptonID,false)) return false;
    else if(!isdataforbackground && !hasnTightLeptons(event,4,leptonID)) return false;
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

bool pass_zgcontrolregion(Event& event, const std::string& leptonID, 
			const bool isdataforbackground){
    cleanjetcollection(event);
    cleanleptoncollection(event);
    if(!hasnFOLeptons(event,3,leptonID)) return false;
    if(isdataforbackground && hasnTightLeptons(event,3,leptonID,false)) return false;
    else if(!isdataforbackground && !hasnTightLeptons(event,3,leptonID)) return false;
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

bool pass_ttzcontrolregion(Event& event, const std::string& leptonID, 
			const bool isdataforbackground){
    // no dedicated ttz control region yet (see analysis note)
    // put dummy code here to avoid compilation warnings
    if(leptonID=="tth" && isdataforbackground) cleanjetcollection(event);
    return false;
}
