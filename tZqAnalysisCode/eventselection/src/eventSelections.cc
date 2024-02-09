// include header 
#include "../interface/eventSelections.h"

//include c++ library classes
#include <functional>

bool passES(Event& event, const std::string& eventselection, 
		const std::string& selectiontype, const std::string& variation,
		const bool selectbjets ){
    // arguments:
    // - object of type Event
    // - event selection identifier, see map below for allowd values
    // - selection type identifier, i.e. 3tight, 3prompt, fakerate or 2tight
    // - variation identifier, i.e. all, nominal, or any JEC/JER/Uncl variation
    // - boolean whether to select b-jets (set to false for b-tag shape normalization)

    // check if selectiontype is valid
    std::vector< std::string > seltypes{ "3tight","3prompt","fakerate","2tight"};
    if( std::find(seltypes.cbegin(), seltypes.cend(), selectiontype)==seltypes.cend() ){
	throw std::invalid_argument("unknown selection type: "+selectiontype);
    }
    // check if variation is valid
    // WARNING: can no longer do this as many split JEC sources are allowed
    /*std::vector< std::string > variations{ "JECUp","JECDown","JERUp","JERDown",
    //					    "UnclUp","UnclDown",
    //					    "nominal","all"};
    //if( std::find(variations.cbegin(), variations.cend(), variation)==variations.cend() ){
    //	throw std::invalid_argument("unknown variation: "+variation);
    }*/
    // map event selection to function
    static std::map< std::string, std::function< 
	bool(Event&, const std::string&, const std::string&, const bool) > > 
	    ESFunctionMap = {
		{ "signalregion", pass_signalregion },
		{ "wzcontrolregion", pass_wzcontrolregion },
		{ "zzcontrolregion", pass_zzcontrolregion },
		{ "zgcontrolregion", pass_zgcontrolregion },
		{ "ttzcontrolregion", pass_ttzcontrolregion },
		{ "signalsideband_noossf", pass_signalsideband_noossf },
		{ "signalsideband_noz", pass_signalsideband_noz },
		{ "signalsideband", pass_signalsideband }, // simple logical OR of noossf and noz
	    };
    auto it = ESFunctionMap.find( eventselection );
    if( it == ESFunctionMap.cend() ){
        throw std::invalid_argument( "unknown event selection condition " + eventselection );
    }
    return (it->second)(event, selectiontype, variation, selectbjets);
}

// help functions for event cleaning //

constexpr double halfwindow = 15;

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

int eventCategory(Event& event, const std::string& variation, const bool selectbjets){
    // determine the event category based on the number of jets and b-jets
    // note that it is assumed the event has been passed through a signal region selection!
    // return values:
    // - if varation is "all", then 0 if at least one b-jet and at least 2 jets, -1 otherwise
    // - if variation is specific, then 1/2/3 for specifiec categories, -1 otherwise
    // note: if selectbjets is false, then 0 is returned if at least 2 jets, -1 otherwise

    // if variation = "all", only apply most general selection: 
    // at least one b-jet and at least two jets in any variation
    if( variation=="all" ){
	if( event.jetCollection().numberOfGoodAnyVariationJets()<2 ) return -1;
	if( selectbjets 
	    && event.jetCollection().maxNumberOfMediumBTaggedJetsAnyVariation()<1 ) return -1;
	return 0;
    }
    // else, determine number of jets and b-jets in correct variation
    JetCollection jetc = event.getJetCollection(variation);
    int njets = jetc.size();
    int nbjets = jetc.numberOfMediumBTaggedJets();
    // if no b-jet selection required, simply require njets>=2
    if( !selectbjets ){
	if( njets<2 ) return -1;
	return 0;
    } 
    // else, determine full event category
    // no bjets or (only 1 bjet and no additional jet): -1
    if(nbjets == 0 or (nbjets==1 and njets==1)) return -1;
    // one bjet and 2-3 jets: 1
    if(nbjets == 1 and (njets==2 or njets==3)) return 1;
    // one bjet and >=4 jets: 2
    if(nbjets == 1) return 2;
    // >=2 bjets: 3
    return 3;
}

