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
    // map event selection to function
    static std::map< std::string, std::function< 
	bool(Event&, const std::string&, const std::string&, const bool) > > 
	    ESFunctionMap = {
		//{ "signalregion", pass_signalregion },
		{ "wzcontrolregion", pass_wzcontrolregion },
		{ "zzcontrolregion", pass_zzcontrolregion },
		{ "zgcontrolregion", pass_zgcontrolregion },
		//{ "ttzcontrolregion", pass_ttzcontrolregion },
		//{ "signalsideband_noossf", pass_signalsideband_noossf },
		//{ "signalsideband_noz", pass_signalsideband_noz },
		//{ "signalsideband", pass_signalsideband },
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

// help functions for determining if event belongs to a sub-category //

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
    for( LeptonCollection::const_iterator l1It = event.leptonCollection().cbegin(); 
	l1It != event.leptonCollection().cend(); l1It++ ){
	for( LeptonCollection::const_iterator l2It = l1It+1; 
	    l2It != event.leptonCollection().cend(); l2It++ ){
            Lepton& lep1 = **l1It;
            Lepton& lep2 = **l2It;
            if((lep1+lep2).mass() < 12.) return false;
	}
    }
    return true;
}


// dedicated functions to check if event passes certain conditions //

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