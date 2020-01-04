#ifndef ConcreteReweighterLeptons_H
#define ConcreteReweighterLeptons_H

//include other parts of framework
#include "ReweighterLeptons.h"
#include "ConcreteLeptonReweighter.h"


class ReweighterMuons : public ReweighterLeptons< Muon, MuonCollection, MuonReweighter >{

    using ReweighterLeptons< Muon, MuonCollection, MuonReweighter >::ReweighterLeptons;

    private:
        virtual MuonCollection leptonCollection( const Event& event ) const{ return event.muonCollection(); }

};



class ReweighterElectronsID : public ReweighterLeptons< Electron, ElectronCollection, ElectronIDReweighter >{
    
    using ReweighterLeptons< Electron, ElectronCollection, ElectronIDReweighter >::ReweighterLeptons;

    private:
        virtual ElectronCollection leptonCollection( const Event& event ) const{ return event.electronCollection(); }
};



class ReweighterElectrons : public ReweighterLeptons< Electron, ElectronCollection, ElectronReweighter >{
    
    using ReweighterLeptons< Electron, ElectronCollection, ElectronReweighter >::ReweighterLeptons;

    private:
        virtual ElectronCollection leptonCollection( const Event& event ) const{ return event.electronCollection(); }

};



class ReweighterTaus : public ReweighterLeptons< Tau, TauCollection, TauReweighter >{

    using ReweighterLeptons< Tau, TauCollection, TauReweighter >::ReweighterLeptons;

    private:
        virtual TauCollection leptonCollection( const Event& event ) const{ return event.tauCollection(); }

};

#endif
