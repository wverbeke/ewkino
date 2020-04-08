#ifndef eventSelections_H
#define eventSelections_H

// include other parts of framework
#include "../../../Event/interface/Event.h"
#include "../../../constants/particleMasses.h"

bool passES(Event&, const std::string&, const bool isnpbackground = false);
// help functions
void cleanleptoncollection(Event&);
void cleanjetcollection(Event&);
bool hasnFOLeptons(Event&, int);
bool hasnTightLeptons(Event&, int);
int eventCategory(Event&);
// pass functions
bool pass_signalregion(Event&, const bool isnpbackground = false);
bool pass_wzcontrolregion(Event&, const bool isnpbackground = false);
bool pass_zzcontrolregion(Event&, const bool isnpbackground = false);
bool pass_zgcontrolregion(Event&, const bool isnpbackground = false);
bool pass_ttzcontrolregion(Event&, const bool isnpbackground = false);
bool pass_signalsideband_noossf(Event&, const bool isnpbackground = false);
bool pass_signalsideband_noz(Event&, const bool isnpbackground = false);

#endif
