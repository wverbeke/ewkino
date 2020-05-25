// A plotting script that takes as input a single file.
// Use it to make plots of variables in samples that are not fully selected and/or flattened.

// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>
#include <functional>

// include ROOT classes 
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"
#include "TMVA/Reader.h"

// include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Tools/interface/HistInfo.h"
#include "../../Event/interface/Event.h"
//#include "../../weights/interface/ConcreteReweighterFactory.h"
//#include "../eventselection/interface/eventSelections.h"
//#include "../eventselection/interface/eventFlattening.h"

double getLeadingLeptonPt(Event&);
double getSubLeadingLeptonPt(Event&);
double getTrailingLeptonPt(Event&);

std::map< std::string, std::function<double(Event&)>> varNameToFuncMap = 
    {	{"leadingLeptonPt", getLeadingLeptonPt},
	{"subLeadingLeptonPt", getSubLeadingLeptonPt},
	{"trailingLeptonPt", getTrailingLeptonPt}
    };

bool checkReadability(const std::string& pathToFile){
    // temporary function to perform error handling when file cannot be read.
    // to be replaced by exception throwing in once figured out how this works in c++.
    TreeReader treeReader;
    try{treeReader.initSampleFromFile( pathToFile );}
    catch( std::domain_error& ){
        std::cerr << "Can not read file. Returning." << std::endl;
        return false;
    }
    return true;
}

double getNthLeptonPt(Event& event, int n){
    // apply the same selection that was used for skimming
    event.selectLooseLeptons();
    event.cleanElectronsFromLooseMuons();
    event.cleanTausFromLooseLightLeptons();
    // additionally remove taus?
    int nleptons = event.leptonCollection().size();
    /*if(nleptons != 3 or n>=nleptons){
	std::cerr << "### WARNING ###: unexpected number of leptons: "<<nleptons<<std::endl;
	std::cerr << "(the pt of lepton n. "<<n<<" was requested.)"<<std::endl;
	return 0.;
    }*/
    LeptonCollection::const_iterator lIt = event.leptonCollection().cbegin();
    for(int i=0; i<n; i++){++lIt;}
    Lepton& lep = **lIt;
    return lep.pt();
}

double getLeadingLeptonPt(Event& event){ return getNthLeptonPt(event,0); }
double getSubLeadingLeptonPt(Event& event){ return getNthLeptonPt(event,1); }
double getTrailingLeptonPt(Event& event){ return getNthLeptonPt(event,2); }

void fillHistograms(const std::string& pathToFile, const std::string& outputFilePath,
		    const double xlow, const double xhigh, const int nbins, 
		    const std::vector<std::string> variables){

    // initialize TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );

    // make output collection of histograms
    HistInfo histInfo( "", "x axis title", nbins, xlow, xhigh);
    std::map<std::string, std::shared_ptr<TH1D>> histmap;
    for(std::string variable : variables){
	histmap[variable] = histInfo.makeHist( variable );
    }

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    //long unsigned numberOfEntries = 100000;
    std::cout<<"starting event loop for "<<numberOfEntries<<" events."<<std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%10000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
	// build event and perform event selection
        Event event = treeReader.buildEvent(entry,false,false);

	// define weight (usually just generator weight)
	double weight = event.weight();

	// fill histograms
	for(std::string variable : variables){
	    double varvalue = varNameToFuncMap[variable](event);
	    histmap[variable]->Fill(varvalue,weight);
	}
    }
    // make output ROOT file
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    outputFilePtr->cd();
    for(std::string variable : variables){
	histmap[variable]->Write();
    }
    outputFilePtr->Close();
}

int main( int argc, char* argv[] ){
    if( argc < 7 ){
        std::cerr << "### ERROR ###: plotsinglehist.cc at least 6 arguments to run: " << std::endl;
        std::cerr << "input_file_path, output_file_path, xlow, xhigh, nbins," << std::endl;
	std::cerr << "at least one variable." << std::endl;
        return -1;
    }
    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    std::string& output_file_path = argvStr[2];
    double xlow = std::stod(argvStr[3]);
    double xhigh = std::stod(argvStr[4]);
    int nbins = std::stod(argvStr[5]);
    std::vector<std::string> variables;
    for(int i=6; i<argc; i++){
        variables.push_back(argvStr[i]);
    } 
    // check validity
    bool validInput = checkReadability( input_file_path );
    if(!validInput) return -1;
    // fill the histograms
    fillHistograms(input_file_path, output_file_path, xlow, xhigh, nbins, variables);
    std::cout<<"done"<<std::endl;
    return 0;
}
