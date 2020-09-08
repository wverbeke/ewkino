// include header 
#include "../interface/eventSelections.h"

//include c++ library classes
#include <functional>

bool passES(Event& event, const std::string& eventselection, 
		const std::string& selectiontype, const std::string& variation){
    std::vector< std::string > selectiontypes{ "3tight","3prompt","fakerate","2tight"};
    if( std::find(selectiontypes.cbegin(), selectiontypes.cend(), selectiontype)==selectiontypes.cend() ){
	throw std::invalid_argument("unknown selection type: "+selectiontype);
    }
    std::vector< std::string > variations{ "JECUp","JECDown","JERUp","JERDown","UnclUp","UnclDown",
					    "nominal","all"};
    if( std::find(variations.cbegin(), variations.cend(), variation)==variations.cend() ){
	throw std::invalid_argument("unknown variation: "+variation);
    }
    static std::map< std::string, std::function< bool(Event&, const std::string&, const std::string&) > > 
    ESFunctionMap = {
        { "signalregion", pass_signalregion },
        { "wzcontrolregion", pass_wzcontrolregion },
        { "zzcontrolregion", pass_zzcontrolregion },
        { "zgcontrolregion", pass_zgcontrolregion },
        { "ttzcontrolregion", pass_ttzcontrolregion },
	{ "signalsideband_noossf", pass_signalsideband_noossf },
	{ "signalsideband_noz", pass_signalsideband_noz },
	{ "npcontrolregion" , pass_npcontrolregion}
    };
    auto it = ESFunctionMap.find( eventselection );
    if( it == ESFunctionMap.cend() ){
        throw std::invalid_argument( "unknown event selection condition " + eventselection );
    }
    return (it->second)(event, selectiontype, variation);
}

// help functions for event cleaning //

constexpr double halfwindow = 7.5;

void cleanLeptonsAndJets(Event& event){
    // select leptons
    event.selectLooseLeptons();
    event.cleanElectronsFromLooseMuons();
    event.cleanTausFromLooseLightLeptons();
    event.removeTaus();
    // select jets
    event.cleanJetsFromFOLeptons();
    event.jetCollection().selectGoodAnyVariationJets();
    // sort leptons and apply cone correction
    event.sortLeptonsByPt();
    event.applyLeptonConeCorrection();
}

// help functions for getting correct jet collection and met //

JetCollection getjetcollection(const Event& event, const std::string& variation){
    if( variation == "nominal" ){
        return event.jetCollection().goodJetCollection();
    } else if( variation == "JECDown" ){
        return event.jetCollection().JECDownCollection().goodJetCollection();
    } else if( variation == "JECUp" ){
        return event.jetCollection().JECUpCollection().goodJetCollection();
    } else if( variation == "JERDown" ){
        return event.jetCollection().JERDownCollection().goodJetCollection();
    } else if( variation == "JERUp" ){
        return event.jetCollection().JERUpCollection().goodJetCollection();
    } else if( variation == "UnclDown" ){
        return event.jetCollection().goodJetCollection();
    } else if( variation == "UnclUp" ){
        return event.jetCollection().goodJetCollection();
    } else {
        throw std::invalid_argument( "Jet variation " + variation + " is unknown." );
    }
}

Met getmet(const Event& event, const std::string& variation){
    if( variation == "nominal" ){
        return event.met();
    } else if( variation == "JECDown" ){
        return event.met().MetJECDown();
    } else if( variation == "JECUp" ){
        return event.met().MetJECUp();
    } else if( variation == "JERDown" ){
        return event.met();
    } else if( variation == "JERUp" ){
	return event.met();
    } else if( variation == "UnclDown" ){
        return event.met().MetUnclusteredDown();
    } else if( variation == "UnclUp" ){
        return event.met().MetUnclusteredUp();
    } else {
        throw std::invalid_argument( "Met variation " + variation + " is unknown." );
    }
}

int eventCategory(Event& event, const std::string& variation){
    // determine the event category based on the number of jets and b-jets
    // note that it is assumed the event has been passed through a signal region selection!

    // if variation = "all", only apply most general selection: 
    // at least one b-jet and at least two jets in any variation
    if( variation=="all" ){
	if( event.jetCollection().numberOfGoodAnyVariationJets()<2 ) return -1;
	if( event.jetCollection().maxNumberOfMediumBTaggedJetsAnyVariation()<1 ) return -1;
	return 0;
    }
    // else, determine correct event category
    JetCollection jetc = getjetcollection(event,variation);
    int njets = jetc.size();
    int nbjets = jetc.numberOfMediumBTaggedJets();
    if(nbjets == 0 or (nbjets==1 and njets==1)) return -1;
    if(nbjets == 1 and (njets==2 or njets==3)) return 1;
    if(nbjets == 1) return 2;
    return 3;
}

