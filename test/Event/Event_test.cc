
//include class to test 
#include "../../TreeReader/interface/TreeReader.h"
//#include "../../objects/interface/PhysicsObject.h"
//#include "../../objects/interface/Lepton.h"
//#include "../../objects/interface/LightLepton.h"
//#include "../../objects/interface/Muon.h"
//#include "../../objects/interface/Electron.h"
//#include "../../objects/interface/Tau.h"
//#include "../../objects/interface/Met.h"
//#include "../../Event/interface/LeptonCollection.h"
//#include "../../Event/interface/JetCollection.h"
//#include "../../Event/interface/MuonCollection.h"
//#include "../../Event/interface/ElectronCollection.h"
//#include "../../Event/interface/TauCollection.h"
//#include "../../Event/interface/TriggerInfo.h"
//#include "../../Event/interface/GeneratorInfo.h"
#include "../../Event/interface/Event.h"
#include "../copyMoveTest.h"

//include c++ library classes
#include <iostream>
#include <chrono>
#include <utility>
#include <vector>


int main(){
    TreeReader treeReader;
    treeReader.readSamples("../testData/samples_test.txt", "../testData");

    treeReader.initSample();

    auto start = std::chrono::high_resolution_clock::now();

    std::chrono::duration< double > getEntryTimer( 0. );
    std::chrono::duration< double > buildEventTimer( 0. );

    for(unsigned i = 0; i < treeReader.numberOfEntries(); ++i){

        auto getEntry_begin = std::chrono::high_resolution_clock::now();
        treeReader.GetEntry(i);
        auto getEntry_end = std::chrono::high_resolution_clock::now();
        getEntryTimer += ( getEntry_end - getEntry_begin );

        auto eventBuild_begin = std::chrono::high_resolution_clock::now();

        Event event( treeReader, true, true);

       	event.leptonCollection();
        event.jetCollection();
        event.met();
        event.triggerInfo();
        event.eventTags();
        event.generatorInfo();

		for( LeptonCollection::size_type leptonIndex = 0; leptonIndex < event.numberOfLeptons(); ++leptonIndex ){
        	event.lepton( leptonIndex );
		}

		for( JetCollection::size_type jetIndex = 0; jetIndex < event.numberOfJets(); ++jetIndex ){
			event.jet( jetIndex );
        }

        event.sortLeptonsByPt();
        event.sortJetsByPt();

        event.numberOfVertices();
        event.weight();

        event.HT();
        event.LT();
        event.metPt();

        //jet selection and cleaning
        event.selectGoodJets();
        event.cleanJetsFromLooseLeptons();
        event.cleanJetsFromFOLeptons();
        event.cleanJetsFromTightLeptons();

        //b-tag collections
        event.looseBTagCollection(); 
        event.mediumBTagCollection();
        event.tightBTagCollection();

        //lepton selection and cleaning
        event.applyLeptonConeCorrection();
        event.selectLooseLeptons();
        event.selectFOLeptons();
        event.selectTightLeptons();
        event.cleanElectronsFromLooseMuons();
        event.cleanElectronsFromFOMuons();
        event.cleanTausFromLooseLightLeptons();
        event.cleanTausFromFOLightLeptons();

		//separate lepton flavor collections
        event.muonCollection();
        event.electronCollection();
        event.tauCollection();

        //lepton collections based on selection
        event.looseLeptonCollection();
        event.FOLeptonCollection();
        event.TightLeptonCollection();

        //Trigger information
        event.passTriggers_e();
        event.passTriggers_m();
        event.passTriggers_ee();
        event.passTriggers_em();
        event.passTriggers_et();
        event.passTriggers_mm();
        event.passTriggers_mt();
        event.passTriggers_eee();
        event.passTriggers_eem();
        event.passTriggers_emm();
        event.passTriggers_mmm();
        event.passMetFilters();
		
        //event.passTrigger( const std::string& triggerName ) const{ return _triggerInfoPtr->passTrigger( triggerName ); }
        //event.passMetFilter( const std::string& filterName ) const{ return _triggerInfoPtr->passMetFilter( filterName ); }


        //number of leptons 
        event.numberOfLeptons();
        event.isSinglelepton();
        event.isDilepton();
        event.isTrilepton();
        event.isFourLepton();

        //lepton flavor and charge combinations
        event.hasOSSFLeptonPair();
        event.hasOSSFLightLeptonPair();
        event.hasOSLeptonPair();
        event.leptonsAreSameSign();
        event.numberOfUniqueOSSFLeptonPairs();
        event.numberOfUniqueOSLeptonPairs();


        //presence of a Z boson
        //event.hasZTollCandidate( const double massWindow );
        //event.bestZBosonCandidateMass();
        //event.bestZBosonCandidateIndices();
        //event.bestZBosonCandidateIndicesAndMass();

        //transverse mass of lepton from W decay in 1 or 3 lepton events and the MET
        //event.WLeptonIndex();
        //event.mtW();

		for( LeptonCollection::size_type leptonIndex = 0; leptonIndex < event.numberOfLeptons(); ++ leptonIndex ){
       		event.mtLeptonMet( leptonIndex );
		}

        //number of jets 
        event.numberOfJets();
        event.numberOfGoodJets();

        //number of b-tagged jets
        event.numberOfLooseBTaggedJets();
        event.numberOfMediumBTaggedJets();
        event.numberOfTightBTaggedJets();
        event.hasLooseBTaggedJet();
        event.hasMediumBTaggedJet();
        event.hasTightBTaggedJet();

        copyMoveTest( event );

        auto eventBuild_end = std::chrono::high_resolution_clock::now();
        buildEventTimer += ( eventBuild_end - eventBuild_begin );

    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time for looping over sample : " << elapsed.count() << " s\n";

    std::cout << "Time spent on GetEntry = " << getEntryTimer.count() << " s\n";
    std::cout << "Time spent on Event building = " << buildEventTimer.count() << " s\n";
    return 0;
}
