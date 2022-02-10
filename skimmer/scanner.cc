//include c++ library classes 
#include <string>
#include <vector>
#include <exception>
#include <iostream>

//include ROOT classes 
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"

//include other parts of framework
#include "../TreeReader/interface/TreeReader.h"
#include "../Tools/interface/stringTools.h"
#include "../Event/interface/Event.h"


void scanFile( const std::string& pathToFile ){

    std::cout << "scanning " << pathToFile << std::endl;

    // initialize TreeReader, input files might be corrupt in rare cases
    TreeReader treeReader;
    try{
        treeReader.initSampleFromFile( pathToFile );
    } catch( std::domain_error& ){
        std::cerr << "Can not read file. Returning." << std::endl;
        return;
    }

    long unsigned nentries = treeReader.numberOfEntries();
    for( long unsigned entry = 0; entry < nentries; ++entry ){

        // build event
        Event event = treeReader.buildEvent( entry, false, false, false, false );

        // optional: apply event selection

	// find properties and do printouts
	double weight = event.weight();
	if( std::abs(weight)>1e4 ){
	    std::cerr << "###error###: large weight: " << weight << std::endl;
	}
    }
}


int main( int argc, char* argv[] ){
    std::cerr << "###starting###" << std::endl;

    if( argc != 2 ){
        std::cerr << "scanner requires exactly one argumente to run : " << std::endl;
	std::cerr << "input_file_path" << std::endl;
        return -1;
    }

    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    std::string& input_file_path = argvStr[1];
    scanFile( input_file_path );

    std::cerr << "###done###" << std::endl;
    return 0;
}
