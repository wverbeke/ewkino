// This is a strongly reduced version of runsystematics.cc,
// specifically meant to do a 'stress test' for pileup reweighting,
// see e.g. TopSystematics (https://twiki.cern.ch/twiki/bin/view/CMS/TopSystematics)

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
#include "../../../TreeReader/interface/TreeReader.h"
#include "../../../Tools/interface/stringTools.h"
#include "../../../Tools/interface/HistInfo.h"
//#include "../../../Tools/interface/SampleCrossSections.h"
#include "../../../Event/interface/Event.h"
#include "../../../weights/interface/ConcreteReweighterFactory.h"
#include "../../../weights/interface/ReweighterPileup.h"

// general help functions //

bool checkReadability(const std::string& pathToFile){
    // temporary function to perform error handling when file cannot be read.
    // maybe to be replaced by exception throwing
    TreeReader treeReader;
    try{treeReader.initSampleFromFile( pathToFile );}
    catch( std::domain_error& ){
        std::cerr << "Can not read file. Returning." << std::endl;
        return false;
    }
    return true;
}

double getVarValue(const std::string varname, std::map<std::string,double> varmap){
    // retrieve a value from a map of all variables associated to the event
    std::map<std::string,double>::iterator it = varmap.find(varname);
    if(it == varmap.end()) return -9999;
    else return it->second;
}


void fillVarValue(std::shared_ptr<TH1D> hist, const double varvalue, const double weight){
    // extension of hist->Fill(), explicitly adding under- and overflow to first resp. last bin
    double lowx = hist->GetBinLowEdge(1);
    double highx = hist->GetBinLowEdge(hist->GetNbinsX())+hist->GetBinWidth(hist->GetNbinsX());
    double newvarvalue = varvalue;
    if(newvarvalue<lowx) newvarvalue = lowx + hist->GetBinWidth(1)/2.;
    if(newvarvalue>highx) newvarvalue = highx - hist->GetBinWidth(hist->GetNbinsX())/2.;
    hist->Fill(newvarvalue,weight);
}

// help functions for trigger and pt-threshold selections //

bool passAnyTrigger(Event& event){
    bool passanytrigger = event.passTriggers_e() || event.passTriggers_ee()
                        || event.passTriggers_eee() || event.passTriggers_m()
                        || event.passTriggers_mm() || event.passTriggers_mmm()
                        || event.passTriggers_em() || event.passTriggers_eem()
                        || event.passTriggers_emm();
    return passanytrigger;
}

bool passLeptonPtThresholds(Event& event){
    event.sortLeptonsByPt();
    if(event.leptonCollection()[0].pt() < 25.
        || event.leptonCollection()[1].pt() < 15.
        || event.leptonCollection()[2].pt() < 10.) return false;
    return true;
}

// help function for histogram initialization //

std::map< std::string,std::map< std::string,std::shared_ptr<TH1D>> > initHistMap(
				const std::vector<std::tuple<std::string,double,double,int>>& vars,
				const std::string processName ){
    // initialize the output histogram map
    std::map< std::string,std::map< std::string,std::shared_ptr<TH1D>> > histMap;
    // loop over variables
    for(unsigned int i=0; i<vars.size(); ++i){
	// get variable info from input struct (vector of tuples)
        std::string variable = std::get<0>(vars[i]);
        double xlow = std::get<1>(vars[i]);
        double xhigh = std::get<2>(vars[i]);
        int nbins = std::get<3>(vars[i]);
        HistInfo histInfo( "", variable.c_str(), nbins, xlow, xhigh);
	// parse the process name and variable name to form the histogram name
	std::string name = processName+"_"+variable;
	name = stringTools::removeOccurencesOf(name," ");
	name = stringTools::removeOccurencesOf(name,"/");
	name = stringTools::removeOccurencesOf(name,"#");
	name = stringTools::removeOccurencesOf(name,"{");
	name = stringTools::removeOccurencesOf(name,"}");
	name = stringTools::removeOccurencesOf(name,"+");
	// initialize the histograms
        histMap[variable]["nominal"] = histInfo.makeHist( name+"_nominal" );
	histMap[variable]["nominal"]->SetTitle(processName.c_str());
        histMap[variable]["pileupUp"] = histInfo.makeHist( name+"_pileupUp" );
	histMap[variable]["pileupUp"]->SetTitle(processName.c_str());
	histMap[variable]["pileupDown"] = histInfo.makeHist( name+"_pileupDown" );
	histMap[variable]["pileupDown"]->SetTitle(processName.c_str());
    }
    return histMap;
}