// help functions for determining if event belongs to a sub-category //

float lWCharge( Event& event ){ // (event cannot be const here)
    int lWindex = 0;
    if(event.hasOSSFLightLeptonPair()){ lWindex = event.WLeptonIndex(); }
    LeptonCollection::const_iterator lIt = event.leptonCollection().cbegin();
    for(int i=0; i<lWindex; i++){++lIt;}
    Lepton& lW = **lIt;
    return lW.charge();
}

bool hasLeptonFromMEExternalConversion( const Event& event ){
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( leptonPtr->isFO() && leptonFromMEExternalConversion( *leptonPtr ) ){
            return true;
        }
    }
    return false;
}

// help functions for overlap removal between inclusive and dedicated photon samples //

bool leptonFromMEExternalConversion( const Lepton& lepton ){
    // from Willem's ewkinoAnalysis code
    if( !( lepton.matchPdgId() == 22 ) ) return false;
    if( !( lepton.isPrompt() && lepton.provenanceConversion() == 0 ) ) return false;
    return true;
}

bool passPhotonOverlapRemoval( const Event& event ){
    bool isPhotonSample = false;
    bool isInclusiveSample = false;
    std::string sampleName = event.sample().fileName();
    if( stringTools::stringContains( sampleName, "DYJetsToLL" ) 
	|| stringTools::stringContains( sampleName, "TTTo" ) 
	|| stringTools::stringContains( sampleName, "TTJets" ) ){
        isInclusiveSample = true;
    } else if( stringTools::stringContains( sampleName, "TTGamma" ) 
	|| stringTools::stringContains( sampleName, "ZGToLLG" ) 
	|| stringTools::stringContains( sampleName, "ZGTo2LG" )
	|| stringTools::stringContains( sampleName, "WGToLNuG" ) ){
        isPhotonSample = true;
    }

    if( !( isPhotonSample || isInclusiveSample ) ){
        return true;
    }

    bool usePhotonSample = false;
    // method 1: check for prompt leptons matched to photons without provenanceConversion
    //if( hasLeptonFromMEExternalConversion( event ) ) usePhotonSample = true;
    // method 2: simply check if all leptons are prompt (note: need to select FO leptons first!)
    if( allLeptonsArePrompt(event) ){
	// if all leptons are prompt -> use ZG sample
	usePhotonSample = true;
	// BUT potential problem in ZG samples with missing phase space (gen level photon cut)
	// solved for 2016 by using LoosePtlPtg sample (? still checking)
	// for 2017/2018, these samples have other problems, instead use DY for low pt part
	//if(event.is2016()) usePhotonSample = true;
	//else{}
    }

    if( isInclusiveSample ){
        return !usePhotonSample;
    } else if( isPhotonSample ){
        return usePhotonSample;
    }
    return true;
}

// help functions for trigger and pt-threshold selections //

bool passAnyTrigger(Event& event){
    bool passanytrigger = event.passTriggers_e() || event.passTriggers_ee()
                        || event.passTriggers_eee() || event.passTriggers_m()
                        || event.passTriggers_mm() || event.passTriggers_mmm()
                        || event.passTriggers_em() || event.passTriggers_eem()
                        || event.passTriggers_emm();
    return passanytrigger;
}

