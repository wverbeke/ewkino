#ifndef tools_h
#define tools_h

//include c++ library classes
#include<tuple>
#include<string>
//include ROOT classes
#include "TH1D.h"

namespace tools{
    //show progress bar
    void printProgress(double);
    //Set negative histogram bins to zero
    void setNegativeZero(TH1D*);
    //String manipulation to read sample file
    void removeBackSpaces(std::string& s);
    void removeFrontSpaces(std::string&s);
    void cleanSpaces(std::string& s);
    std::string extractFirst(std::string& s);
    //translate line from sample file into std::tuple
    std::tuple<std::string, std::string, double> readSampleLine(std::string& line);
    //print a combine datacard
    void printDataCard(const double obsYield, const double sigYield, const std::string& sigName, const double* bkgYield, const unsigned nBkg, const std::string* bkgNames, const std::vector<std::vector<double> >& systUnc, const unsigned nSyst, const std::string* systNames, const std::string* systDist, const std::string& cardName, const bool shapeCard = false, const std::string& shapeFileName = "");
    inline double mt(const TLorentzVector& v1, const TLorentzVector& v2){
        return sqrt(2*v1.Pt()*v2.Pt()*( 1 - cos( v1.Phi()-v2.Phi() ) ) );       //this definition assumes the ultarelativistic limit
    }
}
#endif
