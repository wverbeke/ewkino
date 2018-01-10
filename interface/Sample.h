/*
Class used for storing the info related to a particular sample: xSec, process name, file name
*/
#ifndef Sample_H
#define Sample_H

//include c++ library classes
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <memory>

//include ROOT classes
#include "TFile.h"
#include "TString.h"

class Sample{
    friend std::ostream& operator<<(std::ostream& os, const Sample&); 
    public:
        Sample() = default;
        Sample(const std::string& file, const std::string& proc, double cross = 0);
        Sample(const std::string& line); 
        Sample(std::istream&); 
        std::string getFileName() const {return fileName;}
        std::string getProc() const {return process;} 
        double getXSec() const { return xSec;}
        bool isData() const { return data;}
        std::shared_ptr<TFile> getFile(const std::string&) const;
    private:
        std::string fileName;
        std::string process;
        double xSec;
        bool data;
};

/*
Sample::Sample(const std::string& file, const std::string& proc, double cross): fileName(file), process(proc), xSec(cross){
    //check if sample is data
    data = false;
    static std::vector<std::string> dataNames = {"data", "SingleMuon", "SingleElectron", "SingleMuon", "DoubleMuon", "DoubleEG"};
    for(auto it = dataNames.cbegin(); it != dataNames.cend(); ++it){
        if(fileName.find(*it) != std::string::npos){
            data = true;
        }
    }
}

Sample::Sample(const std::string& line){
    std::istringstream stream(line);
    stream >> process >> fileName >> xSec;
}

Sample::Sample(std::istream& is){
    //read sample info from txt file
    if(!is.eof()){
        std::string line;
        std::getline(is, line);
        *this = Sample(line); 
    }
}

std::shared_ptr<TFile> Sample::getFile(const std::string& directory) const{
    std::string temp = directory;
    if(*(temp.cend() - 1) != '/') temp.pop_back();
    return std::make_shared<TFile>((const TString&) temp + "/" + fileName, "read");
}

//print Sample info
std::ostream& operator<<(std::ostream& os, const Sample& sam){
    os << sam.process << "\t" << sam.fileName << "\t" << sam.xSec;
    return os;
}
*/
#endif
