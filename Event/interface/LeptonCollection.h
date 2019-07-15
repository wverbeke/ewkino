#ifndef LeptonCollection_H
#define LeptonCollection_H

//include other parts of code 
#include "../../objects/interface/Lepton.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "PhysicsObjectCollection.h"
#include "MuonCollection.h"
#include "ElectronCollection.h"
#include "TauCollection.h"
#include "LightLeptonCollection.h"



class LeptonCollection : public PhysicsObjectCollection< Lepton > {
    
    public:
        LeptonCollection( const TreeReader& );

        MuonCollection muonCollection() const;
        ElectronCollection electronCollection() const;
        TauCollection tauCollection() const;
        LightLeptonCollection lightLeptonCollection() const;

        //select leptons
        void selectLooseLeptons();
        void selectFOLeptons();
        void selectTightLeptons();

        //clean electrons and taus 
        void cleanElectronsFromLooseMuons( const double coneSize = 0.05 );
        void cleanElectronsFromFOMuons( const double coneSize = 0.05 );
        void cleanTausFromLooseLightLeptons( const double coneSize = 0.4 );
        void cleanTausFromFOLightLeptons( const double coneSize = 0.4 );

    private:

        //clean electrons and taus
        void cleanElectronsFromMuons( bool (Lepton::*passSelection)() const, const double );
        void cleanTausFromLightLeptons( bool (Lepton::*passSelection)() const, const double );
        void clean( bool (Lepton::*isFlavorToClean)() const, bool (Lepton::*isFlavorToCleanFrom)() const, bool (Lepton::*passSelection)() const, const double );
};



#endif
