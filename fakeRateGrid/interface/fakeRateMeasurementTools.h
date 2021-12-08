#include <vector>
#include <utility>
#include <memory>
#include <string>
#include <iterator>
#include <fstream>
#include <thread>

#include "TH1D.h"
#include "TFile.h"

#include "../../Event/interface/Event.h"
#include "../../Tools/interface/RangedMap.h"
#include "../../Tools/interface/HistInfo.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/analysisTools.h"
#include "../../Tools/interface/systemTools.h"
#include "fakeRateSelection.h"
#include "fakeRateTools.h"
#include "../../plotting/tdrStyle.h"
#include "../../plotting/plotCode.h"
#include "../../weights/interface/ConcreteReweighterFactory.h"
#include "../../weights/interface/ConcreteLeptonReweighter.h"
#include "../../weights/interface/ConcreteReweighterLeptons.h"
#include "../../weights/interface/ConcreteSelection.h"
#include "progressTracker.h"

void fillMCFakeRateMeasurementHistograms(const std::string& leptonFlavor, const std::string& year,
    const std::string& sampleDirectory, const std::string& sampleList, const unsigned sampleIndex,
    double ptRatioCut, double deepFlavorCut, int extraCut,
    const bool isTestRun = false, const long nEvents = -1 );
