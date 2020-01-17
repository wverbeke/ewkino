#ifndef ewkinoSelection_H
#define ewkinoSelection_H

//include ROOT classes
#include "TH2.h"

//include other parts of framework
#include "../../Event/interface/Event.h"

namespace ewkino{
    void applyBaselineObjectSelection( Event& event, const bool allowUncertainties = false );
    bool passLowMllVeto( const Event& event, const double vetoValue = 12. );
    bool passBaselineSelection( Event& event, const bool allowUncertainties = false, const bool bVeto = true, const bool mllVeto = true );
    bool passTriggerSelection( const Event& event );
    bool passPtCuts( const Event& event );
    bool leptonsArePrompt( const Event& event );
    bool leptonsAreTight( const Event& event );
    double fakeRateWeight( const Event& event, const std::shared_ptr< TH2 >& muonMap, const std::shared_ptr< TH2 >& electronMap );
}

#endif