// help functions for determining the number of leptons with correct ID //

bool hasnFOLeptons(Event& event, int n, bool select){
    int nFO = 0;
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( leptonPtr->isFO() ){ ++nFO; }
    }
    if( n!=nFO ){ return false; }
    if( select ){ event.selectFOLeptons(); }
    return true;
}

bool hasnTightLeptons(Event& event, int n, bool select){
    int nTight = 0;
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( leptonPtr->isTight() ){ ++nTight; }
    }
    if( n!=nTight ){ return false; }
    if( select ){ event.selectTightLeptons(); }
    return true;
}

bool allLeptonsArePrompt( const Event& event ){
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( !leptonPtr->isPrompt() ) return false;
    }
    return true;
}

// dedicated functions to check if event passes certain conditions //

bool pass_signalregion(Event& event, const std::string& selectiontype, 
			const std::string& variation){
    // apply basic object selection
    cleanLeptonsAndJets(event);
    // select FO leptons
    if(!hasnFOLeptons(event, 3, true)) return false;
    // do lepton selection for different types of selections
    if(selectiontype=="3tight"){ 
	if(!hasnTightLeptons(event, 3, true)) return false; 
    } else if(selectiontype=="3prompt"){
	if(!hasnTightLeptons(event, 3, true)) return false;
	if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="fakerate"){
	if(hasnTightLeptons(event, 3, false)) return false;
	if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="2tight"){
	if(!hasnTightLeptons(event, 2, false)) return false;
	if(hasnTightLeptons(event, 3, false)) return false;
    } else return false;
    // Z boson candidate
    if(!event.hasOSSFLightLeptonPair()) return false;
    if(!event.hasZTollCandidate(halfwindow)) return false;
    // number of jets and b-jets
    if(eventCategory(event, variation)<0) return false;
    return true;
}