// histogram help functions //

void clipHistogram( std::shared_ptr<TH1D> hist ){
    // put all bins of a histogram to minimum zero (no net negative weights)
    for(int i=0; i<hist->GetNbinsX()+2; ++i){
	if(hist->GetBinContent(i)<0.){
	    hist->SetBinContent(i,0.);
	    hist->SetBinError(i,0.);
	}
    }
}

void copyHistogram( std::shared_ptr<TH1D> histlhs, std::shared_ptr<TH1D> histrhs){
    // copy bin content of histrhs into histlhs while keeping original name and title
    histlhs->Reset();
    for(int i=0; i<histrhs->GetNbinsX()+2; ++i){
	histlhs->SetBinContent(i,histrhs->GetBinContent(i));
    }
}

void fillSystematicsHistograms(const std::string& pathToFile,
				const double norm, 
				const std::string& outputFilePath, 
				const std::string& processName,
				const std::vector<std::tuple<std::string,double,double,int>> vars){

    // initialize TreeReader from input file
    std::cout<<"start function fillSystematicsHistograms"<<std::endl;
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );
    std::string year = "2016";
    if(treeReader.is2017()) year = "2017";
    if(treeReader.is2018()) year = "2018";

    // make flat vector of variable names (often more convenient then argument structure)
    std::vector<std::string> variables;
    for(unsigned int i=0; i<vars.size(); ++i){
	std::string variable = std::get<0>(vars[i]);
	variables.push_back(variable);
    }

    // make pileup reweighter
    std::vector<Sample> thissample;
    thissample.push_back(treeReader.currentSample());
    CombinedReweighter reweighter;
    reweighter.addReweighter( "pileup", std::make_shared< ReweighterPileup >( 
				thissample, "../../../weights" ) );
    
    // make output collection of histograms
    std::cout<<"making output collection of histograms"<<std::endl;
    std::map< std::string,std::map< std::string,std::shared_ptr<TH1D>> > histMap =
        initHistMap(vars,processName);

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    //long unsigned numberOfEntries = 10; 
    numberOfEntries = std::min(numberOfEntries,treeReader.numberOfEntries());
    std::cout<<"starting event loop for "<<numberOfEntries<<" events."<<std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;	
        Event event = treeReader.buildEvent(entry);

	// do some event selection (equivalent to trilepton skim)
	event.selectLooseLeptons();
	event.cleanElectronsFromLooseMuons();
	event.cleanTausFromLooseLightLeptons();
	if( event.numberOfLeptons() < 3 ) continue;
	// select only tight leptons
	//event.selectTightLeptons();
	//if( event.numberOfLeptons() != 3 ) continue;
	//if( !passAnyTrigger(event) || passLeptonPtThresholds(event) ) continue;
	event.sortLeptonsByPt();
	LeptonCollection lepcollection = event.leptonCollection();

	// set variables
	std::map< std::string,double > varmap;
	varmap["_leptonPtLeading"] = lepcollection[0].pt();
        varmap["_leptonPtSubLeading"] = lepcollection[1].pt();
        varmap["_leptonPtTrailing"] = lepcollection[2].pt();
        varmap["_leptonEtaLeading"] = lepcollection[0].eta();
        varmap["_leptonEtaSubLeading"] = lepcollection[1].eta();
        varmap["_leptonEtaTrailing"] = lepcollection[2].eta();
	varmap["_yield"] = 0.5;
	varmap["_vertexMultiplicity"] = event.numberOfVertices();

	

	// declare weights
	double nominalweight = 0;
	double upweight = 0;
	double downweight = 0;
	// set weights for MC
	if(!treeReader.currentSample().isData()){
	    double weight = event.weight() * norm;
	    nominalweight = weight * reweighter.totalWeight(event);
	    upweight = weight * reweighter["pileup"]->weightUp(event);
	    downweight = weight * reweighter["pileup"]->weightDown(event);
	}
	// set weights for data
	else{
	    nominalweight = 1;
	}

	// loop over variables
	for(std::string variable : variables){
	 
	    // fill nominal
	    fillVarValue(histMap[variable][std::string("nominal")],
			 getVarValue(variable,varmap),nominalweight);

	    // stop further event processing in case of data
	    if(event.isData()) continue;

	    // fill up and down
	    fillVarValue(histMap[variable]["pileupUp"],getVarValue(variable,varmap),upweight);
	    fillVarValue(histMap[variable]["pileupDown"],getVarValue(variable,varmap),downweight);
	} // end loop over variables
    } // end loop over events

    // make output ROOT file
    TFile* outputFilePtr = TFile::Open( outputFilePath.c_str() , "RECREATE" );
    // save histograms to the output file
    // loop over variables
    for(std::string variable : variables){
	// first find nominal histogram for this variable
	std::shared_ptr<TH1D> nominalhist;
	for(auto mapelement : histMap[variable]){
	    if( stringTools::stringContains(mapelement.second->GetName(),"nominal") ){
		nominalhist = mapelement.second;
	    }
	}
	// clip and write nominal histogram
	clipHistogram(nominalhist);
	nominalhist->Write();
	// loop over rest of histograms
	for(auto mapelement : histMap[variable]){
	    if( stringTools::stringContains(mapelement.second->GetName(),"nominal")){ continue; }
	    std::shared_ptr<TH1D> hist = mapelement.second;
	    clipHistogram(hist);
	    // save histograms
	    hist->Write();
	}
    }
    outputFilePtr->Close();
}

