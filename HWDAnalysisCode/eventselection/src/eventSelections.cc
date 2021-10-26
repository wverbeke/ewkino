/*
Event selections for the H->W+Ds analysis
*/

// include header 
#include "../interface/eventSelections.h"

//include c++ library classes
#include <functional>


void checkArguments( Event& event,
		     const std::string& selectiontype,
                     const std::string& variation){
    // internal helper function to check validity of some arguments.
    // common to passEventSelection and passCutFlow
    
    // check if selectiontype is valid
    std::vector< std::string > seltypes{ "tight" };
    if( std::find(seltypes.cbegin(), seltypes.cend(), selectiontype)==seltypes.cend() ){
        throw std::invalid_argument("ERROR: unknown selection type: "+selectiontype);
    }
    // check if variation is valid
    // note: need to check total JEC and split JEC variations separately
    std::vector< std::string > variations{ "JECUp","JECDown","JERUp","JERDown",
                                            "UnclUp","UnclDown",
                                            "nominal","all" };
    bool istot = ( std::find(variations.cbegin(), variations.cend(), variation)!=variations.cend() );
    bool issplit = event.jetInfo().hasJECVariation( variation );
    if( !istot && !issplit ){
        throw std::invalid_argument("ERROR: unknown variation: "+variation);
    }
}

bool eventSelections::passEventSelection(Event& event, 
			const std::string& eventselection, 
			const std::string& selectiontype, 
			const std::string& variation,
			const bool selectbjets ){
    // arguments:
    // - event: object of type Event
    // - eventselection: event selection identifier, see map below for allowd values
    // - selectiontype: selection type identifier, i.e. "tight"
    //   NOTE: to be extended at a later stage in the analysis
    // - variation: variation identifier, i.e. "all", "nominal", or any JEC/JER/Uncl variation
    // - selectbjets: boolean whether to select b-jets 
    //   (set to false for b-tag shape normalization)

    checkArguments(event, selectiontype, variation);
    // map event selection to function
    static std::map< std::string, std::function<
    bool(Event&, const std::string&, const std::string&, const bool) > > 
        ESFunctionMap = {
	{ "signalregion", pass_signalregion },
	{ "donly", pass_donly },
        };
    auto it = ESFunctionMap.find( eventselection );
    if( it == ESFunctionMap.cend() ){
        throw std::invalid_argument( "ERROR: unknown event selection condition " + eventselection );
    }
    return (it->second)(event, selectiontype, variation, selectbjets);
}

std::tuple<int, std::string> eventSelections::passCutFlow(Event& event,
				const std::string& eventselection,
				const std::string& selectiontype,
				const std::string& variation,
				const bool selectbjets ){
    // similar to passEventSelection, but returning and int+description rather than a bool.
    // this allows for cutflow studies
    checkArguments(event, selectiontype, variation);
    static std::map< std::string, std::function<
    std::tuple<int, std::string>(Event&, const std::string&, const std::string&, const bool) > >
        CFFunctionMap = {
        { "signalregion", pass_signalregion_cutflow },
        };
    auto it = CFFunctionMap.find( eventselection );
    if( it == CFFunctionMap.cend() ){
        throw std::invalid_argument( "ERROR: unknown event selection condition " + eventselection );
    }
    return (it->second)(event, selectiontype, variation, selectbjets);
}


// help functions for event cleaning //

void eventSelections::cleanLeptonsAndJets(Event& event){
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


int eventSelections::eventCategory(Event& event, 
				   const std::string& variation, 
				   const bool selectbjets){
    // determine the event category
    // NOTE: not yet defined for this analysis
    return 0;
}


// help functions for determining if event belongs to a sub-category //

bool eventSelections::hasLeptonFromMEExternalConversion( const Event& event ){
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( leptonPtr->isFO() && leptonFromMEExternalConversion( *leptonPtr ) ){
            return true;
        }
    }
    return false;
}

// help functions for overlap removal between inclusive and dedicated photon samples //

bool eventSelections::leptonFromMEExternalConversion( const Lepton& lepton ){
    // from Willem's ewkinoAnalysis code
    if( !( lepton.matchPdgId() == 22 ) ) return false;
    if( !( lepton.isPrompt() && lepton.provenanceConversion() == 0 ) ) return false;
    return true;
}


