// include header 
#include "../interface/eventSelections.h"

//include c++ library classes
#include <functional>

bool passES(Event& event, const std::string& eventselection){
    static std::map< std::string, std::function< bool(Event&) > > ESFunctionMap = {
        { "signalregion", pass_signalregion },
    };
    auto it = ESFunctionMap.find( eventselection );
    if( it == ESFunctionMap.cend() ){
        throw std::invalid_argument( "unknown event selection condition " + eventselection );
    } else {
        return (it->second)(event);
    }
}

void cleanleptoncollection(Event& event){
    event.selectLooseLeptons();
    event.cleanElectronsFromLooseMuons();
    event.cleanTausFromLooseLightLeptons();
}

void cleanjetcollection(Event& event){
    event.cleanJetsFromLooseLeptons();
}

bool pass_signalregion(Event& event){
    
    // clean jet collection (warning: need to check whether after or before lepton cleaning)
    cleanjetcollection(event);

    // clean lepton collections (see also ewkino/skimmer/src/skimSelections.cc)
    cleanleptoncollection(event);

    // select 'good' jets (see ewkino/objectSelection/jetSelector.cc for definition)
    event.selectGoodJets();

    // select FO leptons
    event.selectFOLeptons();
    int nFOLeptons = event.numberOfLeptons();
    if(nFOLeptons != 3) return false;

    // select tight leptons
    event.selectTightLeptons();
    int nTightLeptons = event.numberOfLeptons();
    if(nTightLeptons != 3) return false;

    // Z boson candidate
    if(!event.hasOSSFLightLeptonPair()) return false;
    std::cout<<"until here"<<std::endl;
    if(!event.hasZTollCandidate(7.5)) return false;
    
    return true;
}
