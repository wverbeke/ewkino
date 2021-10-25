/*
Main executable to perform event selection
*/

// include c++ library classes 
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
#include "../../Event/interface/Event.h"

// include other parts of the analysis code
#include "../tools/interface/rootFileTools.h"
#include "interface/eventSelections.h"

void eventSelectionLoop( const std::string& pathToFile, 
	    const std::string& outputDirectory, const std::string& outputFileName, 
	    const std::string& eventSelection, const std::string& selectionType, 
	    const std::string& variation,
	    long nEvents ){
    
    // initialize TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );

    // make output ROOT file
    std::string outputdir = "blackJackAndHookers";
    std::string outputtree = "blackJackAndHookersTree";
    std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory );
    outputFilePath += outputFileName;
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    outputFilePtr->mkdir( outputdir.c_str() );
    outputFilePtr->cd( outputdir.c_str() );

    // read histograms from input file and write them to the new file
    std::vector< std::shared_ptr< TH1 > > histVector = treeReader.getHistogramsFromCurrentFile();
    for( const auto& histPtr : histVector ){
        histPtr->Write();
    }
    
    // make output tree
    std::shared_ptr<TTree> outputTreePtr( std::make_shared<TTree>(
	        outputtree.c_str(),outputtree.c_str()));
    treeReader.setOutputTree( outputTreePtr.get() );

    // do event loop
    long numberOfEntries = treeReader.numberOfEntries();
    if( nEvents<0 || nEvents>numberOfEntries ) nEvents = numberOfEntries;
    for(long entry = 0; entry < nEvents; entry++){
	if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<nEvents<<std::endl;
	Event event = treeReader.buildEvent(entry, false, false, false, false, true);
	if( ! eventSelections::passEventSelection( event, 
		eventSelection, selectionType, variation, true) ) continue;
	outputTreePtr->Fill();
    }
    outputTreePtr->Write("", BIT(2) );
    outputFilePtr->Close();
}


int main( int argc, char* argv[] ){

    std::cerr<<"###starting###"<<std::endl;
    if( argc != 8 ){
        std::cerr << "ERROR: eventSelector requires the following arguments: " << std::endl;
	std::cerr << "- input_file_path" << std::endl;
	std::cerr << "- output_directory" << std::endl;
	std::cerr << "- output_file_name" << std::endl;
	std::cerr << "- event_selection" << std::endl;
	std::cerr << "- selection_type" <<std::endl;
	std::cerr << "- variation" << std::endl;
	std::cerr << "- nevents" << std::endl;
	return -1;
    }

    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    std::string& output_directory = argvStr[2];
    std::string& output_file_name = argvStr[3];
    std::string& event_selection = argvStr[4];
    std::string& selection_type = argvStr[5];
    std::string& variation = argvStr[6];
    long nevents = std::stol(argvStr[7]);
   
    bool validInput = rootFileTools::nTupleIsReadable( input_file_path );
    if(!validInput){ return -1; }
 
    eventSelectionLoop( input_file_path, 
			output_directory, output_file_name, 
			event_selection, selection_type, 
			variation,
			nevents );
    std::cerr<<"###done###"<<std::endl;
    return 0;
}
