#ifndef tools_h
#define tools_h

//include c++ library classes
#include<tuple>
#include<string>
//include ROOT classes
#include "TH1D.h"
#include "TLorentzVector.h"

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

    //call std::system, but allow an std::string to be used
    void system(const std::string&);

    //delete file of given name
    void deleteFile(const std::string&);

    //get CMSSW directory currently used
    std::string CMSSWDirectory();

    //initialize a submission script
    std::ostream& initScript(std::ostream&);

    //sleep for x seconds
    void sleep(unsigned);

    //submit script of given name and catch submission errors
    void submitScript(const std::string&, const std::string&); 

    //check whether there are running jobs
    bool runningJobs(const std::string& jobName = "");

    //get current directory
    std::string currentDirectory();

    //list all files in a directory and save this to a txt file of given name
    void makeFileList(const std::string&, const std::string&);

    //add trailing / to directoryName if needed
    std::string formatedDirectoryName(const std::string&);     
}
#endif
