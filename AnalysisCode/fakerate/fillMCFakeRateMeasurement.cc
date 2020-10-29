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
    if( !( argvStr.size() == 5 ) ){
        std::cerr<<"found "<<argc - 1<<" command line args, while 3 are needed."<<std::endl;
        std::cerr<<"usage: ./fillMCFakeRateMeasurement flavour year sampleIndex isTestRun"<<std::endl;
        return 1;
    }
    std::string year = argvStr[2];
    std::string flavor = argvStr[1];
    const unsigned sampleIndex = std::stoi(argvStr[3]);
    bool isTestRun = (argvStr[4]=="True" || argvStr[4]=="true");

    std::string sampleDirectory = "/pnfs/iihe/cms/store/user/llambrec/ntuples_fakerate";
    std::string sampleList = "../samplelists/samples_tunefoselection_";
    sampleList.append(flavor+"_"+year+".txt");

    setTDRStyle();

    fillMCFakeRateMeasurementHistograms(flavor, year, sampleDirectory, 
					sampleList, sampleIndex, isTestRun);
    std::cerr << "###done###" << std::endl;
}
