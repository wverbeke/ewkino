#include "../../weights/src/ReweighterPileup.cc"

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


int main(){

    //loop over samples
    TreeReader treeReader;
    treeReader.readSamples("../testData/samples_test.txt", "../testData");

    //make pileup Reweighter
    ReweighterPileup reweighterPileup( treeReader.sampleVector(), "../../weights/" );

    for( unsigned sampleIndex = 0; sampleIndex < treeReader.numberOfSamples(); ++sampleIndex ){

        //load next sample
        treeReader.initSample();

        //loop over events in sample
        for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){
            Event event = treeReader.buildEvent( entry );
            reweighterPileup.weight( event);
            reweighterPileup.weightDown( event);
            reweighterPileup.weightUp( event);
        }
    }

    return 0;
}
