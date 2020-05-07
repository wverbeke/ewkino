#ifndef eventSelections_H
#define eventSelections_H

// include other parts of framework
#include "../../../Event/interface/Event.h"
#include "../../../constants/particleMasses.h"

bool passES(Event&, const std::string&, const std::string&, 
		const bool isdataforbackground = false);
// help functions
void cleanleptoncollection(Event&);
void cleanjetcollection(Event&);
//JetCollection getjetcollection(const Event&, const std::string& uncertainty = "nominal");
//Met getmet(const Event&, const std::string& uncertainty = "nominal");
bool hasnFOLeptons(Event&, int, const std::string&, bool select = true);
bool hasnTightLeptons(Event&, int, const std::string&, bool select = true);
// pass functions
bool pass_signalregion(Event&, const std::string&, const bool isdataforbackground = false);
bool pass_wzcontrolregion(Event&, const std::string&, const bool isdataforbackground = false);
bool pass_zzcontrolregion(Event&, const std::string&, const bool isdataforbackground = false);
bool pass_zgcontrolregion(Event&, const std::string&, const bool isdataforbackground = false);
bool pass_ttzcontrolregion(Event&, const std::string&, const bool isdataforbackground = false);
bool pass_signalsideband_noossf(Event&, const std::string&, const bool isdataforbackground = false);
bool pass_signalsideband_noz(Event&, const std::string&, const bool isdataforbackground = false);

#endif
