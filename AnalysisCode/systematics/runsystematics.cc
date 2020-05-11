// This is the main C++ class used to run the systematics.
// It is supposed to run on the output file of an event selection procedure
// and produce a root file containing a histogram of systematic up and down variations.

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
#include "../../Tools/interface/HistInfo.h"
#include "../../Event/interface/Event.h"
#include "../../weights/interface/ConcreteReweighterFactory.h"
#include "../eventselection/interface/eventSelections.h"
#include "../eventselection/interface/eventFlattening.h"

bool checkReadability(const std::string& pathToFile){
    // temporary function to perform error handling when file cannot be read.
    // to be replaced by exception throwing in eventloopEF once figured out how this works in c++.
    TreeReader treeReader;
    try{treeReader.initSampleFromFile( pathToFile );}
    catch( std::domain_error& ){
        std::cerr << "Can not read file. Returning." << std::endl;
        return false;
    }
    return true;
}

std::string systematicType(const std::string systematic){
    // get type of sytematic
    // so far either "acceptance" or "weight"
    if(systematic=="JEC" or systematic=="JER" or systematic=="Uncl"){
	return std::string("acceptance");
    }
    if(systematic=="pileup"){ // extend later
	return std::string("weight");
    }
    std::cerr<<"### ERROR ###: systematic '"<<systematic<<"' not recognized."<<std::endl;
    return std::string("ERROR");
}

double getVarValue(const std::string varname){
    // not sure if there is a better way in C++
    // also not sure about return type (maby float?)
    if(varname=="_eventBDT") return _eventBDT;
    if(varname=="_abs_eta_recoil") return _abs_eta_recoil;
    // extend later
    std::cerr<<"### ERROR ###: variable '"<<varname<<"' not recognized."<<std::endl;
    return -99;
}

