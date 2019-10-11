/*
Some simple tools which are useful in most physics analysis tasks
*/

#ifndef analysisTools_h
#define analysisTools_h

//include c++ library classes
#include <string>
#include <utility>

//include ROOT classes
#include "TH1D.h"

namespace analysisTools{

    //show progress bar
    void printProgress(double);

    //Set negative histogram bins to zero
    void setNegativeBinsToZero(TH1D*);
    void setNegativeBinsToZero(const std::shared_ptr<TH1D>&);

    //print a combine datacard
    void printDataCard(const std::string& cardName, const double obsYield, const double sigYield, const std::string& sigName, const double* bkgYield, const unsigned nBkg, const std::string* bkgNames, const std::vector<std::vector<double> >& systUnc = std::vector< std::vector< double > >(), const unsigned nSyst = 0, const std::string* systNames = nullptr, const std::string* systDist = nullptr, const bool shapeCard = false, const std::string& shapeFileName = "", const bool autoMCStats = false);

    //check from sample name whether it is 2016, 2017 or 2018
    bool fileIs2017( const std::string& filePath );
    bool fileIs2018( const std::string& filePath );
    bool fileIs2016( const std::string& filePath );
    
    //first bool checks whether file is 2017, second whether it is 2018
    std::pair< bool, bool > fileIs2017Or2018( const std::string& filePath );
}
#endif
