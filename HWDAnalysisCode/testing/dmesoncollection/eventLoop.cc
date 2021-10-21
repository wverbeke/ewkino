/*
Loop over entries of an ntuple, create Events, and do printouts of leptons and Ds mesons.
*/

//include c++ library classes 
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include ROOT classes 
#include "TFile.h"
#include "TTree.h"

// include other parts of framework
#include "../../../TreeReader/interface/TreeReader.h"
#include "../../../Tools/interface/stringTools.h"
#include "../../../Event/interface/Event.h"

// include other parts of the analysis code
#include "../../tools/interface/rootFileTools.h"

void eventLoop( const std::string& pathToFile, long nEvents ){
    
    // initialize TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );

    // do event loop
    long numberOfEntries = treeReader.numberOfEntries();
    if( nEvents<0 || nEvents>numberOfEntries ) nEvents = numberOfEntries;
    for(long entry = 0; entry < nEvents; entry++){
	if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<nEvents<<std::endl;
	Event event = treeReader.buildEvent(entry, false, false, false, false, true);
	std::cout << "=== Event ===" << std::endl;

	// try to access the D meson collection
	DMesonCollection dmesonCollection = event.dmesonCollection();
	std::cout << "size of d meson collection: " << dmesonCollection.size() << std::endl;

	// try to access individual D mesons
	for( std::shared_ptr<DMeson> dmeson: dmesonCollection ){
	    std::cout << "dmeson: " << std::endl;
	    std::cout << dmeson << std::endl; // pointer to address
	    std::cout << *dmeson << std::endl; // D meson properties
	}

	// do some selections
	dmesonCollection.selectGoodDMesons();
	std::cout << "after selection: " << dmesonCollection.size() << std::endl;
    }
}

int main( int argc, char* argv[] ){

    std::cerr<<"###starting###"<<std::endl;
    if( argc != 3 ){
        std::cerr << "ERROR: eventLoop requires 2 arguments to run: " << std::endl;
	std::cerr << "- input file" << std::endl;
	std::cerr << "- number of events" << std::endl;
        return -1;
    }
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    long nprocess = std::stol(argvStr[2]);
    bool validInput = rootFileTools::nTupleIsReadable( input_file_path );
    if(!validInput){ return -1; }
    eventLoop( input_file_path, nprocess );
    std::cerr<<"###done###"<<std::endl;
    return 0;
}