bool eventSelections::passPhotonOverlapRemoval( const Event& event ){
    // from Willem's ewkinoAnalysis code
    // to be re-checked once needed, especially hard-coded naming conventions!
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

    if( !( isPhotonSample || isInclusiveSample ) ) return true;

    bool usePhotonSample = false;
    // method 1: check for prompt leptons matched to photons without provenanceConversion
    //if( hasLeptonFromMEExternalConversion( event ) ) usePhotonSample = true;
    // method 2: simply check if all leptons are prompt (note: need to select FO leptons first!)
    if( allLeptonsArePrompt(event) ){
	usePhotonSample = true;
    }

    if( isInclusiveSample ) return !usePhotonSample;
    else if( isPhotonSample ) return usePhotonSample;
    return true;
}


// help functions for trigger and pt-threshold selections //

bool eventSelections::passAnyTrigger(Event& event){
    // NOTE: still to decide what triggers to use in this analysis
    bool passanytrigger = event.passTriggers_e() || event.passTriggers_m();
    return passanytrigger;
}


bool eventSelections::passLeptonPtThresholds(Event& event){
    // NOTE: still to decide what pt threshold to use in this analysis
    //       (also depends on trigger)
    event.sortLeptonsByPt();
    if( event.leptonCollection()[0].pt() < 25. ) return false;
    return true; 
}


// help functions for determining the number of leptons with correct ID //

bool eventSelections::hasnFOLeptons(const Event& event, int n){
    int nFO = 0;
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( leptonPtr->isFO() ){ ++nFO; }
    }
    if( n!=nFO ){ return false; }
    return true;
}


bool eventSelections::hasnTightLeptons(const Event& event, int n){
    int nTight = 0;
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( leptonPtr->isTight() ){ ++nTight; }
    }
    if( n!=nTight ){ return false; }
    return true;
}


bool eventSelections::allLeptonsArePrompt( const Event& event ){
    for( const auto& leptonPtr : event.leptonCollection() ){
        if( !leptonPtr->isPrompt() ) return false;
    }
    return true;
}


// help functions for determining the number of D mesons with correct ID //

bool eventSelections::hasnDMesons(const Event& event, int n){
    int nD = 0;
    for( const auto& dmesonPtr : event.dmesonCollection() ){
        if( dmesonPtr->isGood() ){ ++nD; }
    }
    if( n!=nD ){ return false; }
    return true;
}


// dedicated functions to check if event passes certain conditions //

bool eventSelections::pass_signalregion(Event& event, 
	    const std::string& selectiontype, 
	    const std::string& variation, 
	    const bool selectbjets){
    // apply basic object selection
    cleanLeptonsAndJets(event);
    // apply MET filters
    if(not event.passMetFilters()) return false;
    // apply trigger thresholds
    if(not passAnyTrigger(event)) return false;
    // select a single tight lepton
    if(!hasnTightLeptons(event, 1)) return false;
    event.selectTightLeptons();
    // lepton pt threshold
    if(not passLeptonPtThresholds(event)) return false;
    // overlap removal between dedicated photon and general samples
    if(not passPhotonOverlapRemoval(event)) return false;
    // MET
    if(event.getMet(variation).pt() < 10.) return false;
    // do lepton selection for different types of selections
    if(selectiontype!="tight") return false; // NOTE: to be extended
    // select Ds mesons
    if(not hasnDMesons(event, 1)) return false;
    event.selectGoodDMesons();   
    return true;
}


std::tuple<int,std::string> eventSelections::pass_signalregion_cutflow(
	    Event& event,
	    const std::string& selectiontype,
	    const std::string& variation,
	    const bool selectbjets){
    // copy of pass_signalregion, but returning an int+description instead of a bool.
    // this allows to make cutflow studies.
    cleanLeptonsAndJets(event);
    if(not event.passMetFilters()){ 
	return std::make_tuple(0, "fail MET filters"); }
    if(not passAnyTrigger(event)){ 
	return std::make_tuple(1, "fail trigger"); }
    if(!hasnTightLeptons(event, 1)){ 
	return std::make_tuple(2, "fail single tight lepton"); }
    event.selectTightLeptons();
    if(not passLeptonPtThresholds(event)){ 
	return std::make_tuple(3, "fail lepton pt threshold"); }
    if(not passPhotonOverlapRemoval(event)){
	return std::make_tuple(4, "fail photon overlap removal"); }
    if(event.getMet(variation).pt() < 10.){ 
	return std::make_tuple(5, "fail met pt threshold"); }
    if(not hasnDMesons(event, 1)){
	return std::make_tuple(6, "fail single good Ds meson"); }
    event.selectGoodDMesons();
    return std::make_tuple(-1, "pass");
}


bool eventSelections::pass_donly(
	    Event& event,
	    const std::string& selectiontype,
            const std::string& variation,
            const bool selectbjets){
    // selection that only cares about the presence of exactly one good D meson
    if(not hasnDMesons(event, 1)) return false;
    event.selectGoodDMesons();
    return true;      
}
