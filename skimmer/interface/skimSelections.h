/*
Functions to apply a certain skimming condition. If you need new skimming conditions, add them here, and implement them in src/skimSelection.cc
Make sure to always add new skim conditions to the std::map in the passSkim function for them to be able to be used!
*/


#ifndef skimSelections_H
#define skimSelections_H

//include other parts of framework
#include "../../Event/interface/Event.h"

bool passSingleLeptonSkim( Event& );
bool passDileptonSkim( Event& );
bool passTrileptonSkim( Event& );
bool passFourLeptonSkim( Event& );
bool passFakeRateSkim( Event& );
bool passTriFOLeptonSkim( Event& );
bool passDilightleptonSkim( Event& );
bool passTrilightleptonSkim( Event& );
bool passSkim( Event&, const std::string& skimCondition );

#endif
