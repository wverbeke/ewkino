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
    if( !( argvStr.size() == 10 ) ){
        std::cerr<<"found "<<argc - 1<<" command line args, while 9 are needed."<<std::endl;
        std::cerr<<"usage: ./fillMCFakeRateMeasurement flavour year";
	std::cerr<<" sampleDirectory sampleList sampleIndex";
	std::cerr<<" ptRatioCut deepFlavorCut";
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
    bool isTestRun = (argvStr[8]=="True" || argvStr[8]=="true");
    long nEvents = std::stol(argvStr[9]);
    setTDRStyle();
    fillMCFakeRateMeasurementHistograms(flavor, year, sampleDirectory, 
					sampleList, sampleIndex,
					ptRatioCut, deepFlavorCut,
					isTestRun, nEvents);
    std::cerr << "###done###" << std::endl;
}
