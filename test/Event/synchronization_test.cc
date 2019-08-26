
//include c++ library classes 
#include <fstream>
#include <utility>
#include <cmath>

//include analysis framework
#include "../../Event/interface/Event.h"
#include "../../objects/interface/Lepton.h"

//ROOT classes to use in standalone test code 
#include "TLorentzVector.h"


void testCopyMove(){
	TreeReader treeReader;
    treeReader.readSamples("../testData/samples_test.txt", "../testData");

    treeReader.initSample();

    std::ofstream original_dump( "original_dump.txt" );
    std::ofstream event_dump( "event_dump.txt" );
    std::ofstream copyConstructor_dump( "copyConstructor_dump.txt" );
    std::ofstream moveConstructor_dump( "moveConstructor_dump.txt" );
    std::ofstream copyAssignment_dump("copyAssignment_dump.txt" );
    std::ofstream moveAssignment_dump( "moveAssignment_dump.txt" );

    for(unsigned i = 0; i < treeReader.nEntries; ++i){

        treeReader.GetEntry(i);

        //dump leptons produced without any collection classes
        for( unsigned m = 0; m < treeReader._nMu; ++m ){
            original_dump << Muon( treeReader, m ) << std::endl;
        }
        for( unsigned e = treeReader._nMu; e < treeReader._nLight; ++e ){
            original_dump << Electron( treeReader, e ) << std::endl;
        }
        for( unsigned t = treeReader._nLight; t < treeReader._nL; ++t ){
            original_dump << Tau( treeReader, t ) << std::endl;
        }
        for( unsigned j = 0; j < treeReader._nJets; ++j ){
            original_dump << Jet( treeReader, j ) << std::endl;
        }
        original_dump << Met( treeReader ) << std::endl;

		//build event
        Event event( treeReader, true, true);

        for( LeptonCollection::size_type leptonIndex = 0; leptonIndex < event.numberOfLeptons(); ++leptonIndex ){

            event_dump << event.lepton( leptonIndex ) << std::endl;
            
            if( event.lepton( leptonIndex ).isMuon() ){

                //copy constructor 
                Muon copyConstructedLepton( (const Muon&) event.lepton( leptonIndex ) ); 
                copyConstructor_dump << copyConstructedLepton << std::endl;

                //move constructor
                Muon moveConstructedLepton( std::move( copyConstructedLepton ) );
                moveConstructor_dump << moveConstructedLepton << std::endl;

                //copy assignment 
                copyConstructedLepton = moveConstructedLepton;
                copyAssignment_dump << copyConstructedLepton << std::endl;

                //move assignment
                copyConstructedLepton = std::move( moveConstructedLepton );
                moveAssignment_dump << copyConstructedLepton << std::endl;

            } else if( event.lepton( leptonIndex ).isElectron() ){
                
                //copy constructor
                Electron copyConstructedLepton( (const Electron&) event.lepton( leptonIndex ) );
                copyConstructor_dump << copyConstructedLepton << std::endl;

                //move constructor
                Electron moveConstructedLepton( std::move( copyConstructedLepton ) );
                moveConstructor_dump << moveConstructedLepton << std::endl;

                //copy assignment 
                copyConstructedLepton = moveConstructedLepton;
                copyAssignment_dump << copyConstructedLepton << std::endl;

                //move assignment
                copyConstructedLepton = std::move( moveConstructedLepton );
                moveAssignment_dump << copyConstructedLepton << std::endl;

            } else {

                //copy constructor
                Tau copyConstructedLepton( (const Tau&) event.lepton( leptonIndex ) );
                copyConstructor_dump << copyConstructedLepton << std::endl;

                //move constructor
                Tau moveConstructedLepton( std::move( copyConstructedLepton ) );
                moveConstructor_dump << moveConstructedLepton << std::endl;

                //copy assignment 
                copyConstructedLepton = moveConstructedLepton;
                copyAssignment_dump << copyConstructedLepton << std::endl;

                //move assignment
                copyConstructedLepton = std::move( moveConstructedLepton );
                moveAssignment_dump << copyConstructedLepton << std::endl;
            }
        }

        for( JetCollection::size_type jetIndex = 0; jetIndex < event.numberOfJets(); ++jetIndex ){
            event_dump << event.jet( jetIndex ) << std::endl;
            
            //copy constructor
            Jet copyConstructedJet( event.jet( jetIndex ) );
            copyConstructor_dump << copyConstructedJet << std::endl;

            //move constructor
            Jet moveConstructedJet( std::move( copyConstructedJet ) );
            moveConstructor_dump << moveConstructedJet << std::endl;

            //copy assignment
            copyConstructedJet = moveConstructedJet;
            copyAssignment_dump << copyConstructedJet << std::endl;
    
            //move assignment
            copyConstructedJet = std::move( moveConstructedJet );
            moveAssignment_dump << copyConstructedJet << std::endl;

        }

        event_dump << event.met() << std::endl;
        
        //copy constructor 
        Met copyConstructedMet( event.met() );
        copyConstructor_dump << copyConstructedMet << std::endl;
        
        //move constructor 
        Met moveConstructedMet( std::move( copyConstructedMet ) );
        moveConstructor_dump << moveConstructedMet << std::endl;

        //copy assignment
        copyConstructedMet = moveConstructedMet;
        copyAssignment_dump << copyConstructedMet << std::endl;

        //move assignment 
        copyConstructedMet = std::move( moveConstructedMet );
        moveAssignment_dump << copyConstructedMet << std::endl;
    }
    original_dump.close();
    event_dump.close();
    copyConstructor_dump.close();
    moveConstructor_dump.close();
    copyAssignment_dump.close();
    moveAssignment_dump.close();
}


