/*
Read Ds mesons from an ntuple and fill histograms of gen-matched vs non-gen-matched mesons.
*/

// include c++ library classes 
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include ROOT classes 
#include "TFile.h"
#include "TTree.h"

// include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Event/interface/Event.h"

// include other parts of the analysis code
#include "../tools/interface/rootFileTools.h"
#include "../tools/interface/variableTools.h"

void fillHistograms( const std::shared_ptr<DMeson> dmeson, 
		     std::map<std::string,std::shared_ptr<TH1D>>& histograms,
		     double weight ){
    std::map<std::string,std::shared_ptr<TH1D>>::iterator it;
    double varvalue = 0;
    for( it=histograms.begin(); it!=histograms.end(); ++it ){
	if( it->first == "invMass" ) varvalue = dmeson->invMass();
	else if( it->first == "pt" ) varvalue = dmeson->pt();
	else if( it->first == "isolation" ) varvalue = dmeson->isolation();
	else if( it->first == "dR" ) varvalue = dmeson->dR();
	else if( it->first == "intResMass" ) varvalue = dmeson->intResMass();
	else if( it->first == "intResMassDiff" ) varvalue = dmeson->intResMassDiff();
	else if( it->first == "intResVtxNormChi2" ) varvalue = dmeson->intResVtxNormChi2();
	else if( it->first == "firstTrackPt" ) varvalue = dmeson->firstTrackPt();
	else if( it->first == "secondTrackPt" ) varvalue = dmeson->secondTrackPt();
	else if( it->first == "thirdTrackPt" ) varvalue = dmeson->thirdTrackPt();
	else if( it->first == "twotrackSpatialSeparationX" ){ 
	    varvalue = dmeson->twotrackSpatialSeparationX(); }
	else if( it->first == "twotrackSpatialSeparationY" ){ 
	    varvalue = dmeson->twotrackSpatialSeparationY(); }
	else if( it->first == "twotrackSpatialSeparationZ" ){ 
            varvalue = dmeson->twotrackSpatialSeparationZ(); }
	else if( it->first == "restrackSpatialSeparationX" ){ 
            varvalue = dmeson->restrackSpatialSeparationX(); }
	else if( it->first == "restrackSpatialSeparationY" ){
            varvalue = dmeson->restrackSpatialSeparationY(); }
	else if( it->first == "restrackSpatialSeparationZ" ){
            varvalue = dmeson->restrackSpatialSeparationZ(); }
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

void fillDsGenMatchComparison( const std::string& pathToFile, long nEvents,
				std::map<std::string,std::shared_ptr<TH1D>> genMatchHists,
				std::map<std::string,std::shared_ptr<TH1D>> nonGenMatchHists,
				bool selectGoodDMesons ){
    
    // initialize TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );

    // do event loop
    long numberOfEntries = treeReader.numberOfEntries();
    if( nEvents<0 || nEvents>numberOfEntries ) nEvents = numberOfEntries;
    for(long entry = 0; entry < nEvents; entry++){
	if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<nEvents<<std::endl;
	Event event = treeReader.buildEvent(entry, false, false, false, false, true);

	// get the D meson collection
	DMesonCollection dmesonCollection = event.dmesonCollection();

	// select good D mesons if required
	if( selectGoodDMesons ) dmesonCollection.selectGoodDMesons();

	// loop over D mesons
	for( std::shared_ptr<DMeson> dmeson: dmesonCollection ){

	    // set the weight
	    double weight = 1.;

	    // determine if the D meson is gen matched
	    bool isGenMatched = dmeson->hasFastGenMatch();

	    // fill the appropriate histograms
	    if(isGenMatched) fillHistograms(dmeson, genMatchHists, weight);
	    else fillHistograms(dmeson, nonGenMatchHists, weight);
	}	
    }
}

int main( int argc, char* argv[] ){

    std::cerr<<"###starting###"<<std::endl;
    if( argc != 6 ){
        std::cerr << "ERROR: dsGenMatchComparison_fill requires 5 arguments to run: " << std::endl;
	std::cerr << "- input file" << std::endl;
	std::cerr << "- output file" << std::endl;
	std::cerr << "- variable file" << std::endl;
	std::cerr << "- number of events" << std::endl;
	std::cerr << "- do Ds selection" << std::endl;
        return -1;
    }
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    std::string& output_file_path = argvStr[2];
    std::string& variable_file_path = argvStr[3];
    long nprocess = std::stol(argvStr[4]);
    bool selectGoodDMesons = (argvStr[5]=="true" or argvStr[5]=="True");
    bool validInput = rootFileTools::nTupleIsReadable( input_file_path );
    if(!validInput){ return -1; }
    
    // initialize the histograms
    std::vector<HistogramVariable> vars = variableTools::readVariables( variable_file_path );
    std::map< std::string, std::shared_ptr<TH1D> > genMatchHists;
    std::map< std::string, std::shared_ptr<TH1D> > nonGenMatchHists;
    genMatchHists = variableTools::initializeHistograms( vars );
    nonGenMatchHists = variableTools::initializeHistograms( vars );
    
    // fill the histograms
    fillDsGenMatchComparison( input_file_path, nprocess,
                                genMatchHists,
                                nonGenMatchHists,
                                selectGoodDMesons );

    // write histograms to output file
    TFile* outputFilePtr = TFile::Open( output_file_path.c_str() , "RECREATE" );
    std::map<std::string,std::shared_ptr<TH1D>>::iterator it;
    for( it=genMatchHists.begin(); it!=genMatchHists.end(); ++it ){
	it->second->SetName( (std::string(it->second->GetName())+"_match").c_str() );
        it->second->Write();
    }
    for( it=nonGenMatchHists.begin(); it!=nonGenMatchHists.end(); ++it ){
        it->second->SetName( (std::string(it->second->GetName())+"_nomatch").c_str() );
        it->second->Write();
    }
    outputFilePtr->Close();

    std::cerr<<"###done###"<<std::endl;
    return 0;
}
