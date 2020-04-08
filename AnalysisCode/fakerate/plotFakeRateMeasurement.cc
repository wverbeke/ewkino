// import c++ libraries
#include <vector>
#include <utility>
#include <memory>
#include <string>
#include <iterator>
#include <fstream>

// include framework
#include "../../fakeRate/interface/fakeRateTools.h"
#include "../../plotting/plotCode.h"
#include "../../plotting/tdrStyle.h"

// include tools
#include "interface/fakeRateMeasurementTools.h"

int main( int argc, char* argv[] ){
   std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    if( !( argvStr.size() == 4 ) ){
        std::cerr<<"found "<<argc - 1<<" command line args, while 3 are needed."<<std::endl;
        std::cerr<<"usage: ./plotFakeRateMeasurement use_mT flavour year"<<std::endl;
        return 1;
    }
    std::string year = argvStr[3];
    std::string flavor = argvStr[2];
    const bool use_mT = (argvStr[1]=="True" || argvStr[1]=="true");

    const double maxFitValue = 20;

    setTDRStyle();

    std::string instanceName = flavor + "_" + year + "_" + ( use_mT ? "mT" : "met" );
    std::string file_name = "fakeRateMeasurement_" + instanceName + "_histograms.root";
    TFile* measurement_filePtr = TFile::Open( file_name.c_str() );
    std::shared_ptr< TH2D > frMap = fakeRate::produceFakeRateMap_cut( measurement_filePtr, maxFitValue );
    measurement_filePtr->Close();

    systemTools::makeDirectory("fakeRateMaps");
    plot2DHistogram( frMap.get(), ( "fakeRateMaps/fakeRateMap_" + instanceName + ".pdf").c_str() );
    TFile* writeFile = TFile::Open( ( "fakeRateMaps/fakeRateMap_data_" + instanceName + ".root" ).c_str(), 
					"RECREATE" );
    frMap->Write( ("fakeRate_" + flavor + "_" + year ).c_str() );
    writeFile->Close();

    return 0;
}
