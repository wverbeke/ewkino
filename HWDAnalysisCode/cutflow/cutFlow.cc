/*
Perform cutflow study
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
#include "../eventselection/interface/eventSelections.h"

std::shared_ptr<TH1D> makeCutFlowHistogram( const std::string& pathToFile,
			const std::string& eventSelection, 
			const std::string& selectionType, 
			const std::string& variation,
			long nEvents,
			int maxCutFlowValue ){
    // make a TH1D containing the output of cutflow event selection.
    // the output of the cutflow eventselection function is an integer number
    // (usually the number of cuts passed, but see eventselection/src/eventSelections for details)
    
    // initialize TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );
    
    // initialize output histogram
    std::shared_ptr<TH1D> cutFlowHist = std::make_shared<TH1D>( 
	"cutFlowHist", "cutFlowHist;cutflow value;number of events", 
	maxCutFlowValue+2, -1.5, maxCutFlowValue+0.5);
    // set the bin labels to empty strings
    for( int i=1; i<cutFlowHist->GetNbinsX()+1; ++i){
	cutFlowHist->GetXaxis()->SetBinLabel(i, "");
    }

    // do event loop
    long numberOfEntries = treeReader.numberOfEntries();
    if( nEvents<0 || nEvents>numberOfEntries ) nEvents = numberOfEntries;
    for(long entry = 0; entry < nEvents; entry++){
	if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<nEvents<<std::endl;
	Event event = treeReader.buildEvent(entry, false, false, false, false, true);
	std::tuple<int,std::string> cutFlowTuple = eventSelections::passCutFlow( event, 
			    eventSelection, selectionType, variation, true );
	int cutFlowValue = std::get<0>(cutFlowTuple);
	std::string cutFlowDescription = std::get<1>(cutFlowTuple);
	if( cutFlowValue>maxCutFlowValue ){
	    std::string msg = "WARNING: cutFlowValue is "+std::to_string(cutFlowValue);
	    msg += " while maximum was set to "+std::to_string(maxCutFlowValue);
	    msg += "; please run again with larger maxCutFlowValue.";
	    throw std::runtime_error(msg);
	}
	// determine corresponding bin number
	int binnb = cutFlowValue+2;
	// set the bin label if not done so before
	if( std::string(cutFlowHist->GetXaxis()->GetBinLabel(binnb))==std::string("") ){
	    cutFlowHist->GetXaxis()->SetBinLabel(binnb, cutFlowDescription.c_str());
	}
	// fill the histogram
	cutFlowHist->Fill(cutFlowValue);
    }
    return cutFlowHist;
}


int main( int argc, char* argv[] ){

    std::cerr<<"###starting###"<<std::endl;
    if( argc != 8 ){
        std::cerr << "ERROR: eventSelector requires the following arguments: " << std::endl;
	std::cerr << "- input_file_path" << std::endl;
	std::cerr << "- output_file_path" << std::endl;
	std::cerr << "- event_selection" << std::endl;
	std::cerr << "- selection_type" <<std::endl;
	std::cerr << "- variation" << std::endl;
	std::cerr << "- nevents" << std::endl;
	std::cerr << "- max_cutflow_value" << std::endl;
	return -1;
    }

    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    std::string& output_file_path = argvStr[2];
    std::string& event_selection = argvStr[3];
    std::string& selection_type = argvStr[4];
    std::string& variation = argvStr[5];
    long nevents = std::stol(argvStr[6]);
    int max_cutflow_value = std::stoi(argvStr[7]);
   
    bool validInput = rootFileTools::nTupleIsReadable( input_file_path );
    if(!validInput){ return -1; }
 
    // make the cutflow histogram
    std::shared_ptr<TH1D> cutFlowHist = makeCutFlowHistogram( 
			input_file_path, event_selection, 
			selection_type, variation,
			nevents, max_cutflow_value );

    // write to output file
    TFile* outputFilePtr = TFile::Open( output_file_path.c_str() , "RECREATE" );
    cutFlowHist->Write();
    outputFilePtr->Close();

    std::cerr<<"###done###"<<std::endl;
    return 0;
}
