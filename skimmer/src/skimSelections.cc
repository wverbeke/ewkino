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
    event.selectLooseLeptons();
    event.cleanElectronsFromLooseMuons();
    event.cleanTausFromLooseLightLeptons();
    if( event.numberOfLightLeptons() != 1 ) return false;

    event.selectGoodJets();
    event.cleanJetsFromLooseLeptons();
    if( event.numberOfJets() < 1 ) return false;
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
