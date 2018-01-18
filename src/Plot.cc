#include "../interface/Plot.h"

//include plotting scripts
#include "../plotting/plotCode.h"

void Plot::draw(const std::string& analysis, bool log, bool normToData, const std::string& header, TH1D** bkgSyst, const bool* isSMSignal, const bool sigNorm) const{
    std::vector< std::string> names;
    std::vector< TH1D*> bkgHist;
    for(auto bkgIt = bkg.cbegin(); bkgIt != bkg.cend(); ++bkgIt){
        names.push_back(bkgIt->first);
        bkgHist.push_back(bkgIt->second.get());
    }
    plotDataVSMC(data.get(), &bkgHist[0], &names[0], bkgHist.size(), fileName, analysis, log, normToData, header, nullptr, nullptr, nullptr, nullptr, 0, true);
}
