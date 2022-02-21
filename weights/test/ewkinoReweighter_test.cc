#include "../interface/CombinedReweighter.h"
#include "../interface/ConcreteReweighterFactory.h"

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
#include "../../test/copyMoveTest.h"


int main(){

    //loop over samples
    TreeReader treeReader;
    treeReader.readSamples("../testData/samples_test.txt", "../testData");

    std::shared_ptr< ReweighterFactory >reweighterFactory( new EwkinoReweighterFactory() );
    CombinedReweighter reweighter_2016 = reweighterFactory->buildReweighter( 
	"../", "2016", treeReader.sampleVector() );
    CombinedReweighter reweighter_2017 = reweighterFactory->buildReweighter( 
	"../", "2017", treeReader.sampleVector() );
    CombinedReweighter reweighter_2018 = reweighterFactory->buildReweighter( 
	"../", "2018", treeReader.sampleVector() );

    for( unsigned sampleIndex = 0; sampleIndex < treeReader.numberOfSamples(); ++sampleIndex ){

        //load next sample
        treeReader.initSample();

        //loop over events in sample
        for( long unsigned entry = 0; entry < treeReader.numberOfEntries();  ++entry ){
            Event event = treeReader.buildEvent( entry );

            if( event.is2016() ){
                std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
                std::cout << reweighter_2016.totalWeight( event ) << std::endl;
                std::cout << "reweighter_2016[ muonID ].weight( event ) = ";
		std::cout << reweighter_2016[ "muonID" ]->weight( event ) << std::endl;
                std::cout << "reweighter_2016[ electronID ].weight( event ) = ";
		std::cout << reweighter_2016[ "electronID" ]->weight( event ) << std::endl;
                std::cout << "reweighter_2016[ electronReco_pTBelow20 ].weight( event ) = ";
		std::cout << reweighter_2016[ "electronReco_pTBelow20" ]->weight( event ) << std::endl;
                std::cout << "reweighter_2016[ electronReco_pTAbove20 ].weight( event ) = ";
		std::cout << reweighter_2016[ "electronReco_pTAbove20" ]->weight( event ) << std::endl;
                std::cout << "reweighter_2016[ pileup ].weight( event ) = ";
		std::cout << reweighter_2016[ "pileup" ]->weight( event ) << std::endl;
                std::cout << "reweighter_2016[ prefire ].weight( event ) = ";
		std::cout << reweighter_2016[ "prefire" ]->weight( event ) << std::endl;
                std::cout << "reweighter_2016[ bTag ].weight( event ) = ";
		std::cout << reweighter_2016[ "bTag" ]->weight( event ) << std::endl;
            } 
        }
    }

    return 0;
}
