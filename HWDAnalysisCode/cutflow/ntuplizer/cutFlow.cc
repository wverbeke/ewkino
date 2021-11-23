/*
Perform cutflow study (on ntuplizer level)
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
//#include "../../TreeReader/interface/TreeReader.h"
//#include "../../Tools/interface/stringTools.h"
//#include "../../Event/interface/Event.h"

// include other parts of the analysis code
#include "../../tools/interface/rootFileTools.h"

std::shared_ptr<TH1D> makeCutFlowHistogram( const std::string& pathToFile,
			const std::string& cutFlowVarName,
			long nEvents,
			unsigned maxCutFlowValue ){
    // make a TH1D containing a cutflow variable as defined in the ntuplizer.
    
    // experimental to avoid the need for a TreeReader
    TBranch *b__cutFlowVar;
    UInt_t _cutFlowVar;
    std::shared_ptr< TFile > _currentFilePtr;
    _currentFilePtr = std::shared_ptr< TFile >( new TFile( pathToFile.c_str() ) );
    TTree* _currentTreePtr;
    _currentTreePtr = (TTree*) _currentFilePtr->Get( "blackJackAndHookers/blackJackAndHookersTree" );
    _currentTreePtr->SetBranchAddress(cutFlowVarName.c_str(), &_cutFlowVar, &b__cutFlowVar); 

    // initialize TreeReader
    //TreeReader treeReader;
    //treeReader.initSampleFromFile( pathToFile );
    
    // initialize output histogram
    std::shared_ptr<TH1D> cutFlowHist = std::make_shared<TH1D>( 
	"cutFlowHist", "cutFlowHist;cutflow value;number of events", 
	maxCutFlowValue+1, -0.5, maxCutFlowValue+0.5);
    cutFlowHist->SetDirectory(0);
    // set the bin labels to empty strings
    for( int i=1; i<cutFlowHist->GetNbinsX()+1; ++i){
	cutFlowHist->GetXaxis()->SetBinLabel(i, "");
    }

    // experimental to avoid the need for a TreeReader
    long numberOfEntries = _currentTreePtr->GetEntries();
    if( nEvents<0 || nEvents>numberOfEntries ) nEvents = numberOfEntries;
    for(long entry = 0; entry < nEvents; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<nEvents<<std::endl;
	_currentTreePtr->GetEntry(entry);
        unsigned cutFlowValue = _cutFlowVar;
        if( cutFlowValue>maxCutFlowValue ){
            std::string msg = "WARNING: cutFlowValue is "+std::to_string(cutFlowValue);
            msg += " while maximum was set to "+std::to_string(maxCutFlowValue);
            msg += "; please run again with larger maxCutFlowValue.";
            throw std::runtime_error(msg);
        }
        // fill the histogram
        cutFlowHist->Fill(cutFlowValue);
    }
   

    // do event loop
    /*long numberOfEntries = treeReader.numberOfEntries();
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
	int binnb = cutFlowValue+1;
	// set the bin label if not done so before
	if( std::string(cutFlowHist->GetXaxis()->GetBinLabel(binnb))==std::string("") ){
	    cutFlowHist->GetXaxis()->SetBinLabel(binnb, cutFlowDescription.c_str());
	}
	// fill the histogram
	cutFlowHist->Fill(cutFlowValue);
    }*/

    _currentFilePtr->Close();
    return cutFlowHist;
}


int main( int argc, char* argv[] ){

    std::cerr<<"###starting###"<<std::endl;
    if( argc != 6 ){
        std::cerr << "ERROR: cutFlow requires the following arguments: " << std::endl;
	std::cerr << "- input file path" << std::endl;
	std::cerr << "- cutflow variable name" << std::endl;
	std::cerr << "- output file path" << std::endl;
	std::cerr << "- number of events to process" << std::endl;
	std::cerr << "- max cutflow value" << std::endl;
	return -1;
    }

    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    std::string& cutflow_var_name = argvStr[2];
    std::string& output_file_path = argvStr[3];
    long nevents = std::stol(argvStr[4]);
    unsigned max_cutflow_value = (unsigned) std::stoi(argvStr[5]);
   
    bool validInput = rootFileTools::nTupleIsReadable( input_file_path );
    if(!validInput){ return -1; }
 
    // make the cutflow histogram
    std::shared_ptr<TH1D> cutFlowHist = makeCutFlowHistogram( 
			input_file_path, cutflow_var_name,
			nevents, max_cutflow_value );

    // write to output file
    TFile* outputFilePtr = TFile::Open( output_file_path.c_str() , "RECREATE" );
    cutFlowHist->Write();
    outputFilePtr->Close();

    std::cerr<<"###done###"<<std::endl;
    return 0;
}