bool pass_signalsideband_noossf(Event& event, const std::string& selectiontype, 
				const std::string& variation){
    cleanLeptonsAndJets(event);
    if(!hasnFOLeptons(event, 3, true)) return false;
    // do lepton selection for different types of selections
    if(selectiontype=="3tight"){
        if(!hasnTightLeptons(event, 3, true)) return false;
    } else if(selectiontype=="3prompt"){
        if(!hasnTightLeptons(event, 3, true)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="fakerate"){
        if(hasnTightLeptons(event, 3, false)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="2tight"){
        if(!hasnTightLeptons(event, 2, false)) return false;
        if(hasnTightLeptons(event, 3, false)) return false;
    } else return false;
    // inverted cut on OSSF:
    if(event.hasOSSFLightLeptonPair()) return false;
    // number of jets and b-jets
    if(eventCategory(event, variation)<0) return false;
    return true;
}

bool pass_signalsideband_noz(Event& event, const std::string& selectiontype,
				const std::string& variation){
    cleanLeptonsAndJets(event);
    if(!hasnFOLeptons(event,3,true)) return false;
    // do lepton selection for different types of selections
    if(selectiontype=="3tight"){
        if(!hasnTightLeptons(event, 3, true)) return false;
    } else if(selectiontype=="3prompt"){
        if(!hasnTightLeptons(event, 3, true)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="fakerate"){
        if(hasnTightLeptons(event, 3, false)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="2tight"){
        if(!hasnTightLeptons(event, 2, false)) return false;
        if(hasnTightLeptons(event, 3, false)) return false;
    } else return false;
    if(!event.hasOSSFLightLeptonPair()) return false;
    // inverted cut on Z mass
    if(event.hasZTollCandidate(halfwindow)) return false;
    // number of ets and b-jets
    if(eventCategory(event, variation)<0) return false;
    return true;
}

bool pass_wzcontrolregion(Event& event, const std::string& selectiontype,
				const std::string& variation){
    // very similar to signal region but b-jet veto and other specificities
    cleanLeptonsAndJets(event);
    if(!hasnFOLeptons(event,3,true)) return false;
    // do lepton selection for different types of selections
    if(selectiontype=="3tight"){
        if(!hasnTightLeptons(event, 3, true)) return false;
    } else if(selectiontype=="3prompt"){
        if(!hasnTightLeptons(event, 3, true)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="fakerate"){
        if(hasnTightLeptons(event, 3, false)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="2tight"){
        if(!hasnTightLeptons(event, 2, false)) return false;
        if(hasnTightLeptons(event, 3, false)) return false;
    } else return false;
    if(!event.hasOSSFLightLeptonPair()) return false;
    if(!event.hasZTollCandidate(halfwindow)) return false;
    if( variation=="all" ){
	if(event.jetCollection().minNumberOfMediumBTaggedJetsAnyVariation()>0) return false;
	if(event.met().maxPtAnyVariation()<50) return false;
    } else{
	if(getjetcollection(event,variation).numberOfMediumBTaggedJets()>0) return false;
	if(getmet(event,variation).pt()<50.) return false;
    }
    // calculate mass of 3-lepton system and veto mass close to Z mass
    if(fabs(event.leptonSystem().mass()-particle::mZ)<halfwindow) return false;
    return true;
}

bool pass_zzcontrolregion(Event& event, const std::string& selectiontype,
				const std::string& variation){
    cleanLeptonsAndJets(event);
    if(!hasnFOLeptons(event,4,true)) return false;
    // do lepton selection for different types of selections
    if(selectiontype=="3tight"){
        if(!hasnTightLeptons(event, 4, true)) return false;
    } else if(selectiontype=="3prompt"){
        if(!hasnTightLeptons(event, 4, true)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="fakerate"){
        if(hasnTightLeptons(event, 4, false)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="2tight"){
        if(!hasnTightLeptons(event, 3, false)) return false;
        if(hasnTightLeptons(event, 4, false)) return false;
    } else return false;
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
    // dummy condition on variation to avoid warnings
    if(variation=="all") return true;
    return true;
}

bool pass_zgcontrolregion(Event& event, const std::string& selectiontype,
			    const std::string& variation){
    cleanLeptonsAndJets(event);
    if(!hasnFOLeptons(event,3,true)) return false;
    // do lepton selection for different types of selections
    if(selectiontype=="3tight"){
        if(!hasnTightLeptons(event, 3, true)) return false;
    } else if(selectiontype=="3prompt"){
        if(!hasnTightLeptons(event, 3, true)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="fakerate"){
        if(hasnTightLeptons(event, 3, false)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="2tight"){
        if(!hasnTightLeptons(event, 2, false)) return false;
        if(hasnTightLeptons(event, 3, false)) return false;
    } else return false;
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
    // dummy condition on variation to avoid warnings
    if(variation=="all") return true;
    return true;
}

bool pass_ttzcontrolregion(Event& event, const std::string& selectiontype, 
			    const std::string& variation){
    // no dedicated ttz control region yet (see analysis note)
    // put dummy code here to avoid compilation warnings
    if(selectiontype=="dummy") cleanLeptonsAndJets(event);
    // dummy condition on variation to avoid warnings
    if(variation=="all") return false;
    return false;
}

bool pass_npcontrolregion(Event& event, const std::string& selectiontype,
                                const std::string& variation){
    cleanLeptonsAndJets(event);
    if(!hasnFOLeptons(event, 3, true)) return false;
    // do lepton selection for different types of selections
    if(selectiontype=="3tight"){
        if(!hasnTightLeptons(event, 3, true)) return false;
    } else if(selectiontype=="3prompt"){
        if(!hasnTightLeptons(event, 3, true)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="fakerate"){
        if(hasnTightLeptons(event, 3, false)) return false;
        if(event.isMC() and !allLeptonsArePrompt(event)) return false;
    } else if(selectiontype=="2tight"){
        if(!hasnTightLeptons(event, 2, false)) return false;
        if(hasnTightLeptons(event, 3, false)) return false;
    } else return false;
    // either no OSSF or OSSF off Z-mass:
    if(event.hasOSSFLightLeptonPair() and event.hasZTollCandidate(halfwindow)) return false;
    // at least two jets of which at least one b-jet (? mabye leave out?)
    //if(eventCategory(event, variation)<0) return false;
    // dummy condition on variation to avoid warnings
    if(variation=="all") return true;
    return true;
}
