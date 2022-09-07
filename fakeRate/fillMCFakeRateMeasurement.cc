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
    int nargs = 6;
    if( !( argvStr.size() == nargs+1 ) ){
        std::cerr<<"found "<<argc - 1<<" command line args, while "<<nargs<<" are needed."<<std::endl;
        std::cerr<<"usage: ./fillMCFakeRateMeasurement flavour year";
	std::cerr<<" sampleDirectory sampleList sampleIndex isTestRun"<<std::endl;
        return 1;
    }
    std::string flavor = argvStr[1];
    std::string year = argvStr[2];
    std::string sampleDirectory = argvStr[3];
    std::string sampleList = argvStr[4];
    const unsigned sampleIndex = std::stoi(argvStr[5]);
    bool isTestRun = (argvStr[6]=="True" || argvStr[6]=="true");
    setTDRStyle();
    fillMCFakeRateMeasurementHistograms(flavor, year, sampleDirectory, 
					sampleList, sampleIndex, isTestRun);
    std::cerr << "###done###" << std::endl;
}
