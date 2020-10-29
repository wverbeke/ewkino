#ifndef eventSelections_H
#define eventSelections_H

// include other parts of framework
#include "../../../Event/interface/Event.h"
#include "../../../constants/particleMasses.h"

bool passES(Event&, const std::string& eventselection, const std::string& selectiontype,
	     const std::string& variation);
// help functions
void cleanLeptonsAndJets(Event&);
JetCollection getjetcollection(const Event&, const std::string& variation);
Met getmet(const Event&, const std::string& variation);
int eventCategory(Event&, const std::string& variation);
bool passAnyTrigger(Event&);
bool passLeptonPtSelections(Event&);
bool hasnFOLeptons(Event&, int, bool select);
bool hasnTightLeptons(Event&, int, bool select);
bool allLeptonsArePrompt(const Event&);
// pass functions
bool pass_signalregion(Event&, const std::string& selectiontype, const std::string& variation);
bool pass_wzcontrolregion(Event&, const std::string& selectiontype, const std::string& variation);
bool pass_zzcontrolregion(Event&, const std::string& selectiontype, const std::string& variation);
bool pass_zgcontrolregion(Event&, const std::string& selectiontype, const std::string& variation);
bool pass_ttzcontrolregion(Event&, const std::string& selectiontype, const std::string& variation);
bool pass_signalsideband_noossf(Event&, const std::string& selectiontype, const std::string& variation);
bool pass_signalsideband_noz(Event&, const std::string& selectiontype, const std::string& variation);

#endif
