/*
Loop over entries of an ntuple, create Events, and do printouts of gen-level properties.
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

	// try to access the gen-level object
	HToWDGenInfo geninfo = event.htowdGenInfo();
	std::cout << "found gen info" << std::endl;

	// try to access individual variables
	std::cout << "gen-level H->WD event type: " << geninfo.genhtowdEventType() << std::endl;
	std::cout << "gen-level H boson pt: " << geninfo.genhtowdHPt() << std::endl;
	std::cout << "gen-level H boson eta: " << geninfo.genhtowdHEta() << std::endl;
	std::cout << "gen-level H boson phi: " << geninfo.genhtowdHPhi() << std::endl;
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
