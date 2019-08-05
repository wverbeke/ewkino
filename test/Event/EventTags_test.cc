
//include class to test 
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/EventTags.h"
#include "../copyMoveTest.h"

//include c++ library classes
#include <iostream>
#include <chrono>


int main(){
    TreeReader treeReader;
    treeReader.readSamples("../testData/samples_test.txt", "../testData");

    treeReader.initSample();

    auto start = std::chrono::high_resolution_clock::now();

    for(unsigned i = 0; i < treeReader.nEntries; ++i){

        treeReader.GetEntry(i);
        EventTags eventTags( treeReader );
        eventTags.runNumber();
        eventTags.luminosityBlock();
        eventTags.eventNumber();

        copyMoveTest( eventTags );
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time for looping over sample : " << elapsed.count() << " s\n";

    return 0;
}
