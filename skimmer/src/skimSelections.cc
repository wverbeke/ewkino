#include "../interface/skimSelections.h"

//include c++ library classes
#include <functional>


bool passLeptonicSkim( Event& event, LeptonCollection::size_type numberOfLeptons ){
    event.selectLooseLeptons();
    event.cleanElectronsFromLooseMuons();
    event.cleanTausFromLooseLightLeptons();
    return ( event.numberOfLeptons() >= numberOfLeptons );
}


bool passSingleLeptonSkim( Event& event ){
    return passLeptonicSkim( event, 1 );
}


bool passDileptonSkim( Event& event ){
    return passLeptonicSkim( event, 2 );
}


bool passTrileptonSkim( Event& event ){
    return passLeptonicSkim( event, 3 );
}


bool passFourLeptonSkim( Event& event ){
    return passLeptonicSkim( event, 4 );
}


bool passFakeRateSkim( Event& event ){
    // apply single lepton skim 
    // + require passing at least one of a set of strongly prescaled triggers for data
    if( !passLeptonicSkim( event, 1 ) ) return false;
    if( event.isData() ){
	if( !event.passTriggers_FR() ) return false;
    }
    return true;
}


bool passSkim( Event& event, const std::string& skimCondition ){
    static std::map< std::string, std::function< bool(Event&) > > skimFunctionMap = {
        { "noskim", [](Event&){ return true; } },
        { "singlelepton", passSingleLeptonSkim },
        { "dilepton", passDileptonSkim },
        { "trilepton", passTrileptonSkim },
        { "fourlepton", passFourLeptonSkim },
        { "fakerate", passFakeRateSkim }
    };
    auto it = skimFunctionMap.find( skimCondition );
    if( it == skimFunctionMap.cend() ){
        throw std::invalid_argument( "unknown skim condition " + skimCondition );
    } else {
        return (it->second)(event);
    }
}
