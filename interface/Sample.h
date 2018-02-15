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
        //Sample(const std::string& file, const std::string& proc, double cross = 0);
        Sample(const std::string& line); 
        Sample(std::istream&); 
        std::string getFileName() const { return fileName; }
        std::string getProcessName() const { return process; } 
        double getXSec() const { return xSec; }
        bool isData() const { return data; }
        bool isSMSignal() const { return smSignal; }
        bool isNewPhysicsSignal() const { return newPhysicsSignal; }
        std::shared_ptr<TFile> getFile(const std::string&) const;
    private:
        void setData(); 
        std::string fileName;
        std::string process;
        double xSec;
        bool data;
        bool smSignal;
        bool newPhysicsSignal;
};
#endif
