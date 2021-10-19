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

// include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Event/interface/Event.h"

void MVAScoreHistograms( const std::string& pathToInputFile, 
			 const std::string& pathToOutputFile ){

    // intialize output map
    std::map< std::string,std::shared_ptr<TH1D>> mvaScoresMap;
    std::vector< std::string > instances;
    for( std::string mvaName : {"TTH","TZQ","TOP"}){
	for( std::string pnp : {"prompt","nonprompt"}){
	    for( std::string flavour : {"muon","electron"}){
		for( std::string pt : {"pt10to25","pt25"}){
		    instances.push_back(mvaName+"_"+pnp+"_"+flavour+"_"+pt);
		}
	    }
	}
    }
    for( std::string instance : instances ){
	mvaScoresMap[instance] = std::make_shared<TH1D>( instance.c_str(), instance.c_str(), 
							    5000, -1., 1. );
    }

    // initialize TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToInputFile );

    // do event loop
    long unsigned numberOfEntries = treeReader.numberOfEntries();
    //long unsigned numberOfEntries = 100;
    std::cout<<"starting event loop for "<<numberOfEntries<<" events"<<std::endl;
    for(long unsigned entry = 0; entry < numberOfEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<numberOfEntries<<std::endl;
        Event event = treeReader.buildEvent(entry,false,false,false,false);
	//double eventWeight = event.weight(); // simply use generator weight
					     // sufficient if not combining multiple samples
	double eventWeight = 1; // to avoid influence of negative weights in tails

	// do basic lepton selection
	event.selectLooseLeptons();
	event.cleanElectronsFromLooseMuons();
	event.cleanTausFromLooseLightLeptons();
	event.removeTaus();

	// loop over leptons
	for(LeptonCollection::const_iterator lIt = event.leptonCollection().cbegin();
	    lIt != event.leptonCollection().cend(); lIt++){
	    std::shared_ptr<Lepton> lep = *lIt;
	    // do additional lepton selection
	    // ...
	    // fill the histograms
	    std::map< std::string,double > mvaValues;
	    std::string pnp = (lep->isPrompt()) ? "prompt" : "nonprompt";
	    std::string flavour = "";
	    std::string pt = (lep->pt() > 25.)? "pt25" : 
				(lep->pt() > 10.)? "pt10to25" : "";
	    if( pt=="" ) continue;
	    if(lep->isElectron()){
		flavour = "electron";
                std::shared_ptr<Electron> ele = std::static_pointer_cast<Electron>(lep);
		mvaValues["TTH"] = ele->leptonMVAttH();
                mvaValues["TZQ"] = ele->leptonMVAtZq();
                mvaValues["TOP"] = ele->leptonMVATOP();
            }
            else if(lep->isMuon()){
		flavour = "muon";
                std::shared_ptr<Muon> mu = std::static_pointer_cast<Muon>(lep);
		mvaValues["TTH"] = mu->leptonMVAttH();
                mvaValues["TZQ"] = mu->leptonMVAtZq();
                mvaValues["TOP"] = mu->leptonMVATOP();
            }
	    std::vector< std::string > mvaNames{ "TTH", "TZQ", "TOP" };
	    //std::cout << pnp+"_"+flavour+"_"+pt << std::endl;
	    for( std::string mvaName : mvaNames ){
		mvaScoresMap[mvaName+"_"+pnp+"_"+flavour+"_"+pt]->Fill( mvaValues[mvaName],eventWeight );		
	    }
	}
    }
    TFile* outputFilePtr = TFile::Open( pathToOutputFile.c_str() , "RECREATE" );
    for( std::string instance : instances ){ mvaScoresMap[instance]->Write(); }
    outputFilePtr->Close();
}

int main( int argc, char* argv[] ){
    std::cerr << "###starting###" << std::endl;
    if( argc < 3 ){
        std::cerr << "### ERROR ### wrong number of command line arguments." << std::endl;
        std::cerr << "Need <pathToInputFile> <pathToOutputFile>" << std::endl;
        return -1;
    }
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    std::string& output_file_path = argvStr[2];
    
    MVAScoreHistograms( input_file_path,output_file_path );
    
    std::cerr << "###done###" << std::endl;
    return 0;
}
