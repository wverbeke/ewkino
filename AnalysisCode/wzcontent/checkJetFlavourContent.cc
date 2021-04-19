//include c++ library classes 
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include ROOT classes 
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"

// include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/systemTools.h"
#include "../../Event/interface/Event.h"
#include "../../Tools/interface/HistInfo.h"
#include "../eventselection/interface/eventSelections.h"

void fillJetFlavourContent(const std::string& pathToFile, 
			    const std::string& eventSelection,
			    int eventCat){
    
    // initialize TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );

    // intialize other global properties
    bool doCat = true;
    if( eventCat==0 ){ doCat=false; }

    // initialize output histograms
    std::vector<HistInfo> histInfoVec;
    histInfoVec.push_back( HistInfo("nbjets","",5,-0.5,4.5) );
    histInfoVec.push_back( HistInfo("ncjets","",5,-0.5,4.5) );
    histInfoVec.push_back( HistInfo("nbcjets","",5,-0.5,4.5) );
    histInfoVec.push_back( HistInfo("nlightjets","",7,-0.5,6.5) );
    histInfoVec.push_back( HistInfo("njets","",7,-0.5,6.5) );
    std::map< std::string, std::shared_ptr<TH1D> > histMap;
    for( HistInfo histInfo: histInfoVec ){
        histMap[histInfo.name()] = histInfo.makeHist( histInfo.name() );
    }

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    //long unsigned numberOfEntries = 10; // for testing
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
        Event event = treeReader.buildEvent(entry);
        if(!passES(event, eventSelection, "3prompt", "nominal")) continue;
	if(doCat){
	    if(eventCategory(event, "nominal")!=eventCat) continue;
	}
	JetCollection jetcollection = getjetcollection(event, "nominal");

	// determine jet flavour properties of this event
	int nBJets = 0;
	int nCJets = 0;
	int nLightJets = 0;
	for(JetCollection::const_iterator jIt = jetcollection.cbegin();
            jIt != jetcollection.cend(); jIt++){
            Jet& jet = **jIt;
	    if(jet.hadronFlavor()==5) nBJets++;
	    if(jet.hadronFlavor()==4) nCJets++;
	    else nLightJets++;
	}

	// fill output histograms
	histMap["nbjets"]->Fill( nBJets );
	histMap["ncjets"]->Fill( nCJets );
	histMap["nbcjets"]->Fill( nBJets+nCJets );
	histMap["nlightjets"]->Fill( nLightJets );
	histMap["njets"]->Fill( jetcollection.size() );
	
	// printouts for testing
	/*std::cout << "--- event ---" << std::endl;
	std::cout << " - number of b-jets: " << nBJets << std::endl;
	std::cout << " - number of c-jets: " << nCJets << std::endl;
	std::cout << " - number of light jets: " << nLightJets << std::endl;*/

	// make output file
	std::string outputDirectory = "output";
	systemTools::makeDirectory(outputDirectory);
	std::string outputFilePath = stringTools::formatDirectoryName(outputDirectory);
	outputFilePath += stringTools::removeOccurencesOf(pathToFile,"/");
	outputFilePath = stringTools::removeOccurencesOf(outputFilePath,".root");
	outputFilePath += "_" + eventSelection + "_" + std::to_string(eventCat) + ".root";
	TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
	for( auto el: histMap ) el.second->Write();
	outputFilePtr->Close();
    }
}

int main( int argc, char* argv[] ){
    
    std::cerr<<"###starting###"<<std::endl;
    if( argc != 4 ){
        std::cerr << "checkJetFlavourContent requires 3 arguments to run: " << std::endl;
        std::cerr << "input_file_path, event_selection, event_category" <<std::endl;
        return -1;
    }
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    std::string& event_selection = argvStr[2];
    int event_category = std::stoi(argvStr[3]);
    
    fillJetFlavourContent( input_file_path, event_selection, event_category );
    
    std::cerr<<"###done###"<<std::endl;
    return 0;
}