void makeDumpWithFrameWork(){
	TreeReader treeReader;
    treeReader.readSamples("../testData/samples_test.txt", "../testData");

    treeReader.initSample();
    std::cout << treeReader.is2018() << std::endl;

	std::ofstream dump( "framework_dump.txt" );

    for(unsigned i = 0; i < treeReader.nEntries; ++i){
        treeReader.GetEntry(i);	
		Event event( treeReader, true, true);

        /*
        bool isEvent = false;
        if( event.eventTags() == EventTags( {1,224,193549} ) ){  
            std::cout << "EVENT FOUND" << std::endl;
            isEvent = true;
        } else {
            isEvent = false;
        }
        if( !isEvent ) continue;
        */

		//lets select events with at least 2 jets, 1 medium b -jet , 2 tight OSSF leptons 
        event.applyLeptonConeCorrection();
        event.sortLeptonsByPt();
        event.cleanElectronsFromLooseMuons();
        event.cleanTausFromLooseLightLeptons();
		event.selectGoodJets();
        event.cleanJetsFromLooseLeptons();
		if( event.numberOfJets() < 2 ) continue;
		if( !event.hasMediumBTaggedJet() ) continue;

		event.selectTightLeptons();
        /*
        if( isEvent ){
            for( LeptonCollection::size_type l = 0; l < event.numberOfLeptons(); ++l){
                std::cout << event.lepton( l ) << " / ISTIGHT = " << event.lepton( l ).isTight() << std::endl;
            }
        }
        */

        //if( event.numberOfMuons() != event.numberOfLeptons() ) continue; 
		if( event.numberOfLeptons() < 2 ) continue;
    

		//if( ! event.hasOSSFLeptonPair() ) continue;
        PhysicsObject leptonSystem = event.lepton(0);
        for( LeptonCollection::size_type l = 1; l < event.numberOfLeptons(); ++l ){
            leptonSystem += event.lepton( l );
        }

		dump << event.eventTags() << "\tnumber of b tags = " << event.numberOfMediumBTaggedJets()
        << "\tnumber of jets = " << event.numberOfJets() 
        << "\tnumber of muons = " << event.numberOfMuons()
        << "\tnumber of electrons = " << event.numberOfElectrons()
        << "\tnumber of taus = " << event.numberOfTaus()
        << "\tnumber of OSSF pairs = " << event.numberOfUniqueOSSFLeptonPairs()
        << "\tnumber of OS pairs = " << event.numberOfUniqueOSLeptonPairs()
        << "\tHT = " << event.HT() 
        << "\tLT = " << event.LT()
        << "\tmet = " << event.metPt()
        << "\tleading lepton pT = " << event.lepton( 0 ).pt()
        << "\tmll = " << ( event.lepton(0) + event.lepton(1) ).mass() 
        << "\tdeltaR = " << deltaR( event.lepton(0), event.lepton(1) )
        << "\tdeltaPhi = " << deltaPhi( event.lepton(0), event.lepton(1) )
        << "\tdeltaEta = " << deltaEta( event.lepton(0), event.lepton(1) )
        << "\tmtl1met = " << mt( event.lepton(0), event.met() )
        << "\tmtl2met = " << mt( event.lepton(1), event.met() )
        << "\tmlep = " << leptonSystem.mass()
        << "\tptlep = " << leptonSystem.pt()
        << "\tdeltaRLepMet = " << deltaR( leptonSystem, event.met() )
        << "\tdeltaPhiLepMet = " << deltaPhi( leptonSystem, event.met() )
        << std::endl;
    }
    dump.close();
}


