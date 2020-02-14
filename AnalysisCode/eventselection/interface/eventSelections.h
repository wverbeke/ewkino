#ifndef eventSelections_H
#define eventSelections_H

// include other parts of framework
#include "../../../Event/interface/Event.h"
#include "../../../constants/particleMasses.h"

bool passES(Event&, const std::string&);
// help functions
void cleanleptoncollection(Event&);
void cleanjetcollection(Event&);
bool hasnFOLeptons(Event&, int);
bool hasnTightLeptons(Event&, int);
int eventCategory(Event&);
// pass functions
bool pass_signalregion(Event&);
bool pass_wzcontrolregion(Event&);
bool pass_zzcontrolregion(Event&);
bool pass_zgcontrolregion(Event&);
bool pass_ttzcontrolregion(Event&);
// for fake rate determination
bool hasgoodjet(Event&);
bool pass_fr_QCD_FO(Event&);
bool pass_fr_QCD_Tight(Event&);
bool pass_fr_EW_FO(Event&);
bool pass_fr_EW_Tight(Event&);

#endif
