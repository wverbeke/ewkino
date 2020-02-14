#ifndef eventFlattening_H
#define eventFlattening_H

// include other parts of framework
#include "../../../Event/interface/Event.h"

void initOutputTree(TTree*);
//int eventCategory(Event&); move to other file
void eventToEntry(Event&);
std::pair<double,double> pmzcandidates(Event&, Lepton&);
std::pair<double,int> besttopcandidate(Event&, Lepton&, double, double);

#endif

