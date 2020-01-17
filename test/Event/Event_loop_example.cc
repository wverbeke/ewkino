
//include class to test 
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"

int main(){
	// arguments to TreeReader constructor:
	// - sample list (see TreeReader constructor for proper formatting)
	// - folder where to find the files mentioned in the sample list
	TreeReader treeReader("../testData/testsamplelist.txt", "../testData");
	for( unsigned sampleIndex = 0; sampleIndex < treeReader.numberOfSamples(); ++sampleIndex ){

	    //load next sample
		treeReader.initSample();

		//loop over events in sample
		for( long unsigned entry = 0; entry < treeReader.numberOfEntries(); ++entry ){
            
            if(entry%100==0){
                std::cout<<"processing event "<<entry+1<<" of "<<treeReader.numberOfEntries()<<std::endl;
			}
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
        std::cout<<"event loop finished for sample "<<sampleIndex+1<<" of "<<treeReader.numberOfSamples()<<std::endl;
	}
	return 0;
}
