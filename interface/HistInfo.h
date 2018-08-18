#ifndef HistInfo_H
#define HistInfo_H

//include c++ library classes
#include <string>
#include <memory>

//include ROOT classes
#include "TH1D.h"

//include other parts of code 

class HistInfo{
    public:
        HistInfo() = default;

        //constructor takes an optional vector of strings to label the bins 
        HistInfo( const std::string& name, const std::string& x, unsigned bins, double min, double max, const std::vector< std::string>& binLabelList = std::vector<std::string>() ):
            fileName(name), xLabel(x), nBins(bins), xMin(min), xMax(max), binLabels(binLabelList)
        { 
            setMaxBinCenter();
        }


        std::shared_ptr<TH1D> makeHist( const std::string& histName ) const{
            std::shared_ptr<TH1D> hist = std::make_shared<TH1D>( (fileName + histName).c_str(), (fileName + histName + ";" + xLabel + ";Events").c_str(),  nBins, xMin, xMax);

            //set bin labels if they were initialized 
            if( !( binLabels.empty() ) ){
            	for(unsigned bin = 0; bin < nBins; ++bin){
        			hist->GetXaxis()->SetBinLabel( bin + 1, binLabels[bin].c_str() );
    			}
            }
            return hist;
        }


        std::string name() const { return fileName; }
        double maxBinCenter() const { return maxBinC; }
    private:
        std::string fileName;
        std::string xLabel;
        unsigned nBins;
        double xMin, xMax;
        std::vector< std::string > binLabels;
        double maxBinC;

        void setMaxBinCenter() { maxBinC = xMax - 0.5*(xMax - xMin)/nBins; }
};
#endif
