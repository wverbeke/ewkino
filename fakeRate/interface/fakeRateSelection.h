#ifndef fakeRateSelection_H
#define fakeRateSelection_H

//include c++ library classes
#include <string>
#include <vector>

//include other parts of framework
#include "../../Tools/interface/RangedMap.h"

class Event;

namespace fakeRate{
    
    std::map< std::string, double > mapTriggerToLeptonPtThreshold( const std::vector< std::string >& triggerNames );
    std::map< std::string, double > mapTriggerToJetPtThreshold( const std::vector< std::string >& triggerNames );

    RangedMap< std::string > mapLeptonPtToTriggerName( const std::vector< std::string >& triggerNames, const bool isMuon );
    RangedMap< std::string > mapMuonPtToTriggerName( const std::vector< std::string >& triggerNames );
    RangedMap< std::string > mapElectronPtToTriggerName( const std::vector< std::string >& triggerNames );

    //bool passFakeRateTrigger( const Event& event, RangedMap< std::string >& triggerThresholdMap  );
    bool passFakeRateEventSelection( Event& event, bool onlyMuon = false, bool onlyElectrons = false, bool onlyTightLeptons = false, bool requireJet = true, double jetDeltaRCut = 1, double jetPtCut = 25 );

    bool passTriggerJetSelection( Event& event, const std::string& trigger, std::map< std::string, double >& triggerToJetPtMap );

}

#endif 
