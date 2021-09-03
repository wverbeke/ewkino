/*
Test script for electron momentum and energy scale propagation to LeptonCollection.
Similar to ElectronScale_test.cc but using LeptonCollection instead of ElectronCollection
*/


//include class to test 
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/LeptonCollection.h"

//include c++ library classes
#include <iostream>
#include <chrono>
#include <utility>
#include <vector>

void ElectronScale_test2( const std::string inputFilePath,
			      unsigned long nEntries ){

    TreeReader treeReader;
    treeReader.initSampleFromFile( inputFilePath );

    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if( nEntries>0 && nEntries<numberOfEntries) numberOfEntries = nEntries;
    for(unsigned i = 0; i < numberOfEntries; ++i){

        treeReader.GetEntry(i);
	std::cout << "--- Event " << i << " ---" << std::endl;

	LeptonCollection lepCollection( treeReader );
	std::cout << "default collection:" << std::endl;
	for( std::shared_ptr<Lepton> lepPtr: lepCollection ){
	    lepPtr->print(); std::cout << std::endl;
	    //break; // only print first one for now
	}
	std::cout << std::endl;

	LeptonCollection scaleUpCollection = lepCollection.electronScaleUpCollection();
	std::cout << "scale up collection:" << std::endl;
        for( std::shared_ptr<Lepton> lepPtr: scaleUpCollection ){
            lepPtr->print(); std::cout << std::endl;
	    //break; // only print first one for now
        }
	std::cout << std::endl;

	LeptonCollection scaleDownCollection = lepCollection.electronScaleDownCollection();
        std::cout << "scale down collection:" << std::endl;
        for( std::shared_ptr<Lepton> lepPtr: scaleDownCollection ){
            lepPtr->print(); std::cout << std::endl;
            //break; // only print first one for now
        }
        std::cout << std::endl;

	LeptonCollection resUpCollection = lepCollection.electronResUpCollection();
        std::cout << "res up collection:" << std::endl;
        for( std::shared_ptr<Lepton> lepPtr: resUpCollection ){
            lepPtr->print(); std::cout << std::endl;
            //break; // only print first one for now
        }
        std::cout << std::endl;
	
	LeptonCollection resDownCollection = lepCollection.electronResDownCollection();
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

    ElectronScale_test2( input_file_path, number_of_entries );
    return 0;
}
