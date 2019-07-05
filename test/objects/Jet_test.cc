
//include class to test 
#include "../../TreeReader/interface/TreeReader.h"
#include "../../objects/interface/PhysicsObject.h"
//#include "../../objects/interface/Lepton.h"
//#include "../../objects/interface/LightLepton.h"
//#include "../../objects/interface/Muon.h"
//#include "../../objects/interface/Electron.h"
//#include "../../objects/interface/Tau.h"
#include "../../objects/interface/Jet.h"

//include c++ library classes
#include <iostream>
#include <chrono>
#include <utility>
#include <vector>

int main(){
    TreeReader treeReader;
    treeReader.readSamples("../testData/samples_test.txt", "../testData");

    treeReader.initSample();

    auto start = std::chrono::high_resolution_clock::now();

    for(unsigned i = 0; i < treeReader.nEntries; ++i){

        treeReader.GetEntry(i);

        std::vector< Jet > jetVector;
        for( unsigned j = 0; j < treeReader._nJets; ++j ){
            jetVector.push_back( Jet( treeReader, j ) );
            jetVector[j] = jetVector[j];
        }
        
        std::vector< Jet > jetVectorCopy;
        for( auto& j : jetVector ){
            jetVectorCopy.push_back( j );
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time for looping over sample : " << elapsed.count() << " s\n";

    return 0;
}
