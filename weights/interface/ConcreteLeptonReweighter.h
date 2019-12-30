#ifndef ConcreteLeptonReweighter_H
#define ConcreteLeptonReweighter_H

#include "LeptonReweighter.h"

//include other parts of framework
#include "../../objects/interface/Muon.h"
#include "../../objects/interface/Electron.h"
#include "../../objects/interface/Tau.h"



class ElectronIDReweighter : public LeptonReweighter< Electron > {

    public:
        ElectronIDReweighter( const std::shared_ptr< TH2 >& scaleFactors, LeptonSelectionHelper* selector ) :
            LeptonReweighter< Electron >( scaleFactors, selector ) {}
        virtual double etaVariable( const Electron& electron ) const override{ return fabs( electron.etaSuperCluster() ); }
};


class ElectronReweighter : public LeptonReweighter< Electron > {

    public:
        ElectronReweighter( const std::shared_ptr< TH2 >& scaleFactors, LeptonSelectionHelper* selector ) :
            LeptonReweighter< Electron >( scaleFactors, selector ) {}
};



class MuonReweighter : public LeptonReweighter< Muon > {
    
    public:
        MuonReweighter( const std::shared_ptr< TH2 >& scaleFactors, LeptonSelectionHelper* selector ) :
            LeptonReweighter< Muon >( scaleFactors, selector ) {}

};



class TauReweighter : public LeptonReweighter< Tau > {

    public:
        TauReweighter( const std::shared_ptr< TH2 >& scaleFactors, LeptonSelectionHelper* selector ) :
            LeptonReweighter< Tau >( scaleFactors, selector ) {}

};

#endif 
