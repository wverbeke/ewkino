
//include class to test 
#include "../../TreeReader/interface/TreeReader.h"
#include "../../objects/interface/PhysicsObject.h"
#include "../../objects/interface/Lepton.h"
#include "../../objects/interface/LightLepton.h"
#include "../../objects/interface/Muon.h"
#include "../../objects/interface/Electron.h"
#include "../../objects/interface/Tau.h"
#include "../../Event/interface/LeptonCollection.h"
#include "../../Event/interface/JetCollection.h"

//include c++ library classes
#include <iostream>
#include <chrono>
#include <random>
#include <utility>
#include <vector>

//include ROOT TLorentzVector for performance comparison
#include "TLorentzVector.h"


int main(){
    TreeReader treeReader;
    treeReader.readSamples("../testData/samples_test.txt", "../testData");

    treeReader.initSample();

    auto start = std::chrono::high_resolution_clock::now();

    for(unsigned i = 0; i < treeReader.nEntries; ++i){
        std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;

        //test LeptonCollection
        treeReader.GetEntry(i);
        LeptonCollection leptonCollection( treeReader );
        leptonCollection.sortByPt();
        for(auto& lepton : leptonCollection ){
            std::cout << lepton->pt() << "\t";
            std::cout << ( lepton->isTau() ? "Tau" : "" );
            std::cout << ( lepton->isMuon() ? "Muon" : "" );
            std::cout << ( lepton->isElectron() ? "Electron" : "" );
            std::cout << std::endl;
        }

        //test JetCollection
        treeReader.GetEntry(i);
        JetCollection jetCollection( treeReader );
        for( auto& jet : jetCollection ){
            std::cout << "jet:\t" << jet->pt() << "\t" << jet->deepCSV() << std::endl;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time for looping over sample : " << elapsed.count() << " s\n";

    return 0;
}
