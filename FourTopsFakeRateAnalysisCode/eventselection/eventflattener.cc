// This is the main C++ class used to perform event flattening.
// It is supposed to run on the output file of an event selection procedure
// and produce a flat tree containing relevant variables.

// inlcude c++ library classes
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
#include "interface/eventSelections.h"
#include "interface/eventFlattening.h"

void eventloopEF_CR(const std::string& inputDirectory,
		    const std::string& sampleList,
		    int sampleIndex,
		    const std::string& outputDirectory,
                    const std::string& eventselection,
		    const std::string& selectiontype,
		    const std::string& variation,
		    const std::string& muonFRMap,
		    const std::string& electronFRMap){

    std::cout << "start function eventloopEF_CR" << std::endl;
    
    // initialize TreeReader
    std::cout << "initialize TreeReader for sample at index " << sampleIndex << "." << std::endl;
    TreeReader treeReader( sampleList, inputDirectory, true );
    treeReader.initSample();
    for(int idx=1; idx<=sampleIndex; ++idx){ treeReader.initSample(); }
    std::string year = treeReader.getYearString();
    std::string inputFileName = treeReader.currentSample().fileName();

    // make output file
    std::string outputdir = "blackJackAndHookers";
    std::string treename = "blackJackAndHookersTree";
    std::string outputFilePath = stringTools::formatDirectoryName( outputDirectory );
    outputFilePath += inputFileName;
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    outputFilePtr->mkdir( outputdir.c_str() );
    outputFilePtr->cd( outputdir.c_str() );

    // copy histograms from input file to output file
    std::vector< std::shared_ptr< TH1 > > histVector = treeReader.getHistogramsFromCurrentFile();
    for( const auto& histPtr : histVector ){
        histPtr->Write();
    }

    // make output Tree
    std::shared_ptr< TTree > treePtr( std::make_shared< TTree >( 
					    treename.c_str(), treename.c_str() ) );
    eventFlattening::initOutputTree(treePtr.get());

    // make reweighter
    std::string reweighterYear = year;
    if( year=="2016PreVFP" || year=="2016PostVFP" ) reweighterYear = "2016";
    std::shared_ptr< ReweighterFactory >reweighterFactory( new EmptyReweighterFactory() );
    std::vector<Sample> thissample;
    thissample.push_back(treeReader.currentSample());
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( 
                                      "../../weights/", reweighterYear, thissample );

    // read fake rate maps if needed
    std::shared_ptr< TH2D > frMap_muon;
    std::shared_ptr< TH2D > frMap_electron;
    if(selectiontype=="fakerate"){
	frMap_muon = eventFlattening::readFRMap(muonFRMap,"muon",year);
	frMap_electron = eventFlattening::readFRMap(electronFRMap,"electron",year);
    }
    
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    //unsigned numberOfEntries = 5000;
    std::cout<<"starting event loop for "<<numberOfEntries<<" events"<<std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
        Event event = treeReader.buildEvent(entry);
        if(!passES(event, eventselection, selectiontype, variation)) continue;
        eventFlattening::eventToEntry(event, reweighter, selectiontype, 
				      frMap_muon, frMap_electron, variation);
        treePtr->Fill();
    }
    outputFilePtr->cd( outputdir.c_str() );
    treePtr->Write("", BIT(2) );
    outputFilePtr->Close();
} 

int main( int argc, char* argv[] ){
    std::cerr<<"###starting###"<<std::endl;
    if( argc != 10  ){
        std::cerr << "ERROR: event flattening requires different number of arguments:";
        std::cerr << " input_directory, sample_list, sample_index,";
	std::cerr << " output_directory,";
	std::cerr << " event_selection, selection_type, variation,";
	std::cerr << " muonfrmap, electronfrmap" << std::endl;
        return -1;
    }
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    // necessary arguments:
    std::string& input_directory = argvStr[1];
    std::string& sample_list = argvStr[2];
    int sample_index = std::stoi(argvStr[3]);
    std::string& output_directory = argvStr[4];
    std::string& event_selection = argvStr[5];
    // type of selection and variation to consider:
    std::string& selection_type = argvStr[6];
    std::string& variation = argvStr[7];
    // first set of other arguments: 
    std::string& muonfrmap = argvStr[8];
    std::string& electronfrmap = argvStr[9];
    
    // call functions
    eventloopEF_CR( input_directory, sample_list, sample_index, output_directory, 
			event_selection, selection_type, variation, muonfrmap, electronfrmap );
    std::cerr<<"###done###"<<std::endl;
    return 0;
}
