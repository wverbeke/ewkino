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


bool chargeFlips::passChargeFlipEventSelection( Event& event ){

    //select 2 tight electron and very 4th loose electron
    event.cleanElectronsFromLooseMuons();
    event.selectLooseLeptons();

    //clean jets from leptons before tightening selection
    event.cleanJetsFromFOLeptons();

    if( event.numberOfLightLeptons() != 2 ) return false;
    event.selectTightLeptons();
    if( event.numberOfElectrons() != 2 ) return false;

    //require the electrons to pass the charge requirements
    for( const auto& electronPtr : event.electronCollection() ){
        if( ! chargeFlips::passChargeRequirements( *electronPtr ) ) return false;
    }

    //require the invariant mass to be compatible with the Z
    double mll = ( event.electron( 0 ) + event.electron( 1 ) ).mass();
    if( fabs( mll - particle::mZ ) < 10. ) return false;

    //select jets and veto presence of b-jet
    event.selectGoodJets();
    if( event.numberOfMediumBTaggedJets() != 0 ) return false;

    return true;
}