void makeDumpStandalone(){
	TreeReader treeReader;
    treeReader.readSamples("../testData/samples_test.txt", "../testData");

    treeReader.initSample();

    std::ofstream dump( "standalone_dump.txt" );
    std::cout << treeReader.is2018() << std::endl;


    for(unsigned i = 0; i < treeReader.nEntries; ++i){
	
        treeReader.GetEntry(i);

        //if( treeReader._eventNb != 193549 ) continue;


        unsigned numberOfTightLeptons = 0;
        unsigned numberOfMuons = 0;
        unsigned numberOfElectrons = 0;
        unsigned numberOfTaus = 0;
        double LT = 0;
        std::vector< unsigned > ind;
        std::vector< unsigned > looseInd;
        for( unsigned m = 0; m < treeReader._nMu; ++m ){
            if( treeReader._lPt[m] < 5 ) continue;
            if( fabs( treeReader._lEta[m] ) >= 2.4 ) continue;
            if( !treeReader._lPOGLoose[m] ) continue;
            looseInd.push_back( m );

            //WEIRD: there are Tight muons that are not medium or loose
            if( !treeReader._lPOGTight[m] ) continue;
            if( !treeReader._lPOGMedium[m] ) continue;

            ind.push_back( m );
            ++numberOfTightLeptons;
            ++numberOfMuons;

            LT += treeReader._lPt[m];
        }
        for( unsigned e = treeReader._nMu; e < treeReader._nLight; ++e ){
            if( treeReader._lPt[e] < 10 ) continue;
            if( fabs( treeReader._lEta[e] ) >= 2.5 ) continue; 
            if( !( treeReader._lElectronPassEmu[e] ) ) continue;

            bool eleIsClean = true;
            TLorentzVector ele;
            ele.SetPtEtaPhiE( treeReader._lPt[e], treeReader._lEta[e], treeReader._lPhi[e], treeReader._lE[e] );
            for( auto m : looseInd ){
                if( treeReader._lFlavor[m] != 1 ) continue;
                TLorentzVector mu;
                mu.SetPtEtaPhiE( treeReader._lPt[m], treeReader._lEta[m], treeReader._lPhi[m], treeReader._lE[m] );

                if( ele.DeltaR( mu ) < 0.05 ){
                    eleIsClean = false;
                    break;
                }
            }
            if( !eleIsClean ) continue;
            looseInd.push_back( e );
            if( !( treeReader._lPOGTight[e] ) ) continue;
            ind.push_back( e );
            ++numberOfTightLeptons;
            ++numberOfElectrons;
            LT += treeReader._lPt[e];
        }

        for( unsigned t = treeReader._nLight; t < treeReader._nL; ++t ){ 
            if( treeReader._lPt[t] < 20 ) continue;
            if( fabs( treeReader._lEta[t] ) >= 2.3 ) continue;
            if( ! treeReader._decayModeFinding[t] ) continue;
            if( ! treeReader._lPOGVeto[t] ) continue;

            bool tauIsClean = true;
            TLorentzVector tau;
            tau.SetPtEtaPhiE( treeReader._lPt[t], treeReader._lEta[t], treeReader._lPhi[t], treeReader._lE[t] );
            for( auto l : looseInd ){
                if( treeReader._lFlavor[l] == 2 ) continue;
                TLorentzVector lep;
                lep.SetPtEtaPhiE( treeReader._lPt[l], treeReader._lEta[l], treeReader._lPhi[l], treeReader._lE[l] );
                if( tau.DeltaR( lep ) < 0.4 ){
                    tauIsClean = false;
                    break;
                }
            }
            if( !tauIsClean ) continue;

            looseInd.push_back( t );
            //if( ! treeReader._lPOGMedium[t] ) continue;
            if( ! treeReader._tauPOGVTight2017v2[t] ) continue;
            ind.push_back( t );
            ++numberOfTightLeptons;
            ++numberOfTaus;
            LT += treeReader._lPt[t];
        }

        if( numberOfTightLeptons < 2 ) continue;

        unsigned numberOfOSSFPairs = 0;
        std::set< unsigned > usedIndices;
        for( unsigned l1 = 0; l1 < numberOfTightLeptons - 1; ++l1 ){

            if( usedIndices.find( l1 ) != usedIndices.end() ) continue;
            for( unsigned l2 = l1 + 1; l2 < numberOfTightLeptons; ++l2 ){
                if( usedIndices.find( l2 ) != usedIndices.end() ) continue;
                if( treeReader._lFlavor[ ind[l1] ] != treeReader._lFlavor[ ind[l2] ] ) continue;
                if( treeReader._lCharge[ ind[l1] ] == treeReader._lCharge[ ind[l2] ] ) continue; 
                usedIndices.insert( l1 );
                usedIndices.insert( l2 );
                ++numberOfOSSFPairs;
                break;
            }
        }
        
        unsigned numberOfOSPairs = 0;
        usedIndices.clear();
        for( unsigned l1 = 0; l1 < numberOfTightLeptons - 1; ++l1 ){

            if( usedIndices.find( l1 ) != usedIndices.end() ) continue;
            for( unsigned l2 = l1 + 1; l2 < numberOfTightLeptons; ++l2 ){
                if( usedIndices.find( l2 ) != usedIndices.end() ) continue;
                if( treeReader._lCharge[ ind[l1] ] == treeReader._lCharge[ ind[l2] ] ) continue;
                usedIndices.insert( l1 );
                usedIndices.insert( l2 );
                ++numberOfOSPairs;
                break;
            }
        }

        //order lepton indices by pt
        std::vector< std::pair< unsigned, double > > indPt; 
        for( auto l : ind ){
            indPt.push_back( {l, treeReader._lPt[l]} );
        }
        std::sort( indPt.begin(), indPt.end(), []( const std::pair< unsigned, double >& lhs, const std::pair< unsigned, double >& rhs ){ return lhs.second > rhs.second; } );
        for( unsigned l = 0; l < indPt.size(); ++l ){
            ind[l] = indPt[l].first;
        }


        //if( numberOfOSSFPairs < 1 ) continue;

		unsigned numberOfGoodJets = 0;	
		unsigned numberOfMediumBJets = 0;
        double HT = 0;

		for( unsigned j = 0; j < treeReader._nJets; ++j ){
			if( treeReader._jetPt[j] < 25 ) continue;
            if( fabs( treeReader._jetEta[j] ) > 5 ) continue;
            if( !treeReader._jetIsTight[j] ) continue;
    
            bool jetIsClean = true;

            TLorentzVector Jet;
            Jet.SetPtEtaPhiE( treeReader._jetPt[j], treeReader._jetEta[j], treeReader._jetPhi[j], treeReader._jetE[j] );
            for( auto l : looseInd ){
                
                TLorentzVector Lepton;
                Lepton.SetPtEtaPhiE( treeReader._lPt[l], treeReader._lEta[l], treeReader._lPhi[l], treeReader._lE[l] );
                if( Lepton.DeltaR( Jet ) < 0.4 ){
                    jetIsClean = false;
                    break;
                }
            }

            if( !jetIsClean ) continue;


            ++numberOfGoodJets;
            if( ( ( treeReader._jetDeepCsv_b[j] + treeReader._jetDeepCsv_bb[j] ) > ( treeReader.is2018() ? 0.4184 : 0.8838 ) ) && ( fabs( treeReader._jetEta[j] ) < 2.4 ) ){
                ++numberOfMediumBJets;
            }
            HT += treeReader._jetPt[j];
		}
        if( numberOfGoodJets < 2 ) continue;
        if( numberOfMediumBJets < 1 ) continue;

		TLorentzVector l1Vec;
		l1Vec.SetPtEtaPhiE( treeReader._lPt[ind[0]], treeReader._lEta[ind[0]], treeReader._lPhi[ind[0]], treeReader._lE[ind[0]] );
		TLorentzVector l2Vec;
		l2Vec.SetPtEtaPhiE( treeReader._lPt[ind[1]], treeReader._lEta[ind[1]], treeReader._lPhi[ind[1]], treeReader._lE[ind[1]] );
		TLorentzVector metVec;
		metVec.SetPtEtaPhiE( treeReader._met, 0, treeReader._metPhi, treeReader._met );

        TLorentzVector leptonSystem( 0, 0, 0, 0 );
        for( auto l : ind ){
            TLorentzVector lep;
            lep.SetPtEtaPhiE( treeReader._lPt[l], treeReader._lEta[l], treeReader._lPhi[l], treeReader._lE[l] );
            leptonSystem += lep;
        }
    
        dump << "run number/luminosity block/event number : " << treeReader._runNb << "/" << treeReader._lumiBlock << "/" << treeReader._eventNb  << "\tnumber of b tags = " << numberOfMediumBJets
        << "\tnumber of jets = " << numberOfGoodJets
        << "\tnumber of muons = " << numberOfMuons
        << "\tnumber of electrons = " << numberOfElectrons
        << "\tnumber of taus = " << numberOfTaus
        << "\tnumber of OSSF pairs = " << numberOfOSSFPairs
        << "\tnumber of OS pairs = " << numberOfOSPairs
        << "\tHT = " << HT
        << "\tLT = " << LT
        << "\tmet = " << treeReader._met
        << "\tleading lepton pT = " << treeReader._lPt[ ind[0] ]
       	<< "\tmll = " << ( l1Vec + l2Vec ).M()
        << "\tdeltaR = " << l1Vec.DeltaR( l2Vec )
        << "\tdeltaPhi = " << fabs( l1Vec.DeltaPhi( l2Vec ) )
        << "\tdeltaEta = " << fabs( l1Vec.Eta() - l2Vec.Eta() )
        << "\tmtl1met = " << sqrt(2*l1Vec.Pt()*metVec.Pt()*( 1 - cos( l1Vec.Phi()-metVec.Phi() ) ) )
        << "\tmtl2met = " << sqrt(2*l2Vec.Pt()*metVec.Pt()*( 1 - cos( l2Vec.Phi()-metVec.Phi() ) ) )
        << "\tmlep = " << leptonSystem.M()
        << "\tptlep = " << leptonSystem.Pt()
        << "\tdeltaRLepMet = " << leptonSystem.DeltaR( metVec )
        << "\tdeltaPhiLepMet = " << fabs( leptonSystem.DeltaPhi( metVec ) )
        << std::endl;
	}
    dump.close();
}


int main(){

    std::cout << "copy test" << std::endl;
    testCopyMove();
    std::cout << "makeDumpWithFrameWork" << std::endl;
    makeDumpWithFrameWork();
    std::cout << "makeDumpStandalone" << std::endl;
    makeDumpStandalone();
    return 0;
}
