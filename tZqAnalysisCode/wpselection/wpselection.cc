//include c++ library classes 
#include <string>
#include <vector>
#include <exception>
#include <iostream>
#include <fstream>

// include ROOT classes 
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"
#include "TMVA/Reader.h"

// include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"
#include "../eventselection/interface/eventSelections.h"
#include "../eventselection/interface/eventFlattening.h"

std::map<double, std::vector<double>> eventloopWPS(const std::string& pathToFile, 
			const double norm, const std::vector<double> wps,
			const bool useMVA, const std::string& pathToXMLFile){

    // intialize result vector
    std::map<double, std::vector<double>> passtable;
    for(unsigned i=0; i<wps.size(); ++i) passtable[wps[i]] = {0.,0.,0.,0.};

    // initialize TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToFile );

    // initialize tmva reader
    TMVA::Reader* reader = new TMVA::Reader();
    if(useMVA){ reader = initializeReader(reader, pathToXMLFile); }

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    //long unsigned numberOfEntries = 10000;
    std::cout<<"starting event loop for "<<numberOfEntries<<" events"<<std::endl;
    std::string eventselection = "signalregion";
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
        Event event = treeReader.buildEvent(entry,true,true);
	// event must pass signal region event selection
	// note: this function assumes the leptonMVACut in the FO and Tight ID 
	// has been set to a sufficiently low value!
	// additional cuts on leptonMVA value are applied afterwards for different WP's.
        if(!passES(event, eventselection, "tzq", false)) continue;
	if(eventCategory(event, "nominal")<0) continue;

	// furthermore, use event flattening to get the event BDT output value
	if(useMVA){
	    eventToEntry(event, norm, "tzq", false, nullptr, nullptr, "nominal");
	    _eventBDT = reader->EvaluateMVA( "BDT" );
	    if(_eventBDT < 0) continue;
	}

	for(unsigned i=0; i<wps.size(); ++i){
	    double wp = wps[i];
	    bool passwp = true;
	    for(LeptonCollection::const_iterator lIt = event.leptonCollection().cbegin();
					    lIt != event.leptonCollection().cend(); lIt++){
		std::shared_ptr<Lepton> lep = *lIt;
		if(lep->isElectron()){
		    std::shared_ptr<Electron> ele = std::static_pointer_cast<Electron>(lep);
		    //if(ele->leptonMVAtZq() < wp) passwp = false;
		    if(ele->leptonMVATOP() < wp) passwp = false;
		}
		else if(lep->isMuon()){
		    std::shared_ptr<Muon> mu = std::static_pointer_cast<Muon>(lep);
		    //if(mu->leptonMVAtZq() < wp) passwp = false;
		    if(mu->leptonMVATOP() < wp) passwp = false;
		}
	    }
	    if(passwp){
		int nmu = event.numberOfMuons();
		int nel = event.numberOfElectrons();
		double weight = event.weight()*norm;
		//double weight = 1;
		if(nmu==3 && nel==0) passtable.at(wp).at(0) += weight;
		else if(nmu==2 && nel==1) passtable.at(wp).at(1) += weight;
		else if(nmu==1 && nel==2) passtable.at(wp).at(2) += weight;
		else if(nmu==0 && nel==3) passtable.at(wp).at(3) += weight;
		else std::cerr<<"WARNING: found event with unexpected lepton content"<<std::endl;
	    }
	}
    }
    delete reader;
    return passtable;
}

void writeToFile(const std::map<double, std::vector<double>> themap, std::string outFilePath){
    std::ofstream file;
    file.open(outFilePath);
    for(std::map<double,std::vector<double>>::const_iterator it = themap.begin(); 
	    it!=themap.end(); it++){
        file << it->first << ":" << it->second[0] << ":" << it->second[1] << ":";
	file << it->second[2] << ":" << it->second[3] << "\n";
    }
    file.close();
}

int main( int argc, char* argv[] ){
    if( argc < 7 ){
        std::cerr << "event selection at least six arguments to run: " << std::endl;
        std::cerr << "input_file_path, norm, output_file_path, useMVA, pathToXMLFile,";
        std::cerr << "at least one WP." << std::endl;
        return -1;
    }
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    double norm = std::stod(argvStr[2]);
    std::string& output_file_path = argvStr[3];
    bool useMVA = (argvStr[4]=="True" || argvStr[4]=="true");
    std::string& pathToXMLFile = argvStr[5];
    std::vector< double > wps;
    for(int i=6; i<argc; ++i){wps.push_back(std::stod(argvStr[i]));}

    std::map<double, std::vector<double>> passtable = eventloopWPS(input_file_path, norm, wps,
							useMVA, pathToXMLFile);

    writeToFile(passtable,output_file_path);
    
    std::cout<<"done"<<std::endl;
    return 0;
}
