#ifndef ChargeFlipSelection_H
#define ChargeFlipSelection_H

class Electron;
class Event;

namespace chargeFlips{
    bool passChargeRequirements( const Electron& electron );
    bool passChargeFlipEventSelection( Event& event );
}

#endif 
