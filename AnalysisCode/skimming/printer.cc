// run on an unskimmed file and print some values for debugging or synchronization

//include c++ library classes 
#include <string>
#include <vector>
#include <exception>
#include <iostream>

//include ROOT classes 
#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"

//include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Tools/interface/stringTools.h"
#include "../../Event/interface/Event.h"


void readFile( const std::string& pathToFile, unsigned long nentries){

    std::cout << "running on " << pathToFile << std::endl;

    //initialize TreeReader, input files might be corrupt in rare cases
    TreeReader treeReader;
    try{
        treeReader.initSampleFromFile( pathToFile );
    } catch( std::domain_error& ){
        std::cerr << "Can not read file. Returning." << std::endl;
        return;
    }

    if(nentries < 1) nentries = treeReader.numberOfEntries();
    for( long unsigned entry = 0; entry < nentries; ++entry ){

        // build event
        Event event = treeReader.buildEvent( entry, true, true );

	std::cout<<"==== event ====="<<std::endl;
	
	// event id
	std::cout<<"run nb, lumiblock, event nb: "<<event.runNumber()<<", "<<event.luminosityBlock()<<", "<<event.eventNumber()<<std::endl;

	// top mva of leptons in event
	/*for(LeptonCollection::const_iterator lIt = event.leptonCollection().cbegin();
            lIt != event.leptonCollection().cend(); lIt++){
	    std::shared_ptr<Lepton> lep = *lIt;
	    if(lep->isElectron()){
		std::shared_ptr<Electron> ele = std::static_pointer_cast<Electron>(lep);
		std::cout<<"electron with mva value "<<ele->leptonMVATOP()<<std::endl;
		std::cout<<"  input variables:"<<std::endl;
		std::cout<<"    log(dxy): "<<std::log(std::fabs(ele->dxy()))<<std::endl;
		std::cout<<"    miniIsoCharged: "<<ele->miniIsoCharged()<<std::endl;
		std::cout<<"    miniIsoNeutral: "<<ele->miniIsoNeutral()<<std::endl;
		std::cout<<"    ptRel: "<<ele->ptRel()<<std::endl;
		std::cout<<"    sip3d: "<<ele->sip3d()<<std::endl;
		std::cout<<"    ptRatio: "<<ele->ptRatio()<<std::endl;
		std::cout<<"    bTagDeepJetClosestJet: "<<ele->closestJetDeepFlavor()<<std::endl;
		std::cout<<"    pt: "<<ele->pt()<<std::endl;
		std::cout<<"    trackMultClosestJet: "<<ele->closestJetTrackMultiplicity()<<std::endl;
		std::cout<<"    etaAbs: "<<std::fabs(ele->eta())<<std::endl;
		std::cout<<"    log(dz): "<<std::log(std::fabs(ele->dz()))<<std::endl;
		std::cout<<"    relIso: "<<ele->relIso0p3()<<std::endl;
		std::cout<<"    mvaidFall17v2noIso"<<ele->electronMVAFall17NoIso()<<std::endl;
	    }
	    else if(lep->isMuon()){
		std::shared_ptr<Muon> mu = std::static_pointer_cast<Muon>(lep);
		std::cout<<"muon with mva value "<<mu->leptonMVATOP()<<std::endl;
		std::cout<<"  input variables:"<<std::endl;
                std::cout<<"    log(dxy): "<<std::log(std::fabs(mu->dxy()))<<std::endl;
                std::cout<<"    miniIsoCharged: "<<mu->miniIsoCharged()<<std::endl;
                std::cout<<"    miniIsoNeutral: "<<mu->miniIsoNeutral()<<std::endl;
                std::cout<<"    ptRel: "<<mu->ptRel()<<std::endl;
                std::cout<<"    sip3d: "<<mu->sip3d()<<std::endl;
                std::cout<<"    ptRatio: "<<mu->ptRatio()<<std::endl;
                std::cout<<"    bTagDeepJetClosestJet: "<<mu->closestJetDeepFlavor()<<std::endl;
                std::cout<<"    pt: "<<mu->pt()<<std::endl;
                std::cout<<"    trackMultClosestJet: "<<mu->closestJetTrackMultiplicity()<<std::endl;
                std::cout<<"    etaAbs: "<<std::fabs(mu->eta())<<std::endl;
                std::cout<<"    dz: "<<std::log(std::fabs(mu->dz()))<<std::endl;
                std::cout<<"    relIso: "<<mu->relIso0p3()<<std::endl;
		std::cout<<"    segmentCompatibility: "<<mu->segmentCompatibility()<<std::endl;
	    }
	}*/
	// jet properties
	for(JetCollection::const_iterator jIt = event.jetCollection().cbegin();
            jIt != event.jetCollection().cend(); jIt++){
	    std::shared_ptr<Jet> jet = *jIt;
	    std::cout << "PU ID loose " << jet->passPileupIdLoose() << std::endl;
	    std::cout << "PU ID medium " << jet->passPileupIdMedium() << std::endl;
	    std::cout << "PU ID tight " << jet->passPileupIdTight() << std::endl;
	    std::cout << "has gen jet " << jet->hasGenJet() << std::endl;
	}
    }
}

int main( int argc, char* argv[] ){
    if( argc != 3 ){
        std::cerr << "skimmer requires exactly two arguments to run : input_file_path, nentries" << std::endl;
        return -1;
    }

    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );

    std::string& input_file_path = argvStr[1];
    unsigned long nentries = std::stoul(argvStr[2]);
    readFile( input_file_path, nentries );
    std::cout<<"done"<<std::endl;
    return 0;
}
