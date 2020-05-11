#include "../Tools/interface/mergeAndRemoveOverlap.h"

//include c++ library classes
#include <string> 
#include <vector>
#include <iostream>

//include other parts of framework
#include "../Tools/interface/systemTools.h"
#include "../Tools/interface/stringTools.h"


int main( int argc, char* argv[] ){

    //convert all input to std::string format for easier handling
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    //merge data files present in input directory ( separately for the years )
    if( argc == 3 && !stringTools::stringContains( argvStr[2], ".root" ) ){
        const std::string input_directory = argvStr[1];
        const std::string output_directory = argvStr[2];
        const std::vector< std::string > dataIdentifiers = { "DoubleEG", "DoubleMuon", "MuonEG", "SingleElectron", "SingleMuon", "MET", "JetHT", "EGamma" };
        const std::vector< std::string > presentFiles = systemTools::listFiles( input_directory, "", ".root" );

        for( const auto& year : { "Summer16", "Fall17", "Autumn18" } ){
            std::vector< std::string > filesToMerge;
            for( const auto& fileName : presentFiles ){

                if( !stringTools::stringContains( fileName, year ) ) continue;

                //samples might have genMET in their name, and will pass the dataIdentifier check without this continue
                if( stringTools::stringContains( fileName, "gen" ) ) continue;
                
                //make sure file corresponds to data
                bool identified = false;
                for( const auto& id : dataIdentifiers ){
                    if( stringTools::stringContains( fileName, id ) ){
                        identified = true;
                        break;
                    }
                }
                if( identified ){
                    filesToMerge.push_back( fileName );
                }
            }
            
            //submit job to merge files 
            const std::string outputPath = stringTools::formatDirectoryName( output_directory ) + "data_combined_" + year + ".root";
            std::string mergeCommand = "./combinePD " + outputPath;
            for( const auto& input : filesToMerge ){
                mergeCommand += ( " " + input );
            }
            systemTools::submitCommandAsJob( mergeCommand, std::string( "combinePD_" ) + year + ".sh", "169:00:00" );
        }


    
    //one merging job
    } else if( argc > 2 ){
        std::string outputPath = argvStr[1];

        std::vector< std::string > inputFiles( argvStr.begin() + 2, argvStr.end() );
        mergeAndRemoveOverlap( inputFiles, outputPath );

	    return 0;
    } else {
        std::cerr << argc - 1 << " command line arguments given, while at least 2 are expected." << std::endl;
        std::cerr << "Usage: ./combinePD < output_path > < space separated list of input files >" << std::endl;
        std::cerr << "Usage: ./combinePD < input_directory containing data sample > < space separated list of input files >" << std::endl;
        return 1;
    }
}
