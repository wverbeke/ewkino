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
    
        //to prevent overlapping file names when re-using a sample in both the 2016 and 2017 data lists 
        std::string getUniqueName() const { return uniqueName; }

        double getXSec() const { return xSec; }

        bool isData() const { return isDataSample; }

        bool isMC() const { return !isDataSample; }

        bool is2017() const { return is2017Sample; }

        bool is2016() const { return !is2017Sample; }

        bool isSMSignal() const { return smSignal; }

        bool isNewPhysicsSignal() const { return newPhysicsSignal; }

        std::shared_ptr<TFile> getFile(const std::string&) const;

    private:
        void setData(); 

        void set2017();

        void setOptions(const std::string&);


        std::string fileName;
        std::string process;
        std::string uniqueName;

        double xSec;
        bool isDataSample;
        bool is2017Sample;
        bool smSignal;
        bool newPhysicsSignal;
};
//remove .root from fileName to generate unique name
std::string fileNameWithoutExtension(const std::string&);
        
#endif
