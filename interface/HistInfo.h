#ifndef HistInfo_H
#define HistInfo_H

//include c++ library classes
#include <string>
#include <memory>

//include ROOT classes
#include "TH1D.h"
#include "TString.h"

//include other parts of code 

class HistInfo{
    public:
        HistInfo() = default;
        HistInfo(const std::string& name, const std::string& x, unsigned bins, double min, double max):
            fileName(name), xLabel(x), nBins(bins), xMin(min), xMax(max) { setMaxBinCenter(); }
        std::shared_ptr<TH1D> makeHist(const std::string& histName) const{
            return std::make_shared<TH1D>( (const TString&) fileName + histName, (const TString&) fileName + histName + ";" + xLabel + ";Events",  nBins, xMin, xMax);
        }
        std::string name() const { return fileName; }
        double maxBinCenter() const { return maxBinC; }
    private:
        std::string fileName;
        std::string xLabel;
        unsigned nBins;
        double xMin, xMax;
        double maxBinC;
        void setMaxBinCenter() { maxBinC = xMax - 0.5*(xMax - xMin)/nBins; }
};
#endif