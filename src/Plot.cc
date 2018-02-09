#include "../interface/Plot.h"

//include plotting scripts
#include "../plotting/plotCode.h"

#include "TCanvas.h"

void Plot::draw(const std::string& outputDirectory, const std::string& analysis, bool log, bool normToData, const std::string& header, TH1D** bkgSyst, const bool* isSMSignal, const bool sigNorm) const{
    std::vector< std::string> names = {"Obs."};
    std::vector< TH1D*> bkgHist;
    std::map < std::string, std::string > translation;
    translation["TT"] = "TT + Jets";
    translation["WJets"] = "WJets";
    translation["TTX"] = "TT + X";
    translation["ST"] = "T + X";
    translation["DY"] = "DY";
    translation["VV"] = "VV";
    for(auto bkgIt = bkg.cbegin(); bkgIt != bkg.cend(); ++bkgIt){
        names.push_back(translation[bkgIt->first]);
        bkgHist.push_back(bkgIt->second.get());
    }
    std::string outputDir(outputDirectory);
    if(outputDir.back() != '/') outputDir.append("/");
    plotDataVSMC(data.get(), &bkgHist[0], &names[0], bkgHist.size(), outputDir + fileName, analysis, log, normToData, header, nullptr, nullptr, nullptr, nullptr, 0, true);
}
