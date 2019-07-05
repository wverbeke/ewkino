
//include class to test 
#include "../../TreeReader/interface/TreeReader.h"
#include "../../objects/interface/PhysicsObject.h"
#include "../../objects/interface/Electron.h"

//include c++ library classes
#include <iostream>
#include <chrono>
//#include <utility>
#include <vector>

int main(){
    TreeReader treeReader;
    treeReader.readSamples("../testData/samples_test.txt", "../testData");

    treeReader.initSample();

    auto start = std::chrono::high_resolution_clock::now();

    for(unsigned i = 0; i < treeReader.nEntries; ++i){

        treeReader.GetEntry(i);

        std::vector< Electron > electronVector;
        for( unsigned l = treeReader._nMu; l < treeReader._nLight; ++l ){
            electronVector.push_back( Electron( treeReader, l ) );
            electronVector.back() = electronVector.back();
        }
        
        std::vector< Electron > electronVectorCopy;
        for( auto& e : electronVector ){
            electronVectorCopy.push_back( e );
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time for looping over sample : " << elapsed.count() << " s\n";

    return 0;
}
