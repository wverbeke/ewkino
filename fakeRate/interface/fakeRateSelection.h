#ifndef fakeRateSelection_H
#define fakeRateSelection_H

//include c++ library classes
#include <string>

class Event;

double triggerLeptonPtCut( const std::string& );
double triggerJetPtCut( const std::string& );

std::string muonPtToTriggerName( const double );
std::string electronPtToTriggerName( const double );

bool passFakeRateTrigger( const Event& event );
bool passFakeRateEventSelection( Event& event, bool isMuonMeasurement, bool onlyTightLeptons = false, bool requireJet = true, double jetDeltaRCut = 1);

#endif 