bool passLeptonPtThresholds(Event& event){
    event.sortLeptonsByPt();
    if(event.leptonCollection()[0].pt() < 25.
	|| event.leptonCollection()[1].pt() < 15.
        || event.leptonCollection()[2].pt() < 10.) return false;
    return true; 
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

// help function for lepton pair mass constraint //
// here for reference, but not yet sure if need to use,
// also not sure on what lepton collection (loose? fo? tight?)

bool passMllMassVeto( const Event& event ){
    for( LeptonCollection::const_iterator l1It = event.leptonCollection().cbegin(); l1It != event.leptonCollection().cend(); l1It++ ){
	for( LeptonCollection::const_iterator l2It = l1It+1; l2It != event.leptonCollection().cend(); l2It++ ){
            Lepton& lep1 = **l1It;
            Lepton& lep2 = **l2It;
            if((lep1+lep2).mass() < 12.) return false;
	}
    }
    return true;
}


// dedicated functions to check if event passes certain conditions //

bool pass_signalregion(Event& event, const std::string& selectiontype, 
			const std::string& variation, const bool selectbjets){
    // apply basic object selection
    cleanLeptonsAndJets(event);
    // apply MET filters
    if(not event.passMetFilters()) return false;
    // apply trigger thresholds
    if(not passAnyTrigger(event)) return false;
    // select FO leptons
    if(!hasnFOLeptons(event, 3, true)) return false;
    if(not passLeptonPtThresholds(event)) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
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
    if(eventCategory(event, variation, selectbjets)<0) return false;
    return true;
}

std::tuple<int, std::string> pass_signalregion_cutflow(
    Event& event, const std::string& selectiontype,
    const std::string& variation, const bool selectbjets){
    // apply basic object selection
    cleanLeptonsAndJets(event);
    // apply MET filters
    if(not event.passMetFilters()) return std::make_tuple(1, "Fail MET filters");
    // apply trigger thresholds
    if(not passAnyTrigger(event)) return std::make_tuple(2, "Fail trigger");
    // select FO leptons
    if(!hasnFOLeptons(event, 3, true)) return std::make_tuple(3, "Fail 3 tight leptons");
    if(not passLeptonPtThresholds(event)) return std::make_tuple(4, "Fail pT thresholds");
    if(not passPhotonOverlapRemoval(event)) return std::make_tuple(3, "Fail 3 tight leptons");
    // do lepton selection for different types of selections
    if(selectiontype=="3tight"){
        if(!hasnTightLeptons(event, 3, true)) return std::make_tuple(3, "Fail 3 tight leptons");
    } else if(selectiontype=="3prompt"){
        if(!hasnTightLeptons(event, 3, true)) return std::make_tuple(3, "Fail 3 tight leptons");
        if(event.isMC() and !allLeptonsArePrompt(event)) return std::make_tuple(3, "Fail 3 tight leptons");
    } else if(selectiontype=="fakerate"){
        if(hasnTightLeptons(event, 3, false)) return std::make_tuple(-1, "2 <should not happen>");
        if(event.isMC() and !allLeptonsArePrompt(event)) return std::make_tuple(-1, "2 <should not happen>");
    } else if(selectiontype=="2tight"){
        if(!hasnTightLeptons(event, 2, false)) return std::make_tuple(-1, "<should not happen>");
        if(hasnTightLeptons(event, 3, false)) return std::make_tuple(-1, "<should not happen>");
    } else return std::make_tuple(-1, "<should not happen>");
    // Z boson candidate
    if(!event.hasOSSFLightLeptonPair()) return std::make_tuple(5, "Fail Z boson candidate");
    if(!event.hasZTollCandidate(halfwindow)) return std::make_tuple(5, "Fail Z boson candidate");
    // number of jets and b-jets
    if(eventCategory(event, variation, selectbjets)<0) return std::make_tuple(6, "Fail number of (b-) jets");
    return std::make_tuple(7, "Pass all selections");
}

bool pass_signalsideband_noossf(Event& event, const std::string& selectiontype, 
				const std::string& variation, const bool selectbjets){
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return false;
    if(not passAnyTrigger(event)) return false;
    if(!hasnFOLeptons(event, 3, true)) return false;
    if(not passLeptonPtThresholds(event)) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
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
    // -> do not impose this condition here, call further downstream
    //if(eventCategory(event, variation)<0) return false;
    if(variation=="dummy") return true; // dummy to avoid unused parameter warning
    if(selectbjets){} // dummy to avoid unused parameter warning
    return true;
}


// for reference: old noZ selection
/* bool pass_signalsideband_noz(Event& event, const std::string& selectiontype,
                                const std::string& variation){
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return false;
    if(not passAnyTrigger(event)) return false;
    if(!hasnFOLeptons(event,3,true)) return false;
    if(not passLeptonPtThresholds(event)) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
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
    // require presence of an OSSF pair
    if(!event.hasOSSFLightLeptonPair()) return false;
    // inverted cut on Z mass
    if(event.hasZTollCandidate(halfwindow)) return false;
    // number of jets and b-jets
    // -> do not impose this condition here, call manually in runsystematics
    //if(eventCategory(event, variation)<0) return false;
    if(variation=="dummy") return true; // dummy to avoid unused parameter warning
    return true;
} */

bool pass_signalsideband_noz(Event& event, const std::string& selectiontype,
				const std::string& variation, const bool selectbjets){
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return false;
    if(not passAnyTrigger(event)) return false;
    if(!hasnFOLeptons(event,3,true)) return false;
    if(not passLeptonPtThresholds(event)) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
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
    // require presence of OSSF pair
    if(!event.hasOSSFLightLeptonPair()) return false;
    // impose mass constraints on lepton pairs
    // update: only condition is mll>35 GeV for the OSSF pair!
    for( LeptonCollection::const_iterator l1It = event.leptonCollection().cbegin(); l1It != event.leptonCollection().cend(); l1It++ ){
	for( LeptonCollection::const_iterator l2It = l1It+1; l2It != event.leptonCollection().cend(); l2It++ ){
	    Lepton& lep1 = **l1It;
	    Lepton& lep2 = **l2It;
	    //std::cout << l1It - event.leptonCollection().cbegin() << " ";
	    //std::cout << l2It - event.leptonCollection().cbegin() << std::endl;
	    if(oppositeSignSameFlavor(lep1,lep2) && (lep1+lep2).mass() < 35.) return false;
	}
    }
    // impose mass constraint on 3-lepton system in case of eee/mme channel
    if(event.WLepton().isElectron()){
	if(fabs(event.leptonSystem().mass()-particle::mZ)<halfwindow) return false;
    }
    // impose mass constraint on OSSF pair
    if(event.hasZTollCandidate( halfwindow )) return false;
    // number of jets and b-jets
    // -> do not impose this condition here, call further downstream
    //if(eventCategory(event, variation)<0) return false;
    if(variation=="dummy") return true; // dummy to avoid unused parameter warning
    if(selectbjets){} // dummy to avoid unused parameter warning
    return true;
}

bool pass_signalsideband(Event& event, const std::string& selectiontype,
				const std::string& variation, const bool selectbjets){
    bool pass_noossf = pass_signalsideband_noossf(event, selectiontype, variation, selectbjets);
    if( pass_noossf ) return true;
    bool pass_noz = pass_signalsideband_noz(event, selectiontype, variation, selectbjets);
    if( pass_noz ) return true;
    return false;
}

bool pass_wzcontrolregion(Event& event, const std::string& selectiontype,
				const std::string& variation, const bool selectbjets){
    // very similar to signal region but b-jet veto and other specificities
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return false;
    if(not passAnyTrigger(event)) return false;
    if(!hasnFOLeptons(event,3,true)) return false;
    if(not passLeptonPtThresholds(event)) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
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
    // require OSSF pair making a Z mass
    if(!event.hasOSSFLightLeptonPair()) return false;
    if(!event.hasZTollCandidate(halfwindow)) return false;
    // b-jet veto and minimum MET threshold
    if( variation=="all" ){
	if(selectbjets 
	    && event.jetCollection().minNumberOfMediumBTaggedJetsAnyVariation()>0) return false;
	if(event.met().maxPtAnyVariation()<50) return false;
    } else{
	if(selectbjets 
	    && event.getJetCollection(variation).numberOfMediumBTaggedJets()>0) return false;
	if(event.getMet(variation).pt()<50.) return false;
    }
    // calculate mass of 3-lepton system and veto mass close to Z mass
    if(fabs(event.leptonSystem().mass()-particle::mZ)<halfwindow) return false;
    return true;
}

bool pass_zzcontrolregion(Event& event, const std::string& selectiontype,
				const std::string& variation, const bool selectbjets){
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return false;
    if(not passAnyTrigger(event)) return false;
    if(!hasnFOLeptons(event,4,true)) return false;
    if(not passLeptonPtThresholds(event)) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
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
    if(variation=="dummy") return true;
    if(selectbjets){}
    return true;
}

bool pass_zgcontrolregion(Event& event, const std::string& selectiontype,
			    const std::string& variation, const bool selectbjets){
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return false;
    if(not passAnyTrigger(event)) return false;
    if(!hasnFOLeptons(event,3,true)) return false;
    if(not passLeptonPtThresholds(event)) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
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
	for(LeptonCollection::const_iterator lIt2 = lIt1+1; 
	    lIt2!=event.leptonCollection().cend(); lIt2++){
	    Lepton& lep2 = **lIt2;
	    if(fabs((lep1+lep2).mass()-particle::mZ)<halfwindow) pairZmass = true;
	    if(oppositeSignSameFlavor(lep1,lep2) && (lep1+lep2).mass() < 35.) return false;
	}
    }
    if(pairZmass) return false;
    // dummy condition on variation to avoid warnings
    if(variation=="dummy") return true;
    if(selectbjets){}
    return true;
}

