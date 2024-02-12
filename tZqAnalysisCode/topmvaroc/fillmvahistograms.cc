/*
Make histograms to plot ROC curves for the lepton MVA
*/

// Similar ROC curves are shown in AN-2022-016 (Kirill's lepton MVA AN).
// This is an explicit attempt to obtain ROC curves that are compatible with those.
// Note: small differences can be expected because of UL samples vs pre-UL samples.

// Remark: reproduction of the plots in AN-2022-016 is probably not possible
// with the samples that were actually used for the tZq analysis,
// as the ntuplizer at that point in history still contained dxy and dz cuts on leptons.

// Therefore, just make ROC curves with respect to the loose lepton ID used in the tZq analysis,
// but now at least the difference between the plots is understood and explainable.


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

void makeMVAScoreHistograms(
    const std::string& pathToInputFile,
    const std::string& pathToOutputFile,
    long nEntries ){

    // intialize output map
    std::map< std::string,std::shared_ptr<TH1D>> mvaScoresMap;
    std::vector< std::string > mvaInstances;
    std::vector< std::string > mvaNames = {"TTH","TZQ","TOP"};
    for( std::string mvaName : mvaNames ){
	for( std::string pnp : {"prompt","nonprompt"}){
	    for( std::string flavour : {"muon","electron"}){
		for( std::string pt : {"pt10","pt10to25","pt25"}){
		    mvaInstances.push_back(mvaName+"_"+pnp+"_"+flavour+"_"+pt);
		}
	    }
	}
    }
    for( std::string instance : mvaInstances ){
	mvaScoresMap[instance] = std::make_shared<TH1D>(
	    instance.c_str(), instance.c_str(), 5000, -1., 1. );
    }

    // initialize a second output map for cut-based IDs
    std::map< std::string,std::shared_ptr<TH1D>> cutBasedIDMap;
    std::vector< std::string > cutBasedIDInstances;
    std::vector< std::string > cutBasedIDNames = {"POGLoose","POGMedium","POGTight"};
    // also append specific working points for the MVAs above
    cutBasedIDNames.push_back("TOPMedium");
    cutBasedIDNames.push_back("TOPTight");
    for( std::string idName : cutBasedIDNames ){
        for( std::string pnp : {"prompt","nonprompt"}){
            for( std::string flavour : {"muon","electron"}){
                for( std::string pt : {"pt10","pt10to25","pt25"}){
                    cutBasedIDInstances.push_back(idName+"_"+pnp+"_"+flavour+"_"+pt);
                }
            }
        }
    }
    for( std::string instance : cutBasedIDInstances ){
        cutBasedIDMap[instance] = std::make_shared<TH1D>(
            instance.c_str(), instance.c_str(), 2, -0.5, 1.5 );
    }

    // initialize TreeReader
    TreeReader treeReader;
    treeReader.initSampleFromFile( pathToInputFile );

    // do event loop
    long numberOfEntries = treeReader.numberOfEntries();
    if( nEntries<0 || nEntries>numberOfEntries ) nEntries = numberOfEntries;
    for(long entry = 0; entry < nEntries; entry++){
        if(entry%1000 == 0) std::cout<<"processed: "<<entry<<" of "<<nEntries<<std::endl;
        Event event = treeReader.buildEvent(entry,false,false,false,false);
	// determine suitable weight per event (also to be used per lepton)
	// - either use generator weight (ok if not combining multiple samples in one histogram)
	// - or just use weight 1 for all events (to avoid issues with negative weights)
	//double eventWeight = event.weight();
	double eventWeight = 1;

	// do basic lepton selection
	// see comments above: synchronization with AN-2022-016 is not exactly possible,
	// so just make the ROC curves with respect to the loose lepton ID used in this analysis
	event.cleanElectronsFromLooseMuons();
	event.removeTaus();
	event.selectLooseLeptons();
	
	// loop over leptons
	for(LeptonCollection::const_iterator lIt = event.leptonCollection().cbegin();
	    lIt != event.leptonCollection().cend(); lIt++){
	    std::shared_ptr<Lepton> lep = *lIt;

	    // determine lepton flavor
	    std::shared_ptr<Muon> mu;
	     std::shared_ptr<Electron> ele;
	    if( lep->isMuon() ) mu = std::static_pointer_cast<Muon>(lep);
	    else if( lep->isElectron() ) ele = std::static_pointer_cast<Electron>(lep);
	    else continue;
 
	    // do lepton selection
	    // try to reproduce "loose" (v2) selections from AN-2022-016
	    /*if( lep->uncorrectedPt() < 10 ) continue;
	    if( lep->absEta() > 2.5 ) continue;
	    if( lep->isMuon() && lep->absEta() > 2.4 ) continue;
	    if( lep->sip3d() > 15 ) continue;
	    if( lep->isMuon() && (mu->relIso0p4() > 1) ) continue;
	    if( lep->isElectron() && (ele->relIso0p4() > 1) ) continue;*/

	    // determine the lepton parameters
	    std::map< std::string,double > mvaValues;
	    std::map< std::string,int > cutBasedIDValues;
	    std::string pnp = (lep->isPrompt()) ? "prompt" : "nonprompt";
	    std::string flavour = "";
	    std::vector<std::string> pttags;
	    if( lep->uncorrectedPt() > 25. ) pttags.push_back("pt25");
	    if( lep->uncorrectedPt() > 10. ) pttags.push_back("pt10");
	    if( (lep->uncorrectedPt() > 10.) && (lep->uncorrectedPt() < 25.) ) pttags.push_back("pt10to25");
	    if( pttags.size()==0 ) continue;
	    if(lep->isElectron()){
		flavour = "electron";
		mvaValues["TTH"] = ele->leptonMVAttH();
                mvaValues["TZQ"] = ele->leptonMVAtZq();
                mvaValues["TOP"] = ele->leptonMVATOP();
		// for electrons: can use cut-based ID defined by the POG directly.
		// reference for definition: CMS-EGM-17-001
                cutBasedIDValues["POGLoose"] = (int) ele->isLoosePOGElectron();
                cutBasedIDValues["POGMedium"] = (int) ele->isMediumPOGElectron();
                cutBasedIDValues["POGTight"] = (int) ele->isTightPOGElectron();
		// also add working points of lepton MVAs
		cutBasedIDValues["TOPMedium"] = (int) (ele->leptonMVATOP()>0.4);
		cutBasedIDValues["TOPTight"] = (int) (ele->leptonMVATOP()>0.9);
            }
            else if(lep->isMuon()){
		flavour = "muon";
		mvaValues["TTH"] = mu->leptonMVAttH();
                mvaValues["TZQ"] = mu->leptonMVAtZq();
                mvaValues["TOP"] = mu->leptonMVATOP();
		// for muons: using cut-based POG ID in comparison is not really useful,
		// as medium POG ID is already used in loose definition.
		// moreover, it is designed to be efficient both for prompt leptons
		// and for leptons from heavy quark decays, and e.g. isolation cuts should be added.
		// see CMS-MUO-16-001
		// and https://twiki.cern.ch/twiki/bin/viewauth/CMS/SWGuideMuonIdRun2#Muon_Identification
                cutBasedIDValues["POGLoose"] = (int) (mu->isLoosePOGMuon() && mu->relIso0p4()<0.25);
                cutBasedIDValues["POGMedium"] = (int) (mu->isTightPOGMuon() && mu->relIso0p4()<0.2);
                cutBasedIDValues["POGTight"] = (int) (mu->isTightPOGMuon() && mu->relIso0p4()<0.15);
		// also add working points of lepton MVAs
                cutBasedIDValues["TOPMedium"] = (int) (mu->leptonMVATOP()>0.4);
                cutBasedIDValues["TOPTight"] = (int) (mu->leptonMVATOP()>0.9);
	    }

	    // if lepton does not pass additional selections, put MVA scores to minimum value
	    /*if( lep->isMuon() ){
		if( fabs( mu->dxy() ) > 0.05
		    || fabs( mu->dz() ) > 0.1
		    || mu->sip3d() > 8
		    || mu->miniIso() > 0.4
		    || !mu->isMediumPOGMuon() ){
		    mvaValues["TTH"] = -1+1e-6;
		    mvaValues["TZQ"] = -1+1e-6;
		    mvaValues["TOP"] = -1+1e-6;
		}
	    }
	    if( lep->isElectron() ){
                if( fabs( ele->dxy() ) > 0.05 
                    || fabs( ele->dz() ) > 0.1
		    || ele->sip3d() > 8
                    || ele->miniIso() > 0.4 
                    || ele->numberOfMissingHits() >= 2 ){
                    mvaValues["TTH"] = -1+1e-6;
                    mvaValues["TZQ"] = -1+1e-6;
                    mvaValues["TOP"] = -1+1e-6;
                }
            }*/
	
	    // fill the MVA and cut-based ID histograms for this lepton
	    for( std::string mvaName : mvaNames ){
		for( std::string pttag : pttags ){
		    mvaScoresMap[mvaName+"_"+pnp+"_"+flavour+"_"+pttag]->Fill( mvaValues[mvaName], eventWeight );
		}	
	    }
            for( std::string cutBasedIDName : cutBasedIDNames ){
                for( std::string pttag : pttags ){
                    cutBasedIDMap[cutBasedIDName+"_"+pnp+"_"+flavour+"_"+pttag]->Fill( 
                      cutBasedIDValues[cutBasedIDName], eventWeight );
                }
            }
	}
    }

    // write output file
    TFile* outputFilePtr = TFile::Open( pathToOutputFile.c_str() , "RECREATE" );
    for( std::string instance : mvaInstances ){ mvaScoresMap[instance]->Write(); }
    for( std::string instance : cutBasedIDInstances ){ cutBasedIDMap[instance]->Write(); }
    outputFilePtr->Close();
}

int main( int argc, char* argv[] ){
    std::cerr << "###starting###" << std::endl;
    if( argc < 4 ){
        std::cerr << "ERROR: wrong number of command line arguments." << std::endl;
        std::cerr << "Need <pathToInputFile> <pathToOutputFile> <nEntries>" << std::endl;
        return -1;
    }
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    std::string& input_file_path = argvStr[1];
    std::string& output_file_path = argvStr[2];
    long nentries = std::stol(argvStr[3]);
    
    makeMVAScoreHistograms( input_file_path, output_file_path, nentries );
    
    std::cerr << "###done###" << std::endl;
    return 0;
}
