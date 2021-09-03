/*
Test script for ElectronCollection.
The following items will be tested:
  - electron momentum and energy scale and resolution 
*/


//include class to test 
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/LeptonCollection.h"
#include "../../Event/interface/LightLeptonCollection.h"
#include "../../Event/interface/ElectronCollection.h"

//include c++ library classes
#include <iostream>
#include <chrono>
#include <utility>
#include <vector>

void ElectronCollection_test( const std::string inputFilePath,
			      unsigned long nEntries ){

    TreeReader treeReader;
    treeReader.initSampleFromFile( inputFilePath );

    long unsigned numberOfEntries = treeReader.numberOfEntries();
    if( nEntries>0 && nEntries<numberOfEntries) numberOfEntries = nEntries;
    for(unsigned i = 0; i < numberOfEntries; ++i){

        treeReader.GetEntry(i);
	std::cout << "--- Event " << i << " ---" << std::endl;

	ElectronCollection electronCollection( treeReader );
	std::cout << "default collection:" << std::endl;
	for( std::shared_ptr<Electron> elPtr: electronCollection ){
	    elPtr->print(); std::cout << std::endl;
	    break; // only print first one for now
	}
	std::cout << std::endl;

	ElectronCollection scaleUpCollection = electronCollection.ElectronScaleUpCollection();
	std::cout << "scale up collection:" << std::endl;
        for( std::shared_ptr<Electron> elPtr: scaleUpCollection ){
            elPtr->print(); std::cout << std::endl;
	    break; // only print first one for now
        }
	std::cout << std::endl;

	ElectronCollection scaleDownCollection = electronCollection.ElectronScaleDownCollection();
        std::cout << "scale down collection:" << std::endl;
        for( std::shared_ptr<Electron> elPtr: scaleDownCollection ){
            elPtr->print(); std::cout << std::endl;
            break; // only print first one for now
        }
        std::cout << std::endl;

	ElectronCollection resUpCollection = electronCollection.ElectronResUpCollection();
        std::cout << "res up collection:" << std::endl;
        for( std::shared_ptr<Electron> elPtr: resUpCollection ){
            elPtr->print(); std::cout << std::endl;
            break; // only print first one for now
        }
        std::cout << std::endl;
	
	ElectronCollection resDownCollection = electronCollection.ElectronResDownCollection();
        std::cout << "res down collection:" << std::endl;
        for( std::shared_ptr<Electron> elPtr: resDownCollection ){
            elPtr->print(); std::cout << std::endl;
            break; // only print first one for now
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

    ElectronCollection_test( input_file_path, number_of_entries );
    return 0;
}
