#include "../interface/fakeRateSelection.h"

//include c++ library classes 
#include <stdexcept>
#include <map>

//include other parts of framework
#include "../../Tools/interface/stringTools.h"
#include "../../Event/interface/Event.h"


double extractPtCut( const std::string& triggerPath, const std::string& objectIdentifier ){
	if( triggerPath.find( objectIdentifier ) == std::string::npos ){
		throw std::invalid_argument( "object identifier " + objectIdentifier + " not found in trigger path " + triggerPath );
	}
	auto cutBegin = triggerPath.find( objectIdentifier ) + objectIdentifier.size();
	auto cutEnd = triggerPath.find( "_", cutBegin );
	if( cutEnd == std::string::npos ){
		cutEnd = triggerPath.size();
	}
	std::string cutString = triggerPath.substr( cutBegin, cutEnd - cutBegin );
	
	return std::stod( cutString );
}


double extractLeptonPtCut( const std::string& triggerPath ){
    std::string flavorIdentifier;
    if( stringTools::stringContains( triggerPath, "Mu") ){
        flavorIdentifier = "Mu";
    } else if( stringTools::stringContains( triggerPath, "Ele" ) ){
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



double triggerObjectPtCut( const std::string& triggerPath, std::map< std::string, double >& cutMap, double (&extractCut)( const std::string& ) ){
    auto it = cutMap.find( triggerPath );
    if( it != cutMap.cend() ){
        return it->second;
    } else {
        double cut = extractCut( triggerPath );
        cutMap.insert( { triggerPath, cut } );
        return cut;
    }
}



double triggerLeptonPtCut( const std::string& triggerPath ){
    static std::map< std::string, double > ptCutMap;
    return triggerObjectPtCut( triggerPath, ptCutMap, extractLeptonPtCut );
}


double triggerJetPtCut( const std::string& triggerPath ){
    static std::map< std::string, double > ptCutMap;
    return triggerObjectPtCut( triggerPath, ptCutMap, extractJetPtCut );
}


std::string muonPtToTriggerName( const double pt ){
    if( pt < 8 ){
        return "HLT_Mu3_PFJet40";
    } else if( pt < 17 ){
        return "HLT_Mu8";
    } else if( pt < 20 ){
        return "HLT_Mu17";
    } else if( pt < 27 ){
        return "HLT_Mu20";
    } else if( pt < 50 ){
        return "HLT_Mu27";
    } else {
        return "HLT_Mu50";
    }
}


std::string electronPtToTriggerName( const double pt ){
    if( pt < 17 ){
        return "HLT_Ele8_CaloIdL_TrackIdL_IsoVL_PFJet30";
    } else if( pt < 23 ){
        return "HLT_Ele17_CaloIdM_TrackIdM_PFJet30";
    } else {
        return "HLT_Ele23_CaloIdM_TrackIdM_PFJet30";
    }
}


bool passFakeRateTrigger( const Event& event ){

    //leading selected lepton
    Lepton& lepton = event.lepton( 0 );

    //WARNING: There must be a ptRatio cut-off in the cone_correction 
    //IMPLEMENT MAXIMUM CONE CORRECTION BASED ON ISFO DEFINITION
    double ptRatioCutoff = 0.75;

    double maximumConeCorrection = 1./ptRatioCutoff;

    double effectivePt = lepton.pt()/maximumConeCorrection;

    std::string trigger_to_use = lepton.isMuon() ? muonPtToTriggerName( effectivePt ) : electronPtToTriggerName( effectivePt );
    return event.passTrigger( trigger_to_use );
}


//event selection for fake-rate measurement 
bool passFakeRateEventSelection( Event& event, bool isMuonMeasurement, bool onlyTightLeptons, bool requireJet, double jetDeltaRCut ){

    //Require the presence of just one lepton, and veto a second loose lepton
    event.cleanElectronsFromLooseMuons();
    event.cleanTausFromLooseLightLeptons();
    if( event.numberOfLooseLeptons() != 1 ){
        return false;
    }

    //Select FO or Tight leptons 
    if( onlyTightLeptons ){
        event.selectTightLeptons();
    } else {
        event.selectFOLeptons();
    }
    if( event.numberOfLeptons() != 1 ){
        return false;
    }

    //IMPORTANT : apply cone correction
    event.applyLeptonConeCorrection();

    Lepton& lepton = event.lepton( 0 );

    //select correct lepton flavor
    if( isMuonMeasurement ){
        if( !lepton.isMuon() ) return false;
    } else {
        if( !lepton.isElectron() ) return false;
    }

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
