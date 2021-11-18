/*
Main executable to perform studies of different higgs boson reconstruction techniques
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
#include "../tools/interface/variableTools.h"
#include "../eventselection/interface/eventSelections.h"
#include "interface/higgsReco.h"


void fillHistograms( const PhysicsObject& higgsCandidate,
		     std::map<std::string,std::shared_ptr<TH1D>>& histograms,
		     double weight ){
    std::map<std::string,std::shared_ptr<TH1D>>::iterator it;
    double varvalue = 0;
    for( it=histograms.begin(); it!=histograms.end(); ++it ){
        if( it->first == "hmass" ) varvalue = higgsCandidate.mass();
	else{
            std::string msg = "ERROR: unrecognized variable in histogram map: ";
            msg += it->first;
            throw std::runtime_error( msg );
        }
        std::shared_ptr<TH1D> hist = it->second;
        int nbins = hist->GetNbinsX();
        double xlow = hist->GetBinLowEdge(0);
        double xhigh = hist->GetBinLowEdge(nbins)+hist->GetBinWidth(nbins);
        if( varvalue > xhigh ){
            varvalue = xhigh - hist->GetBinWidth(nbins)/2.; }
        if( varvalue < xlow ){
            varvalue = xlow + hist->GetBinWidth(0)/2.; }
        it->second->Fill( varvalue, weight );
    }
}


void fillHiggsReco(
	    const std::string& pathToFile, 
	    long nEvents,
	    const std::string& eventSelection,
	    std::map<std::string,std::map<std::string,std::shared_ptr<TH1D>>> hists ){

    // hard-coded arguments
    std::string selectionType = "tight";
    std::string variation = "nominal";
    bool selectbjets = true;

    // initialize TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );

    // make reweighter
    // copied from eventFlattener.cc but probably not needed for these studies
    /*std::shared_ptr< ReweighterFactory >reweighterFactory( new EmptyReweighterFactory() );
    // NOTE: use empty reweighter for now, change later!
    std::vector<Sample> thissample;
    thissample.push_back(treeReader.currentSample());
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( 
			"../../weights/", year, thissample );*/

    // do event loop
    long numberOfEntries = treeReader.numberOfEntries();
    if( nEvents<0 || nEvents>numberOfEntries ) nEvents = numberOfEntries;
    std::cout<<"starting event loop for "<<nEvents<<" events."<<std::endl;
    for(long entry = 0; entry < nEvents; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<nEvents<<std::endl;
	
	// build event and perform event selection
        Event event = treeReader.buildEvent(entry, false, false, false, false, true);
        if(!eventSelections::passEventSelection(event, eventSelection, 
		selectionType, variation, selectbjets)) continue;
	
	// set weight
	double weight = 1.; // maybe extend later
 
	// do higgs boson reconstruction and fill histograms
	for( auto& [method, thishists]: hists ){
	    std::pair<PhysicsObject,PhysicsObject> temp;
	    temp = higgsReco::genericHiggsReco( event, method );
	    PhysicsObject hCandidate = temp.first;
	    PhysicsObject nuCandidate = temp.second;
	    fillHistograms( hCandidate, thishists, weight ); 
	}
    }
}


int main( int argc, char* argv[] ){
    std::cerr<<"###starting###"<<std::endl;
    if( argc != 6  ){
        std::cerr << "ERROR: fillHiggsReco requires 5 arguments to run: " << std::endl;
        std::cerr << "- input file" << std::endl;
	std::cerr << "- output file" << std::endl;
	std::cerr << "- variable file" << std::endl;
	std::cerr << "- event selection" << std::endl;
	std::cerr << "- number of events" << std::endl;
        return -1;
    }
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    std::string& output_file_path = argvStr[2];
    std::string& variable_file_path = argvStr[3];
    std::string& event_selection = argvStr[4];
    long nprocess = std::stol(argvStr[5]);

    // initialize the histograms
    std::vector<HistogramVariable> vars = variableTools::readVariables( variable_file_path );
    std::map<std::string,std::map<std::string, std::shared_ptr<TH1D>>> hists;
    std::vector<std::string> methods = higgsReco::genericHiggsRecoMethods();
    for( std::string method: methods ){
	std::map<std::string, std::shared_ptr<TH1D>> thishists;
	thishists = variableTools::initializeHistograms( vars );
	hists[method] = thishists;
    }

    // check readability of input file
    bool validInput = rootFileTools::nTupleIsReadable( input_file_path );
    if(!validInput){ return -1; }
    
    // call higgs reconstruction function
    fillHiggsReco( input_file_path, nprocess,
		    event_selection, hists );

    // write histograms to output file
    TFile* outputFilePtr = TFile::Open( output_file_path.c_str() , "RECREATE" );
    for( std::string method: methods ){
	std::map<std::string,std::shared_ptr<TH1D>>::iterator it;
	std::map<std::string,std::shared_ptr<TH1D>> thishists = hists[method];
	for( it=thishists.begin(); it!=thishists.end(); ++it ){
	    it->second->SetName( (std::string(it->second->GetName())+"_"+method).c_str() );
	    it->second->Write();
	}
    }
    outputFilePtr->Close();
    
    std::cerr<<"###done###"<<std::endl;
    return 0;
}