void fillSystematicsHistograms(const std::string& pathToFile, const double norm, 
				const std::string& outputFilePath,
				const std::string& leptonID, 
				const std::string& variable,
				const std::string pathToXMLFile,
				const double xlow, const double xhigh, const int nbins,
				const std::vector<std::string>& systematics){

    // initialize TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );
    std::string year = "2016";
    if(treeReader.is2017()) year = "2017";
    if(treeReader.is2018()) year = "2018";

    // make output collection of histograms
    HistInfo histInfo( "", variable.c_str(), nbins, xlow, xhigh);
    std::map< std::string, std::shared_ptr<TH1D> > histMap;
    histMap[std::string("nominal")] = histInfo.makeHist(std::string("nominal"));
    for(std::string systematic : systematics){
	std::string temp = systematic + "Up";
	histMap[temp] = histInfo.makeHist( temp );
	temp = systematic + "Down";
	histMap[temp] = histInfo.makeHist( temp );
    }

    // initialize TMVA reader
    TMVA::Reader* reader = new TMVA::Reader();
    if(variable=="_eventBDT"){ reader = initializeReader(reader, pathToXMLFile); }

    // make reweighter
    std::shared_ptr< ReweighterFactory >reweighterFactory( new tZqReweighterFactory() );
    std::vector<Sample> thissample;
    thissample.push_back(treeReader.currentSample());
    CombinedReweighter reweighter = reweighterFactory->buildReweighter( 
					"../../weights/", year, thissample );

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    //long unsigned numberOfEntries = 100;
    std::cout<<"starting event loop for "<<numberOfEntries<<" events."<<std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
	// build event and perform event selection
        Event event = treeReader.buildEvent(entry,true,true);
        if(!passES(event, "signalregion", leptonID, false)) continue;
	// fill nominal histogram
	int eventcategory = eventCategory(event, "nominal");
        if(eventcategory == -1) continue;
	eventToEntry(event, norm, leptonID, false, nullptr, nullptr, "nominal");
        if(variable=="_eventBDT"){ _eventBDT = reader->EvaluateMVA( "BDT" ); }
        if(event.isMC()) _normweight *= reweighter.totalWeight(event);
        histMap[std::string("nominal")]->Fill(getVarValue(variable),_normweight);
	// loop over systematics
	for(std::string systematic : systematics){
	    // determine type of systematic (acceptance or weight)
	    std::string sysType = systematicType(systematic);
	    if(sysType=="acceptance"){
		// do event selection and flattening with up variation
		std::string upvar = systematic + "Up";
		int eventcategory = eventCategory(event, upvar);
		if(eventcategory == -1) continue;
		eventToEntry(event, norm, leptonID, false, nullptr, nullptr, upvar);
		if(variable=="_eventBDT"){ _eventBDT = reader->EvaluateMVA( "BDT" ); }
		if(event.isMC()) _normweight *= reweighter.totalWeight(event);
		histMap[upvar]->Fill(getVarValue(variable),_normweight);
		// and with down variation
		std::string downvar = systematic + "Down";
                eventcategory = eventCategory(event, downvar);
                if(eventcategory == -1) continue;
                eventToEntry(event, norm, leptonID, false, nullptr, nullptr, downvar);
                if(variable=="_eventBDT"){ _eventBDT = reader->EvaluateMVA( "BDT" ); }
                if(event.isMC()) _normweight *= reweighter.totalWeight(event);
                histMap[downvar]->Fill(getVarValue(variable),_normweight);
	    }
	    else if(sysType=="weight"){
		// do nominal event selection
                int eventcategory = eventCategory(event, "nominal");
                if(eventcategory == -1) continue;
                eventToEntry(event, norm, leptonID, false, nullptr, nullptr, "nominal");
                if(variable=="_eventBDT"){ _eventBDT = reader->EvaluateMVA( "BDT" ); }
		// apply up and down weights
		double upweight = _normweight;
		double downweight = _normweight;
                if(event.isMC()){
		    double reweight = reweighter.totalWeight(event);
		    reweight = reweight/reweighter[systematic]->weight(event);
		    upweight = upweight * reweight * reweighter[systematic]->weightUp(event);
		    downweight = downweight * reweight * reweighter[systematic]->weightDown(event);
		}
		std::string upvar = systematic + "Up";
		histMap[upvar]->Fill(getVarValue(variable),upweight);
		std::string downvar = systematic + "Down";
		histMap[downvar]->Fill(getVarValue(variable),downweight);
	    }
        }
    }
    // make output ROOT file
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    //outputFilePtr->cd();
    for(auto mapelement : histMap) mapelement.second->Write();
    outputFilePtr->Close();

    // delete TMVA reader
    delete reader;
}

int main( int argc, char* argv[] ){
    if( argc < 11 ){
        std::cerr << "### ERROR ###: runsystematics.cc requires at least 10 arguments to run: ";
        std::cerr << "input_file_path, norm, output_file_path ";
	std::cerr << "leptonID, variable, pathToXMLFile ";
	std::cerr << "xlow, xhigh, nbins, ";
	std::cerr << "at least one systematic" << std::endl;
        return -1;
    }
    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    double norm = std::stod(argvStr[2]);
    std::string& output_file_path = argvStr[3];
    std::string& leptonID = argvStr[4];
    std::string& variable = argvStr[5];
    std::string& pathToXMLFile = argvStr[6];
    double xlow = std::stod(argvStr[7]);
    double xhigh = std::stod(argvStr[8]);
    int nbins = std::stod(argvStr[9]);
    std::vector<std::string> systematics;
    for(int i=10; i<argc; i++){
	systematics.push_back(argvStr[i]);
    }
    // check validity
    bool validInput = checkReadability( input_file_path );
    if(!validInput) return -1;
    double testvarvalue = getVarValue(variable);
    if(testvarvalue==99) return -1;
    for(std::string systematic : systematics){
	std::string testsyst = systematicType(systematic);
	if(testsyst=="ERROR") return -1;
    }

    // fill the histograms
    fillSystematicsHistograms(input_file_path, norm, output_file_path, 
				    leptonID, variable, pathToXMLFile,
				    xlow, xhigh, nbins,
				    systematics);
    std::cout<<"done"<<std::endl;
    return 0;
}
