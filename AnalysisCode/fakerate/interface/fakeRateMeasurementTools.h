#include <vector>
#include <utility>
#include <memory>
#include <string>
#include <iterator>
#include <fstream>
#include <thread>

#include "TH1D.h"
#include "TFile.h"

#include "../../../Event/interface/Event.h"
#include "../../../Tools/interface/RangedMap.h"
#include "../../../Tools/interface/HistInfo.h"
#include "../../../Tools/interface/stringTools.h"
#include "../../../Tools/interface/analysisTools.h"
#include "../../../Tools/interface/systemTools.h"
#include "../../../fakeRate/interface/fakeRateSelection.h"
#include "../../../fakeRate/interface/fakeRateTools.h"
#include "../../../fakeRate/interface/Prescale.h"
#include "../../../plotting/tdrStyle.h"
#include "../../../plotting/plotCode.h"
#include "../../../weights/interface/ConcreteReweighterFactory.h"
#include "../../../weights/interface/ConcreteLeptonReweighter.h"
#include "../../../weights/interface/ConcreteReweighterLeptons.h"
#include "../../../weights/interface/ConcreteSelection.h"

HistInfo makeVarHistInfo( const unsigned numberOfBins, const double cut, 
			    const double max, const bool useMT = true);
RangedMap< RangedMap< std::shared_ptr< TH1D > > > build2DHistogramMap( 
    const std::vector< double >& ptBinBorders, const std::vector< double >& etaBinBorders, 
    const HistInfo& mtHistInfo, const std::string& name );
void write2DHistogramMap( const RangedMap< RangedMap< std::shared_ptr< TH1D > > >& histMap );
std::shared_ptr< Reweighter > makeLeptonReweighter( const std::string& year, const bool isMuon, 
    const bool isFO);

void fillPrescaleMeasurementHistograms( const std::string& year, 
    const std::string& sampleDirectoryPath, 
    const std::string& sampleListPath, const unsigned sampleIndex,
    const std::vector< std::string >& triggerVector, const bool useMT = true, 
    const double metCut = 0, double mtCut = 0);

void fillFakeRateMeasurementHistograms(const std::string& leptonFlavor, const std::string& year,
    const std::string& sampleDirectory, const std::string& sampleList, const unsigned sampleIndex,
    const std::vector< std::string >& triggerVector,
    const std::map< std::string, Prescale >& prescaleMap, double maxMT, double maxMet);

void fillMCFakeRateMeasurementHistograms(const std::string& leptonFlavor, const std::string& year,
    const std::string& sampleDirectory, const std::string& sampleList, const unsigned sampleIndex);

