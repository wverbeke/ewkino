
//include class to test 
#include "../../TreeReader/interface/TreeReader.h"
#include "../../objects/interface/PhysicsObject.h"
#include "../../objects/interface/Tau.h"

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

        std::vector< Tau > tauVector;
        for( unsigned l = treeReader._nLight; l < treeReader._nL; ++l ){
            tauVector.push_back( Tau( treeReader, l ) );
            tauVector.back() = tauVector.back();
			Tau& tau = tauVector.back();

			tau.isLightLepton();
        	tau.isMuon();
        	tau.isElectron();
        	tau.isTau();

        	tau.decayMode();
        	tau.passDecayModeFinding();
        	tau.passDecayModeFindingNew();

        	tau.passMuonVetoLoose();
        	tau.passMuonVetoTight();
        	tau.passEleVetoVLoose();
        	tau.passEleVetoLoose();
        	tau.passEleVetoMedium();
        	tau.passEleVetoTight();
        	tau.passEleVetoVTight();

        	tau.passVLooseMVAOld2015();
        	tau.passLooseMVAOld2015();
        	tau.passMediumMVAOld2015();
        	tau.passTightMVAOld2015();
        	tau.passVTightMVAOld2015();

        	tau.passVLooseMVANew2015();
        	tau.passLooseMVANew2015();
        	tau.passMediumMVANew2015();
        	tau.passTightMVANew2015();
        	tau.passVTightMVANew2015();

        	tau.passVVLooseMVAOld2017();
        	tau.passVLooseMVAOld2017();
        	tau.passLooseMVAOld2017();
        	tau.passMediumMVAOld2017();
        	tau.passTightMVAOld2017();
        	tau.passVTightMVAOld2017();
        	tau.passVVTightMVAOld2017();

        	tau.passVLooseMVANew2017();
        	tau.passLooseMVANew2017();
        	tau.passMediumMVANew2017();
        	tau.passTightMVANew2017();
        	tau.passVTightMVANew2017();
        }
        
        std::vector< Tau > tauVectorCopy;
        for( auto& t : tauVector ){
            tauVectorCopy.push_back( t );
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time for looping over sample : " << elapsed.count() << " s\n";

    return 0;
}
