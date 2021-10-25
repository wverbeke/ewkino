/*
Event selections for the H->W+Ds analysis
*/

#ifndef eventSelections_H
#define eventSelections_H

// include c++ library classes
#include <functional>

// include other parts of framework
#include "../../../Event/interface/Event.h"
#include "../../../constants/particleMasses.h"

namespace eventSelections{

    bool passEventSelection(Event& event, 
			const std::string& eventselection, 
			const std::string& selectiontype, 
			const std::string& variation,
			const bool selectbjets );

    // help functions
    void cleanLeptonsAndJets(Event& event);
    int eventCategory(Event& event, const std::string& variation, const bool selectbjets);
    bool hasLeptonFromMEExternalConversion( const Event& event );
    bool leptonFromMEExternalConversion( const Lepton& lepton );
    bool passPhotonOverlapRemoval( const Event& event );
    bool passAnyTrigger(Event& event);
    bool passLeptonPtThresholds(Event& event);
    bool hasnFOLeptons(const Event& event, int n);
    bool hasnTightLeptons(const Event& event, int n);
    bool allLeptonsArePrompt( const Event& event );

    // dedicated functions to check if event passes certain conditions //
    bool pass_signalregion(Event& event, const std::string& selectiontype, 
	    const std::string& variation, const bool selectbjets);

}

#endif
