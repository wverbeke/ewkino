#include <vector>
#include <utility>
#include <memory>
#include <string>
#include <iterator>
#include <fstream>
#include <thread>

#include "TH1D.h"
#include "TFile.h"

#include "../../../Tools/interface/HistInfo.h"
#include "../../../Tools/interface/stringTools.h"
#include "../../../Tools/interface/analysisTools.h"
#include "../../../Tools/interface/systemTools.h"
#include "../../../Tools/interface/Categorization.h"
#include "../../../fakeRate/interface/SlidingCut.h"

std::map< std::string, double > getCutParameters();
std::tuple< std::vector<double>, SlidingCutCollection, Categorization > getCutCollection();