bool pass_ttzcontrolregion(Event& event, const std::string& selectiontype, 
			    const std::string& variation, const bool selectbjets){
    // ttz 4l control region, in addition to ttz 3l control region implicit in signal regions
    cleanLeptonsAndJets(event);
    // apply trigger and pt thresholds
    if(not event.passMetFilters()) return false;
    if(not passAnyTrigger(event)) return false;
    if(!hasnFOLeptons(event,4,true)) return false;
    if(not passLeptonPtThresholds(event)) return false;
    if(not passPhotonOverlapRemoval(event)) return false;
    // apply lepton pair mass constraint
    //if(not passMllMassVeto(event)) return false;
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
    // require at least one OSSF pair
    if(!event.hasOSSFLeptonPair()) return false;
    // case 1: only one OSSF pair present
    if( event.numberOfUniqueOSSFLeptonPairs()==1 ){
	// require that this one OSSF pair is close to Z
	if(!event.hasZTollCandidate( halfwindow )) return false;
    }
    // case 2: two OSSF pairs present
    else{
	// first OSSF pair close to Z
	std::pair< std::pair< int, int >, double > temp;
	temp = event.bestZBosonCandidateIndicesAndMass();
	if(fabs(temp.second-particle::mZ) > halfwindow) return false;
	// second OSSF pair not close to Z
	PhysicsObject lvec;
	for(LeptonCollection::const_iterator lIt = event.leptonCollection().cbegin();
	    lIt != event.leptonCollection().cend(); lIt++){
	    Lepton& lep = **lIt;
	    if(lIt-event.leptonCollection().cbegin()==temp.first.first
                or lIt-event.leptonCollection().cbegin()==temp.first.second) continue;
	    lvec += lep;
	}
	double llmass = lvec.mass();
	if(fabs(llmass-particle::mZ) < halfwindow) return false;
    }
    // number of jets at least 2
    if( variation=="all" ){
        if( event.jetCollection().numberOfGoodAnyVariationJets()<2 ) return false;
    } else{
	// require at least 2 central jets
	unsigned int ncentraljets = 0;
	for( auto jetPtr : event.getJetCollection(variation) ){
	    if(fabs(jetPtr->eta())<2.4) ncentraljets++;
	}
	if(ncentraljets<2) return false;
    }
    // dummy condition to avoid unused parameter warning
    if(selectbjets){}
    return true;
}
