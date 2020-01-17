#include "../interface/ewkinoSelection.h"

void ewkino::applyBaselineObjectSelection( Event& event, const bool allowUncertainties ){

    event.selectLooseLeptons();
    event.cleanElectronsFromLooseMuons();
    event.cleanTausFromLooseLightLeptons();
    event.cleanJetsFromFOLeptons();
    if( allowUncertainties ){
        event.jetCollection().selectGoodAnyVariationJets();
    } else {
        event.selectGoodJets();
    }
}


bool ewkino::passLowMllVeto( const Event& event, const double vetoValue ){

    for( const auto& leptonPtrPair : event.lightLeptonCollection().pairCollection() ){

        //veto OSSF pairs of low mass
        Lepton& lepton1 = *( leptonPtrPair.first );
        Lepton& lepton2 = *( leptonPtrPair.second );
        if( !oppositeSignSameFlavor( lepton1, lepton2 ) ){
            continue;
        }
        if( ( lepton1 + lepton2 ).mass() < vetoValue ){
            return false;
        }
    }
    return true;
}


bool ewkino::passBaselineSelection( Event& event, const bool allowUncertainties, const bool bVeto, const bool mllVeto ){
    
    applyBaselineObjectSelection( event, allowUncertainties );
    if( event.numberOfLeptons() < 3 ) return false;
    if( mllVeto && !passLowMllVeto( event, 12 ) ) return false;
    event.selectFOLeptons();
    event.applyLeptonConeCorrection();
    event.sortLeptonsByPt();
    if( event.numberOfLeptons() < 3 ) return false;
    if( event.numberOfTaus() > 2 ) return false;
    if( bVeto ){
        if( allowUncertainties ){
            if( event.jetCollection().minNumberOfTightBTaggedJetsAnyVariation() > 0 ){
                return false;
            }
        } else {
            if( event.numberOfTightBTaggedJets() > 0 ){
                return false;
            }
        }
    }
    return true;
}


bool ewkino::passTriggerSelection( const Event& event ){
    if( event.numberOfMuons() >= 1 ){
        if( event.passTriggers_m() ) return true;
    } 
    if( event.numberOfMuons() >= 2 ){
        if( event.passTriggers_mm() ) return true;
    }
    if( event.numberOfMuons() >= 3 ){
        if( event.passTriggers_mmm() ) return true;
    }
    if( event.numberOfElectrons() >= 1 ){
        if( event.passTriggers_e() ) return true;
    }
    if( event.numberOfElectrons() >= 2 ){
        if( event.passTriggers_ee() ) return true;
    }
    if( event.numberOfElectrons() >= 3 ){
        if( event.passTriggers_eee() ) return true;
    }
    if( ( event.numberOfMuons() >= 1 ) && ( event.numberOfElectrons() >= 1 ) ){
        if( event.passTriggers_em() ) return true;
    }
    if( ( event.numberOfMuons() >= 2 ) && ( event.numberOfElectrons() >= 1 ) ){
        if( event.passTriggers_emm() ) return true;
    }
    if( ( event.numberOfMuons() >= 1 ) && ( event.numberOfElectrons() >= 2 ) ){
        if( event.passTriggers_eem() ) return true;
    }
    if( ( event.numberOfMuons() >= 1 ) && ( event.numberOfTaus() >= 1 ) ){
        if( event.passTriggers_mt() ) return true;
    }
    if( ( event.numberOfElectrons() >= 1 ) && ( event.numberOfTaus() >= 1 ) ){
        if( event.passTriggers_et() ) return true;
    }

    return false;
}


bool ewkino::passPtCuts( const Event& event ){
    
    //assume leptons were ordered while applying baseline selection
    
    //leading lepton
    if( event.lepton( 0 ).isMuon() && event.lepton( 0 ).pt() < 20 ) return false;
    if( event.lepton( 0 ).isElectron() && event.lepton( 0 ).pt() < 25 ) return false;

    //subleading lepton
    if( event.lepton( 1 ).isMuon() && event.lepton( 1 ).pt() < 10 ) return false;
    if( event.lepton( 1 ).isElectron() && event.lepton( 1 ).pt() < 15 ) return false;
    return true;
}
