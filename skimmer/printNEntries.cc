/*
Small utility to quickly check number of events in an ntuple
*/

// include c++ library classes 
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include ROOT classes 
#include "TFile.h"
#include "TTree.h"

// include other parts of framework
#include "../TreeReader/interface/TreeReader.h"
#include "../Event/interface/Event.h"


void printNEntries( const std::string& pathToFile ){

    // initialize TreeReader, input files might be corrupt in rare cases
    TreeReader treeReader;
    try{
        treeReader.initSampleFromFile( pathToFile );
    } catch( std::domain_error& ){
        std::cerr << "Can not read file. Returning." << std::endl;
        return;
    }

    long unsigned nentries = treeReader.numberOfEntries();
    std::cout << "number of entries: " << nentries << std::endl;
}


int main( int argc, char* argv[] ){
    std::cerr << "###starting###" << std::endl;

    if( argc != 2 ){
        std::cerr << "printNEntries requires exactly one argument to run : " << std::endl;
	std::cerr << "- path to input file" << std::endl;
        return -1;
    }

    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    std::string& input_file_path = argvStr[1];
    printNEntries( input_file_path );

    std::cerr << "###done###" << std::endl;
    return 0;
}
