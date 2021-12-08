// import c++ libraries
#include <vector>
#include <utility>
#include <memory>
#include <string>
#include <iterator>
#include <fstream>

// import tools
#include "interface/fakeRateMeasurementTools.h"

int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;
    // check command line arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    if( !( argvStr.size() == 11 ) ){
        std::cerr<<"found "<<argc - 1<<" command line args, while 10 are needed."<<std::endl;
        std::cerr<<"usage: ./fillMCFakeRateMeasurement flavour year";
	std::cerr<<" sampleDirectory sampleList sampleIndex";
	std::cerr<<" ptRatioCut deepFlavorCut extraCut";
	std::cerr<<" isTestRun nEvents"<<std::endl;
        return 1;
    }
    std::string flavor = argvStr[1];
    std::string year = argvStr[2];
    std::string sampleDirectory = argvStr[3];
    std::string sampleList = argvStr[4];
    const unsigned sampleIndex = std::stoi(argvStr[5]);
    double ptRatioCut = std::stod(argvStr[6]);
    double deepFlavorCut = std::stod(argvStr[7]);
    int extraCut = std::stoi(argvStr[8]);
    bool isTestRun = (argvStr[9]=="True" || argvStr[9]=="true");
    long nEvents = std::stol(argvStr[10]);
    setTDRStyle();
    fillMCFakeRateMeasurementHistograms(flavor, year, sampleDirectory, 
					sampleList, sampleIndex,
					ptRatioCut, deepFlavorCut, extraCut,
					isTestRun, nEvents);
    std::cerr << "###done###" << std::endl;
}
