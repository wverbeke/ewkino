#ifndef fakeRateSelection_H
#define fakeRateSelection_H

//include c++ library classes
#include <string>
#include <vector>

//include other parts of framework
#include "../../Tools/interface/RangedMap.h"

class Event;

namespace fakeRate{
    
    bool passFakeRateEventSelection( Event& event, bool onlyMuon = false, bool onlyElectrons = false,
	    bool onlyTightLeptons = false, double ptRatioCut=0., double deepFlavorCut=0.,
	    bool requireJet = true, double jetDeltaRCut = 1, double jetPtCut = 25);

}

#endif 
