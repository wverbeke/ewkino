#include "../interface/chargeFlipSelection.h"

//include other parts of framework
#include "../../objects/interface/Electron.h"
#include "../../Event/interface/Event.h"
#include "../../Event/interface/ElectronCollection.h"
#include "../../constants/particleMasses.h"


bool chargeFlips::passChargeRequirements( const Electron& electron ){
    if( !electron.isTight() ){
        return false;
    }
    return electron.passChargeConsistency();
}


bool chargeFlips::passChargeRequirements( const Muon& muon ){
    if( !muon.isTight() ){
        return false;
    }
    return ( fabs( muon.trackPtError() / muon.trackPt() ) < 0.2 );
}


bool chargeFlips::passChargeFlipEventSelection( Event& event, const bool diElectron, const bool onZ, const bool bVeto){

    //don't consider taus here
    event.removeTaus();

    //baseline lepton selection
    event.cleanElectronsFromLooseMuons();
    event.selectLooseLeptons();

    //clean jets from leptons before tightening selection
    event.cleanJetsFromFOLeptons();
    event.selectGoodJets();
    
    //select 2 leptons passing tight + charge requirements and veto third lepton
    if( event.numberOfLightLeptons() != 2 ) return false;
    event.selectTightLeptons();

    //apply charge requirements
    //event.selectElectrons( chargeFlips::passChargeRequirements );
    //event.selectMuons( chargeFlips::passChargeRequirements );

    if( event.numberOfLightLeptons() != 2 ) return false;

    //if specified require that both leptons are electrons
    if( diElectron && event.numberOfElectrons() != 2 ) return false;

    //if specified require the invariant mass to be compatible with the Z
    if( onZ ){
        double mll = ( event.electron( 0 ) + event.electron( 1 ) ).mass();
        if( fabs( mll - particle::mZ ) >= 10. ) return false;
    }

    //if specified select jets and veto presence of b-jet
    if( bVeto ){
        if( event.numberOfMediumBTaggedJets() != 0 ) return false;
    }

    return true;
}
