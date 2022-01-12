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

    // check from sample name what era it is
    bool fileIs2016( const std::string& filePath );
    bool fileIs2016PreVFP( const std::string& filePath );
    bool fileIs2016PostVFP( const std::string& filePath );
    bool fileIs2017( const std::string& filePath );
    bool fileIs2018( const std::string& filePath );
    
    //first bool checks whether file is 2017, second whether it is 2018
    std::pair< bool, bool > fileIs2017Or2018( const std::string& filePath );

    //check if a string is either 2016, 2016PreVFP, 2016PostVFP, 2017 or 2018, 
    // if not throw an error
    void checkYearString( const std::string& );

    //check from a sample name whether the sample is a SUSY scan or not
    bool sampleIsSusy( const std::string& filePath );
}
#endif
