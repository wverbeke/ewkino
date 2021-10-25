/*
Main executable to perform event flattening
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
#include "TMVA/Reader.h"

// include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Event/interface/Event.h"
#include "../../weights/interface/ConcreteReweighterFactory.h"

// include other parts of the analysis code
#include "../tools/interface/rootFileTools.h"
#include "interface/eventSelections.h"
#include "interface/eventFlattening.h"


void eventloopEventFlattening(
	    const std::string& pathToFile, 
	    const double norm, 
	    const std::string& outputDirectory,
	    const std::string& outputFileName,
	    const std::string& eventSelection,
	    const std::string& selectionType,
	    const std::string& variation,
	    const std::shared_ptr< TH2D>& frMap_muon = nullptr,
            const std::shared_ptr< TH2D>& frMap_electron = nullptr,
	    const bool doMVA = false,
	    const std::string& pathToXMLFile = "",
	    long nEvents = -1 ){

    // initialize TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );
    std::string year = "2016";
    if(treeReader.is2017()) year = "2017";
    if(treeReader.is2018()) year = "2018";

    // make output ROOT file
    std::string outputdir = "HWDAnalysis";
    std::string outputtree = "HWDTree";
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
    std::shared_ptr< TTree > outputTreePtr( std::make_shared< TTree >( 
	outputtree.c_str(), outputtree.c_str() ) );
    eventFlattening::initOutputTree(outputTreePtr.get());

    // initialize TMVA reader
    TMVA::Reader* reader = new TMVA::Reader();
    if(doMVA){ reader = eventFlattening::initializeReader(reader, pathToXMLFile); }

    // make reweighter
    std::shared_ptr< ReweighterFactory >reweighterFactory( new EmptyReweighterFactory() );
    // NOTE: use empty reweighter for now, change later!
    std::vector<Sample> thissample;
    thissample.push_back(treeReader.currentSample());
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( 
			"../../weights/", year, thissample );

    // do event loop
    long numberOfEntries = treeReader.numberOfEntries();
    if( nEvents<0 || nEvents>numberOfEntries ) nEvents = numberOfEntries;
    std::cout<<"starting event loop for "<<nEvents<<" events."<<std::endl;
    for(long entry = 0; entry < nEvents; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<nEvents<<std::endl;
	// build event and perform event selection
        Event event = treeReader.buildEvent(entry);
        if(!eventSelections::passEventSelection(event, eventSelection, 
		selectionType, variation, true)) continue;
	// set all high-level variables and make BDT output
        std::map< std::string,double> varmap;
	varmap = eventFlattening::eventToEntry( event, norm, reweighter, selectionType, 
	    frMap_muon, frMap_electron, variation, doMVA, reader);
        outputTreePtr->Fill();
    }
    outputFilePtr->cd( outputdir.c_str() );
    outputTreePtr->Write("", BIT(2) );
    outputFilePtr->Close();

    // delete reader
    delete reader;
}


int main( int argc, char* argv[] ){
    std::cerr<<"###starting###"<<std::endl;
    if( argc != 13  ){
        std::cerr << "ERROR: eventFlattener requires the following arguments: " << std::endl;
        std::cerr << "- input_file_path" << std::endl;
	std::cerr << "- norm" << std::endl;
	std::cerr << "- output_directory" << std::endl;
	std::cerr << "- output_file_name" << std::endl;
	std::cerr << "- event_selection" << std::endl;
	std::cerr << "- selection_type" << std::endl;
	std::cerr << "- variation" << std::endl;
	std::cerr << "- pathToMuonFakeRateMap" << std::endl;
	std::cerr << "- pathToElectronFakeRateMap" << std::endl;
	std::cerr << "- doMVA" << std::endl;
	std::cerr << "- pathToXMLFile" << std::endl;
	std::cerr << "- nevents" << std::endl;
        return -1;
    }
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    double norm = std::stod(argvStr[2]);
    std::string& output_directory = argvStr[3];
    std::string& output_file_name = argvStr[4];
    std::string& event_selection = argvStr[5];
    std::string& selection_type = argvStr[6];
    std::string& variation = argvStr[7];
    std::shared_ptr< TH2D > frMap_muon;
    std::shared_ptr< TH2D > frMap_electron;
    std::string year = "2016";
    if(stringTools::stringContains(input_file_path,"2017")) year = "2017";
    else if(stringTools::stringContains(input_file_path,"2018")) year = "2018";
    if(selection_type=="fakerate"){
	frMap_muon = fakeRateTools::readFRMap(argvStr[8],"muon",year);
	frMap_electron = fakeRateTools::readFRMap(argvStr[9],"electron",year);
    }
    bool doMVA = (argvStr[10]=="True" || argvStr[10]=="true");
    std::string pathToXMLFile = "";
    if(doMVA){ pathToXMLFile = argvStr[11]; }
    long nevents = std::stol(argvStr[12]);

    // check readability of input file
    bool validInput = rootFileTools::nTupleIsReadable( input_file_path );
    if(!validInput){ return -1; }
    
    // call event flattening function
    eventloopEventFlattening( input_file_path, norm, output_directory, output_file_name,
	    event_selection, selection_type, variation, frMap_muon, frMap_electron,
	    doMVA, pathToXMLFile, nevents );
    
    std::cerr<<"###done###"<<std::endl;
    return 0;
}