int main( int argc, char* argv[] ){

    std::cerr << "###starting###" << std::endl;

    if( argc != 5 ){
        std::cerr << "### ERROR ###: fillpileup.cc requires 4 arguments to run: ";
        std::cerr << "input_file_path, norm, output_file_path, process_name " << std::endl;
        return -1;
    }

    // parse arguments
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    double norm = std::stod(argvStr[2]);
    std::string& output_file_path = argvStr[3];
    std::string& process_name = argvStr[4];

    // make structure for variables
    std::vector<std::tuple<std::string,double,double,int>> vars;
    vars.push_back(std::make_tuple("_vertexMultiplicity",-0.5,70.5,71));
    vars.push_back(std::make_tuple("_yield",0.,1.,1));
    vars.push_back(std::make_tuple("_leptonPtLeading",0.,300.,12));
    vars.push_back(std::make_tuple("_leptonPtSubLeading",0.,180.,12));
    vars.push_back(std::make_tuple("_leptonPtTrailing",0.,120.,12));
    vars.push_back(std::make_tuple("_leptonEtaLeading",-2.5,2.5,20));
    vars.push_back(std::make_tuple("_leptonEtaSubLeading",-2.5,2.5,20));
    vars.push_back(std::make_tuple("_leptonEtaTrailing",-2.5,2.5,20));

    // check validity
    bool validInput = checkReadability( input_file_path );
    if(!validInput) return -1;

    // fill the histograms
    fillSystematicsHistograms(input_file_path, norm, output_file_path, process_name, vars);

    std::cerr << "###done###" << std::endl;
    return 0;
}
