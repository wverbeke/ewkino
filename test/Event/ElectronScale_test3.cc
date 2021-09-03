/*
Test script for electron momentum and energy scale propagation to Event.
Similar to ElectronScale_test but using Event instead of ElectronCollection
*/


//include class to test 
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"

//include c++ library classes
#include <iostream>
#include <chrono>
#include <utility>
#include <vector>

void ElectronScale_test( const std::string inputFilePath,
			      unsigned long nEntries ){

    TreeReader treeReader;
    treeReader.initSampleFromFile( inputFilePath );

    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if( nEntries>0 && nEntries<numberOfEntries) numberOfEntries = nEntries;
    for(unsigned i = 0; i < numberOfEntries; ++i){

	std::cout << "--- Event " << i << " ---" << std::endl;

	Event event = treeReader.buildEvent(i);	

	LeptonCollection lepCollection = event.leptonCollection();
	std::cout << "default collection:" << std::endl;
	for( std::shared_ptr<Lepton> lepPtr: lepCollection ){
	    lepPtr->print(); std::cout << std::endl;
	    //break; // only print first one for now
	}
	std::cout << std::endl;

	Event scaleupevent = event.electronScaleUpEvent();
	LeptonCollection scaleUpCollection = event.leptonCollection();
	std::cout << "scale up collection:" << std::endl;
        for( std::shared_ptr<Lepton> lepPtr: scaleUpCollection ){
            lepPtr->print(); std::cout << std::endl;
	    //break; // only print first one for now
        }
	std::cout << std::endl;

	Event scaledownevent = event.electronScaleDownEvent();
	LeptonCollection scaleDownCollection = scaledownevent.leptonCollection();
        std::cout << "scale down collection:" << std::endl;
        for( std::shared_ptr<Lepton> lepPtr: scaleDownCollection ){
            lepPtr->print(); std::cout << std::endl;
            //break; // only print first one for now
        }
        std::cout << std::endl;

	Event resupevent = event.electronResUpEvent();
	LeptonCollection resUpCollection = resupevent.leptonCollection();
        std::cout << "res up collection:" << std::endl;
        for( std::shared_ptr<Lepton> lepPtr: resUpCollection ){
            lepPtr->print(); std::cout << std::endl;
            //break; // only print first one for now
        }
        std::cout << std::endl;

	Event resdownevent = event.electronResDownEvent();	
	LeptonCollection resDownCollection = resdownevent.leptonCollection();
        std::cout << "res down collection:" << std::endl;
        for( std::shared_ptr<Lepton> lepPtr: resDownCollection ){
            lepPtr->print(); std::cout << std::endl;
            //break; // only print first one for now
        } 
        std::cout << std::endl;

    }
}


int main( int argc, char* argv[] ){

    if( argc != 3 ){
        std::cerr << "need command line arguments: " << std::endl;
	std::cerr << "input_file_path, number_of_entries" << std::endl;
        return -1;
    }
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    long unsigned number_of_entries = std::stoul(argvStr[2]);

    ElectronScale_test( input_file_path, number_of_entries );
    return 0;
}
