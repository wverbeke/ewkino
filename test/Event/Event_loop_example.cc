
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
		}
	}
	return 0;
}
