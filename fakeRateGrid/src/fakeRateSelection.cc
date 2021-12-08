#include "../interface/fakeRateSelection.h"

//include c++ library classes 
#include <stdexcept>
#include <map>

//include other parts of framework
#include "../../Tools/interface/stringTools.h"
#include "../../Event/interface/Event.h"

//event selection for fake-rate measurement 
bool fakeRate::passFakeRateEventSelection( Event& event, bool onlyMuons, bool onlyElectrons, 
	bool onlyTightLeptons, double ptRatioCut, double deepFlavorCut, int extraCut,
	bool requireJet, double jetDeltaRCut, double jetPtCut ){

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
        event.selectFORunTimeLeptons( ptRatioCut, deepFlavorCut, extraCut );
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

    //optionally require the presence of at least one good jet
    if( requireJet ){
        event.selectGoodJets();
        event.cleanJetsFromLooseLeptons();
        if( event.numberOfJets() < 1 ) return false;

        //apply cut on deltaR between lepton and any of the selected jets 
        double maxDeltaR = 0;
        for( auto jetPtr : event.jetCollection() ){
            if( jetPtr->pt() < jetPtCut ) continue;
            double currentDeltaR = deltaR( lepton, *jetPtr );
            if( currentDeltaR > maxDeltaR ){
                maxDeltaR = currentDeltaR;
            }
        }
        if( maxDeltaR < jetDeltaRCut ) return false;
    }
    return true;
}
