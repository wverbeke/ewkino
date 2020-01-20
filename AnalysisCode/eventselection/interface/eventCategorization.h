#ifndef eventCategorization_H
#define eventCategorization_H

// include other parts of framework
#include "../../../Event/interface/Event.h"

void initOutputTree(TTree*);
int eventCategory(Event&);
void entryFromEvent(Event&);
std::pair<double,double> pmzcandidates(Event&, Lepton&);
std::pair<double,int> besttopcandidate(Event&, Lepton&, double, double, JetCollection);

#endif

