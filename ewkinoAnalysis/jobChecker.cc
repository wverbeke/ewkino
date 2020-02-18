
//include c++ library classes 
#include <string>

//include other parts of framework
#include "../Tools/interface/stringTools.h"
#include "../Tools/interface/systemTools.h"
#include "../Tools/interface/SusyScan.h"
#include "../Tools/interface/analysisTools.h"
#include "../TreeReader/interface/TreeReader.h"


bool massSplittingPresent( const std::string& modelName, const double massSplitting, const std::string& year ){
	std::string deltaM = std::to_string( massSplitting );
	deltaM = stringTools::split( deltaM, "." ).front();
    std::string expectedDirectory = "datacards/" + year + "/" + modelName + "/" + deltaM;

	//if the directory exist check that there are nonempty files
    if( systemTools::directoryExists( expectedDirectory ) ){
		std::vector< std::string > fileList = systemTools::listFiles( expectedDirectory, "neural", "txt" );
        if( !fileList.empty() ){
            return true;
        }
	}
	return false;
}


void checkAllMassSplittings( const std::string& modelName, const std::string& year, const std::string& sampleDirectoryPath ){

    analysisTools::checkYearString( year );

	//make a TreeReader
    TreeReader treeReader( "sampleLists/samples_" + modelName + "_" + year + ".txt", sampleDirectoryPath );

	//use the TreeReader to make a SusyScan, tracking the available mass splittings
    SusyScan susyScan;
    for( const auto& sample : treeReader.sampleVector() ){
        if( sample.isNewPhysicsSignal() ){
            susyScan.addScan( sample );
        }
    }

    for( auto massSplitting : susyScan.massSplittings() ){

		//if the mass splitting is not present, launch a job to process it
		if( !massSplittingPresent( modelName, massSplitting, year ) ){

			std::string deltaM = std::to_string( massSplitting );
			std::string commandString = "./Ewkino_3lOSSF " + modelName + " " + deltaM + " " + year;
			std::string scriptName = "ewkino_" + modelName + "_" + deltaM + "_" + year + "_retry.sh";
			systemTools::submitCommandAsJob( commandString, scriptName, "100:00:00", "", 2 );	
		}
    }
}



int main( int argc, char* argv[] ){
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    const std::string sampleDirectoryPath = "/pnfs/iihe/cms/store/user/wverbeke/ntuples_ewkino/";

    if( argc == 1 ){
        for( const auto& year : { "2016", "2017", "2018" } ){
            for( const auto& model : { "TChiWZ", "TChiSlepSnu_x0p95", "TChiSlepSnu_x0p5", "TChiSlepSnu_x0p05" } ){
                checkAllMassSplittings( model, year, sampleDirectoryPath );
            }
        }
    } else if( argc == 2 ){
        std::string model = argvStr[1];
        for( const auto& year : { "2016", "2017", "2018" } ){
            checkAllMassSplittings( model, year, sampleDirectoryPath );
        }

    } else if( argc == 3 ){
        std::string model = argvStr[1];
        std::string year = argvStr[2];
        checkAllMassSplittings( model, year, sampleDirectoryPath );
    } else {

    }
    return 0;
}
