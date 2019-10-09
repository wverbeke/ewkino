
//include class to test 
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"

int main(){
	TreeReader treeReader("../testData/samples_test.txt", "../testData");
	for( unsigned sampleIndex = 0; sampleIndex < treeReader.numberOfSamples(); ++sampleIndex ){

		//load next sample
		treeReader.initSample();

		//loop over events in sample
		for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){

			//build next event
			Event event = treeReader.buildEvent( entry ); 

            //clean electrons from muon overlap
            event.cleanElectronsFromLooseMuons();

            //clean jets from lepton overlap
            event.cleanJetsFromFOLeptons();

            //select tight leptons
            event.selectTightLeptons();

            //require 3 leptons 
            if( event.numberOfLeptons() != 3 ) continue;

            //require 2 medium b -jets 
            if( event.numberOfMediumBTaggedJets() != 2 ) continue;

            //....fill histograms....
            
		}
	}
	return 0;
}
