#ifndef ChargeFlipSelection_H
#define ChargeFlipSelection_H

class Electron;
class Muon;
class Event;

namespace chargeFlips{
    bool passChargeRequirements( const Electron& electron );
    bool passChargeRequirements( const Muon& muon );
    bool passChargeFlipEventSelection( Event& event, const bool diElectron = true, const bool onZ = true, const bool bVeto = true );
}

#endif 
