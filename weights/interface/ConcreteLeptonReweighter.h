#ifndef ConcreteLeptonReweighter_H
#define ConcreteLeptonReweighter_H

#include "LeptonReweighter.h"

//include other parts of framework
#include "../../objects/interface/Muon.h"
#include "../../objects/interface/Electron.h"
#include "../../objects/interface/Tau.h"



class ElectronIDReweighter : public LeptonReweighter< Electron > {

    using LeptonReweighter< Electron >::LeptonReweighter;

    public:
        virtual double etaVariable( const Electron& electron ) const override{ return fabs( electron.etaSuperCluster() ); }
};


class ElectronReweighter : public LeptonReweighter< Electron > {

    using LeptonReweighter< Electron >::LeptonReweighter;
};



class MuonReweighter : public LeptonReweighter< Muon > {
    
    using LeptonReweighter< Muon >::LeptonReweighter;
};



class TauReweighter : public LeptonReweighter< Tau > {

    using LeptonReweighter< Tau >::LeptonReweighter;
};

#endif 
