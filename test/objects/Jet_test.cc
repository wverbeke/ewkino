
//include class to test 
#include "../../TreeReader/interface/TreeReader.h"
#include "../../objects/interface/PhysicsObject.h"
#include "../../objects/interface/Jet.h"
#include "../copyMoveTest.h"

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
            jetVector.back() = jetVector.back();

            Jet& jet = jetVector.back();
            jet.isGood();
            jet.isBTaggedLoose();
            jet.isBTaggedMedium();
            jet.isBTaggedTight();
            
            copyMoveTest( jet );
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time for looping over sample : " << elapsed.count() << " s\n";

    return 0;
}
