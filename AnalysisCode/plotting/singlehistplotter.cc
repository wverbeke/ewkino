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
#include "../eventselection/interface/eventSelections.h"
#include "../eventselection/interface/eventFlattening.h"

// declare variable getter functions and map
double getLeadingLeptonPt(Event&);
double getSubLeadingLeptonPt(Event&);
double getTrailingLeptonPt(Event&);
double getLeadingPtCorrection(Event&);
double getSubLeadingPtCorrection(Event&);
double getTrailingPtCorrection(Event&);
double getLeadingPtCorrectionRel(Event&);
double getSubLeadingPtCorrectionRel(Event&);
double getTrailingPtCorrectionRel(Event&);
double getMinTOPMVA(Event&);
double getMintZqMVA(Event&);
double getMinttHMVA(Event&);

std::map< std::string, std::function<double(Event&)>> varNameToFuncMap = 
    {	{"leadingLeptonPt", getLeadingLeptonPt},
	{"subLeadingLeptonPt", getSubLeadingLeptonPt},
	{"trailingLeptonPt", getTrailingLeptonPt},
	{"leadingPtCorrection", getLeadingPtCorrection},
	{"subLeadingPtCorrection", getSubLeadingPtCorrection},
	{"trailingPtCorrection", getTrailingPtCorrection},
	{"leadingPtCorrectionRel", getLeadingPtCorrectionRel},
        {"subLeadingPtCorrectionRel", getSubLeadingPtCorrectionRel},
        {"trailingPtCorrectionRel", getTrailingPtCorrectionRel},
	{"minTOPMVA", getMinTOPMVA},
	{"mintZqMVA", getMintZqMVA},
	{"minttHMVA", getMinttHMVA}
    };

// declare event selection functions and map
bool selection_atLeastThreeLooseLightLeptons(Event&);
bool selection_threeFOLightLeptons(Event&);
bool selection_threeFOLightLeptonsThreeTightVeto(Event&);
bool selection_threeTightLightLeptons(Event&);
bool selection_fullEventSelection(Event&);

