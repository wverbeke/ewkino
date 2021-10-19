/*
Tools to split samples into sub-categories
*/ 

// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>
#include <map>

// include ROOT classes 
#include "TH1D.h"
#include "TFile.h"
#include "TROOT.h"

// include other parts of framework
#include "../../Tools/interface/stringTools.h"
#include "../../Event/interface/Event.h"
#include "../eventselection/interface/eventSelections.h"

std::vector< std::string > splitProcessNames( const std::string& );

std::string splitProcessName( const std::string&, Event& );
