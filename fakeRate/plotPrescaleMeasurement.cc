// import c++ libraries
#include <vector>
#include <utility>
#include <memory>
#include <string>
#include <iterator>
#include <fstream>
#include <iostream>

// import parts of the framework 
#include "interface/Prescale.h"
#include "interface/fakeRateTools.h"
#include "../plotting/tdrStyle.h"

int main( int argc, char* argv[] ){
    // check command line arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    if( !( argvStr.size() == 5 ) ){
        std::cerr<<"found "<<argc - 1<<" command line args, while 4 are needed."<<std::endl;
        std::cerr<<"usage: ./plotPrescaleMeasurement use_mT year doPlot doSave"<<std::endl;
        return 1;
    }
    std::string year = argvStr[2];
    const bool use_mT = (argvStr[1]=="True" || argvStr[1]=="true");
    const bool doPlot = (argvStr[3]=="True" || argvStr[3]=="true");
    const bool doSave = (argvStr[4]=="True" || argvStr[4]=="true");

    // configuration and variable definition
    const double mTLowerCut_prescaleFit = 90; // 90
    const double mTUpperCut_prescaleFit = 130; // 130

    std::map< std::string, Prescale > prescaleMap;
    std::string file_name = std::string( "prescaleMeasurement_" ) + ( use_mT ? "mT" : "met" );
    file_name.append("_histograms_" + year + ".root");
    TFile* prescale_filePtr = TFile::Open( file_name.c_str() );
    prescaleMap = fakeRate::fitTriggerPrescales_cut( prescale_filePtr, mTLowerCut_prescaleFit,
                    mTUpperCut_prescaleFit, doPlot, doSave );
    prescale_filePtr->Close();
}
