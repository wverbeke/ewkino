
//#include "../../weights/interface/LeptonReweighter.h"
//#include "../../weights/interface/ConcreteLeptonReweighter.h"
#include "../../weights/interface/ReweighterLeptons.h"
#include "../../weights/interface/ConcreteReweighterLeptons.h"

//include c++ library classes
#include <iostream>
#include <memory>

//include ROOT classes
#include "TFile.h"
#include "TH2D.h"
#include "TROOT.h"

//include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"
#include "../copyMoveTest.h"
#include "../../weights/interface/ConcreteSelection.h"


int main(){


    //Read muon scale factors
    TFile* muonSFFile = TFile::Open( "testSF/looseToTight_2016_m_3l.root" );
    std::shared_ptr< TH2 > muonWeights( dynamic_cast< TH2F* >( muonSFFile->Get( "EGamma_SF2D" ) ) );
    muonWeights->SetDirectory( gROOT );
    muonSFFile->Close();

    //Read electron scale factors
    TFile* eleSFFile = TFile::Open( "testSF/looseToTight_2016_e_3l.root" );
    std::shared_ptr< TH2 > electronWeights( dynamic_cast< TH2F* >( eleSFFile->Get( "EGamma_SF2D" ) ) );
    electronWeights->SetDirectory( gROOT );
    eleSFFile->Close(); 

    //make Reweighters, and check their behavior under copy/move etc
    MuonReweighter muonReweighter( muonWeights, new TightSelector );
    copyMoveTest( muonReweighter );
    ElectronIDReweighter electronReweighter( electronWeights, new TightSelector );
    copyMoveTest( electronReweighter );

    //loop over samples
    TreeReader treeReader;
    treeReader.readSamples("../testData/samples_test.txt", "../testData");

    ReweighterMuons reweighterMuons( muonReweighter );
    ReweighterElectronsID reweighterElectrons( electronReweighter );

    for( unsigned sampleIndex = 0; sampleIndex < treeReader.numberOfSamples(); ++sampleIndex ){

        //load next sample
        treeReader.initSample();

        //loop over events in sample
        for( long unsigned entry = 0; entry < treeReader.numberOfEntries();  ++entry ){
            Event event = treeReader.buildEvent( entry );

            ReweighterMuons.weight( event );
            ReweighterElectronsID.weight( event );
        }
    }

    return 0;
}
