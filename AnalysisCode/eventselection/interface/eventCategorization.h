#ifndef eventCategorization_H
#define eventCategorization_H

// include other parts of framework
#include "../../../Event/interface/Event.h"

void initOutputTree(TTree*);
void entryFromEvent(Event&, TTree*);

#endif

