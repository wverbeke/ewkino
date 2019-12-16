#include "../interface/fakeRateSelection.h"

//include c++ library classes 
#include <stdexcept>
#include <map>

//include other parts of framework
#include "../../Tools/interface/stringTools.h"
#include "../../Event/interface/Event.h"


bool isMuonTrigger( const std::string& triggerPath ){
    return stringTools::stringContains( triggerPath, "Mu" );
}


bool isElectronTrigger( const std::string& triggerPath ){
    return stringTools::stringContains( triggerPath, "Ele" );
}


double extractPtCut( const std::string& triggerPath, const std::string& objectIdentifier ){

    //check that the requested object identifier is present
    if( !stringTools::stringContains( triggerPath, objectIdentifier ) ){
		throw std::invalid_argument( "object identifier " + objectIdentifier + " not found in trigger path " + triggerPath );
	}

    std::string cutString = stringTools::split( stringTools::split( triggerPath, objectIdentifier ).back(), "_" ).front();
	return std::stod( cutString );
}


double extractLeptonPtCut( const std::string& triggerPath ){
    std::string flavorIdentifier;
    if( isMuonTrigger( triggerPath ) ){
        flavorIdentifier = "Mu";
    } else if( isElectronTrigger( triggerPath ) ){
        flavorIdentifier = "Ele";
    } else {
        throw std::invalid_argument( "can not extract electron or muon pT threshold from trigger path " + triggerPath + "." );
    }

    //extract pt threshold string 
	return extractPtCut( triggerPath, flavorIdentifier );
}


double extractJetPtCut( const std::string& triggerPath ){
	return extractPtCut( triggerPath, "PFJet" );
}


double muonPtThreshold( double triggerThreshold ){
    static constexpr double offset = 1.; //trigger plateau reached 1 GeV above the threshold 
    return ( triggerThreshold + offset );
}


double electronPtThreshold( double triggerThreshold ){
    static constexpr double offset = 2.; //trigger plateau reached about 3 GeV above the threshold
    return ( triggerThreshold + offset );
}


double jetPtThreshold( double triggerThreshold ){
    static constexpr double offset = 10.;
    return ( triggerThreshold + offset ); 
}


std::map< std::string, double > fakeRate::mapTriggerToLeptonPtThreshold( const std::vector< std::string >& triggerNames ){

    //map each trigger to their pT cut + offset to reach plateau
    std::map< std::string, double > cutMap;
    for( const auto& trigger : triggerNames ){
        if( isMuonTrigger( trigger ) ){
            cutMap[ trigger ] = muonPtThreshold( extractLeptonPtCut( trigger ) );
        } else if( isElectronTrigger( trigger ) ){
            cutMap[ trigger ] = electronPtThreshold( extractLeptonPtCut( trigger ) );
        } else {
            throw std::invalid_argument( "Can't make pT threshold for trigger '" + trigger + "' since it is neither a muon nor electron trigger" );
        }
    }
    return cutMap;
}


std::map< std::string, double > fakeRate::mapTriggerToJetPtThreshold( const std::vector< std::string >& triggerNames ){
    
    std::map< std::string, double > cutMap; 
    for( const auto& trigger : triggerNames ){
        if( !stringTools::stringContains( trigger, "PFJet" ) ) continue;
        cutMap[ trigger ] = jetPtThreshold( extractJetPtCut( trigger ) );
    }
    return cutMap;
}
    

//event selection for fake-rate measurement 
bool fakeRate::passFakeRateEventSelection( Event& event, bool onlyMuons, bool onlyElectrons, bool onlyTightLeptons, bool requireJet, double jetDeltaRCut ){

    //ignore taus here since we are measuring light lepton fake-rate
    event.removeTaus();

    //Require the presence of just one lepton, and veto a second loose lepton
    event.cleanElectronsFromLooseMuons();
    event.selectLooseLeptons();
    if( event.numberOfLightLeptons() != 1 ){
        return false;
    }

    //Select FO or Tight leptons 
    if( onlyTightLeptons ){
        event.selectTightLeptons();
    } else {
        event.selectFOLeptons();
    }
    if( event.numberOfLightLeptons() != 1 ){
        return false;
    }

    //IMPORTANT : apply cone correction
    event.applyLeptonConeCorrection();

    Lepton& lepton = event.lightLepton( 0 );

    //select correct lepton flavor
    if( onlyMuons && !lepton.isMuon() ) return false;
    else if( onlyElectrons && !lepton.isElectron() ) return false;

    //apply pT thresholds on leptons 

    //optionally require the presence of at least one good jet
    if( requireJet ){
        event.selectGoodJets();
        event.cleanJetsFromLooseLeptons();
        if( event.numberOfJets() < 1 ) return false;

        //require deltaR of at least 1 between lepton and any of the selected jets 
        double maxDeltaR = 0;
        for( auto jetPtr : event.jetCollection() ){
            double currentDeltaR = deltaR( lepton, *jetPtr );
            if( currentDeltaR > maxDeltaR ){
                maxDeltaR = currentDeltaR;
            }
        }
        if( maxDeltaR < jetDeltaRCut ) return false;
    }
    return true;
}



RangedMap< std::string > fakeRate::mapLeptonPtToTriggerName( const std::vector< std::string >& triggerNames, const bool isMuon ){

    //make an std::map that maps the lower bounds of the RangedMap to strings, this will be used to initialize the RangedMap
    std::map< double, std::string > initializerMap; 
    for( const auto& trigger : triggerNames ){

        //check that the trigger contains the correct lepton flavor
        if( isMuon && !isMuonTrigger( trigger ) ) continue;
        if( !isMuon && !isElectronTrigger( trigger ) ) continue;

        //add trigger pT cut to map, and make sure it is above the plateau by calling one of the 'PtThreshold' functions
        double threshold;
        if( isMuon ){
            threshold = muonPtThreshold( extractLeptonPtCut( trigger ) );
        } else {
            threshold = electronPtThreshold( extractLeptonPtCut( trigger ) );
        }
        initializerMap[ threshold ] = trigger;
    }

    return RangedMap< std::string >( initializerMap );
}


RangedMap< std::string > fakeRate::mapMuonPtToTriggerName( const std::vector< std::string >& triggerNames ){
    return fakeRate::mapLeptonPtToTriggerName( triggerNames, true );
}


RangedMap< std::string > fakeRate::mapElectronPtToTriggerName( const std::vector< std::string >& triggerNames ){
    return fakeRate::mapLeptonPtToTriggerName( triggerNames, false );
}


bool fakeRate::passTriggerJetSelection( Event& event, const std::string& trigger, std::map< std::string, double >& triggerToJetPtMap ){
    if( !stringTools::stringContains( trigger, "PFJet" ) ){
        return true;
    } else{
		event.selectGoodJets();
		event.cleanJetsFromLooseLeptons();
		if( event.numberOfJets() < 1 ) return false; 
       	event.sortJetsByPt();

		//require central jet 
		if( event.jet(0).absEta() >= 2.4 ) return false;

		//apply offline pT threshold to be on the trigger plateau
        if( event.jet(0).pt() <= triggerToJetPtMap[ trigger ] ) return false;
		
		return true;
	}
}
