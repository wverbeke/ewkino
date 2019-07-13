
//include class to test 
#include "../../TreeReader/interface/TreeReader.h"
#include "../../objects/interface/PhysicsObject.h"
#include "../../objects/interface/Muon.h"

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

        std::vector< Muon > muonVector;
        for( unsigned l = 0; l < treeReader._nMu; ++l ){
            muonVector.push_back( Muon( treeReader, l ) );
            muonVector.back() = muonVector.back();
            Muon& muon = muonVector.back();
    
            muon.isMuon();
            muon.isElectron();

            muon.isLoosePOGMuon();
            muon.isMediumPOGMuon();
            muon.isTightPOGMuon();

            muon.segmentCompatibility();
            muon.trackPt();
            muon.trackPtError();
        }
        
        std::vector< Muon > muonVectorCopy;
        for( auto& m : muonVector ){
            muonVectorCopy.push_back( m );
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time for looping over sample : " << elapsed.count() << " s\n";

    return 0;
}
