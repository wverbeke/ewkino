#ifndef eventSelections_H
#define eventSelections_H

// include other parts of framework
#include "../../../Event/interface/Event.h"

bool passES(Event&, const std::string&);
void cleanleptoncollection(Event&);
void cleanjetcollection(Event&);
bool pass_signalregion(Event&);

#endif