std::map< std::string, std::function<bool(Event&)>> selectionNameToFuncMap = 
    {	{"atLeastThreeLooseLightLeptons", selection_atLeastThreeLooseLightLeptons},
	{"threeFOLightLeptons", selection_threeFOLightLeptons},
	{"threeFOLightLeptonsThreeTightVeto", selection_threeFOLightLeptonsThreeTightVeto},
	{"threeTightLightLeptons", selection_threeTightLightLeptons},
	{"fullEventSelection", selection_fullEventSelection}
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

///// event selection functions /////

void selection_basic(Event& event){
    event.selectLooseLeptons();
    event.cleanElectronsFromLooseMuons();
    event.cleanTausFromLooseLightLeptons();
    event.removeTaus();
    event.applyLeptonConeCorrection();
    event.sortLeptonsByPt();   
}

bool selection_atLeastThreeLooseLightLeptons(Event& event){
    selection_basic(event);
    int nleptons = event.leptonCollection().size();
    if(nleptons < 3) return false;
    return true;
}

bool selection_threeFOLightLeptons(Event& event){
    selection_basic(event);
    event.selectFOLeptons();
    int nleptons = event.leptonCollection().size();
    if(nleptons != 3) return false;
    return true;
}

bool selection_threeFOLightLeptonsThreeTightVeto(Event& event){
    // note: events with three tight leptons are vetoed but tight leptons are kept;
    // so every event still has at least 3 leptons, of which at least one non-tight!
    // mabye change later to only fill the histograms with non-tight leptons but more complicated.
    if(!selection_threeFOLightLeptons(event)) return false;
    if(event.leptonCollection().numberOfTightLeptons()==3) return false;
    return true;
}

bool selection_threeTightLightLeptons(Event& event){
    selection_basic(event);
    event.selectTightLeptons();
    int nleptons = event.leptonCollection().size();
    if(nleptons != 3) return false;
    return true;
}

bool selection_fullEventSelection(Event& event){
    return passES(event, "signalregion", "3tight", "nominal");
}

///// variable getter functions /////

double getNthLeptonPt(Event& event, int n, bool usecorrectedpt){
    int nleptons = event.leptonCollection().size();
    if(n>=nleptons){
	std::cerr << "### WARNING ###: unexpected number of leptons: "<<nleptons<<std::endl;
	std::cerr << "(the pt of lepton n. "<<n<<" was requested.)"<<std::endl;
	return -99.;
    }
    LeptonCollection::const_iterator lIt = event.leptonCollection().cbegin();
    for(int i=0; i<n; i++){++lIt;}
    Lepton& lep = **lIt;
    double pt = (usecorrectedpt) ? lep.pt() : lep.uncorrectedPt();
    return pt;
}

double getLeadingLeptonPt(Event& event){ return getNthLeptonPt(event,0,true); }
double getSubLeadingLeptonPt(Event& event){ return getNthLeptonPt(event,1,true); }
double getTrailingLeptonPt(Event& event){ return getNthLeptonPt(event,2,true); }
double getLeadingPtCorrection(Event& event){
    return getNthLeptonPt(event,0,true)-getNthLeptonPt(event,0,false); }
double getSubLeadingPtCorrection(Event& event){ 
    return getNthLeptonPt(event,1,true)-getNthLeptonPt(event,1,false); }
double getTrailingPtCorrection(Event& event){ 
    return getNthLeptonPt(event,2,true)-getNthLeptonPt(event,2,false); }
double getLeadingPtCorrectionRel(Event& event){
    return getLeadingPtCorrection(event)/getNthLeptonPt(event,0,false); }
double getSubLeadingPtCorrectionRel(Event& event){
    return getSubLeadingPtCorrection(event)/getNthLeptonPt(event,1,false); }
double getTrailingPtCorrectionRel(Event& event){
    return getTrailingPtCorrection(event)/getNthLeptonPt(event,2,false); }


double getMinLightLeptonValue(Event& event, double (LightLepton::*value)() const ){
    double minvalue = 99.;
    for( const auto& llepPtr : event.lightLeptonCollection() ){
	if( (*llepPtr.*value)() < minvalue ) minvalue = (*llepPtr.*value)();
    }
    return minvalue;
}

double getMinTOPMVA(Event& event){ return getMinLightLeptonValue(event, &LightLepton::leptonMVATOP); }
double getMintZqMVA(Event& event){ return getMinLightLeptonValue(event, &LightLepton::leptonMVAtZq); }
double getMinttHMVA(Event& event){ return getMinLightLeptonValue(event, &LightLepton::leptonMVAttH); }

void fillHistograms(const std::string& pathToFile, const std::string& outputFilePath,
		    const std::string& eventSelection, 
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
    //long unsigned numberOfEntries = 10000;
    std::cout<<"starting event loop for "<<numberOfEntries<<" events."<<std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%10000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
	// build event and perform event selection
        Event event = treeReader.buildEvent(entry,false,false);
	if(!selectionNameToFuncMap[eventSelection](event)) continue;

	// define weight (usually just generator weight)
	double weight = event.weight();
	if(event.isData()) weight = 1;

	// fill histograms
	for(std::string variable : variables){
	    double varvalue = varNameToFuncMap[variable](event);
	    if(varvalue < xlow || varvalue > xhigh) continue;
	    histmap[variable]->Fill(varvalue,weight);
	}

	// for testing
	/*int ntight = event.leptonCollection().numberOfTightLeptons();
	std::cout << "event with " << ntight << " tight leptons" << std::endl;
	if(ntight < 3){
	    std::cout << event.leptonCollection()[0].pt() - event.leptonCollection()[0].uncorrectedPt() << std::endl;
	    std::cout << event.leptonCollection()[1].pt() - event.leptonCollection()[1].uncorrectedPt() << std::endl;
	    std::cout << event.leptonCollection()[2].pt() - event.leptonCollection()[2].uncorrectedPt() << std::endl;
	}*/
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
    if( argc < 8 ){
        std::cerr << "### ERROR ###: plotsinglehist.cc at least 7 arguments to run: " << std::endl;
        std::cerr << "input_file_path, output_file_path, event_selection, xlow, xhigh, nbins," << std::endl;
	std::cerr << "at least one variable." << std::endl;
        return -1;
    }
    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    std::string& output_file_path = argvStr[2];
    std::string& event_selection = argvStr[3];
    double xlow = std::stod(argvStr[4]);
    double xhigh = std::stod(argvStr[5]);
    int nbins = std::stod(argvStr[6]);
    std::vector<std::string> variables;
    for(int i=7; i<argc; i++){
        variables.push_back(argvStr[i]);
    } 
    // check validity
    bool validInput = checkReadability( input_file_path );
    if(!validInput) return -1;
    // fill the histograms
    fillHistograms(input_file_path, output_file_path, event_selection, xlow, xhigh, nbins, variables);
    std::cout<<"done"<<std::endl;
    return 0;
}
