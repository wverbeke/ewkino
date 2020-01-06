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

class Sample{
    friend std::ostream& operator<<( std::ostream& os, const Sample& ); 
    public:
        Sample() = default;

        //initialize from an input text file
        Sample( const std::string& line, const std::string& directory ); 
        Sample( std::istream&, const std::string& directory ); 

        //initialize manually
        Sample( const std::string& directory, const std::string& fileName, const bool is2017, const bool is2018, const bool isData, const std::string& processName = "", const double xSec = 1., const bool isSMSignal = false, const bool isNewPhysicsSignal = false );
        Sample( const std::string& pathToFile, const bool is2017, const bool is2018, const bool isData, const std::string& processName = "", const double xSec = 1., const bool isSMSignal = false, const bool isNewPhysicsSignal = false );

        std::string fileName() const { return _fileName; }
        std::string processName() const { return _processName; } 
    
        //to prevent overlapping file names when re-using a sample in both the 2016 and 2017 data lists 
        std::string uniqueName() const { return _uniqueName; }

        double xSec() const { return _xSec; }

        bool isData() const { return _isData; }
        bool isMC() const { return !_isData; }

        bool is2016() const { return !( _is2017 || _is2018 ); }
        bool is2017() const { return _is2017; }
        bool is2018() const{ return _is2018; }

        bool isSMSignal() const { return _isSMSignal; }
        bool isNewPhysicsSignal() const { return _isNewPhysicsSignal; }

        std::shared_ptr<TFile> filePtr() const;

    private:
        void setIsData(); 
        void setOptions(const std::string&);

        std::string _fileName;
        std::string _directory;
        std::string _uniqueName;
        std::string _processName;

        double _xSec;
        bool _isData;
        bool _is2017;
        bool _is2018;
        bool _isSMSignal;
        bool _isNewPhysicsSignal;

};

//read a txt file containing a list of samples
std::vector< Sample > readSampleList( const std::string& list, const std::string& directory );
        
#endif
