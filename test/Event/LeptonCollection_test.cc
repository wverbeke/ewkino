
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
#include "../../Event/interface/MuonCollection.h"
#include "../../Event/interface/ElectronCollection.h"
#include "../../Event/interface/TauCollection.h"

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

        //test LeptonCollection
        LeptonCollection leptonCollection( treeReader );
        leptonCollection.selectLooseLeptons();
        leptonCollection.selectFOLeptons();
        leptonCollection.selectTightLeptons();

        leptonCollection.cleanElectronsFromLooseMuons();
        leptonCollection.cleanTausFromLooseLightLeptons();

        MuonCollection muonColletion = leptonCollection.muonCollection();
        ElectronCollection electronCollection = leptonCollection.electronCollection();
        TauCollection tauCollection = leptonCollection.tauCollection();

        leptonCollection.sortByPt();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time for looping over sample : " << elapsed.count() << " s\n";

    return 0;
}
