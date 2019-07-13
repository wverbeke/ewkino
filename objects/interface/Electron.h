#ifndef Electron_H
#define Electron_H

//include other parts of code 
#include "LightLepton.h"

class Electron : public LightLepton{

    public:
        Electron( const TreeReader&, const unsigned );
        Electron( const Electron& );
        Electron( Electron&& ) noexcept;

        Electron& operator=( const Electron& ) = default;
        Electron& operator=( Electron&& ) = default;

        virtual bool isMuon() const override { return false; }
        virtual bool isElectron() const override { return true; }

        bool passChargeConsistency() const{ return _passChargeConsistency; }
        bool passDoubleEGEmulation() const{ return _passDoubleEGEmulation; }
        bool passConversionVeto() const{ return _passConversionVeto; }
        unsigned numberOfMissingHits() const{ return _numberOfMissingHits; }

        float electronMVASummer16GP() const{ return _electronMVASummer16GP; }
        float electronMVASummer16HZZ() const{ return _electronMVASummer16HZZ; }
        float electronMVAFall17Iso() const{ return _electronMVAFall17Iso; }
        float electronMVAFall17NoIso() const{ return _electronMVAFall17NoIso; }

        double etaSuperCluster() const{ return _etaSuperCluster; }

        bool isVetoPOGElectron() const{ return _isVetoPOGElectron; }
        bool isLoosePOGElectron() const{ return _isLoosePOGElectron; }
        bool isMediumPOGElectron() const{ return _isMediumPOGElectron; }
        bool isTightPOGElectron() const{ return _isTightPOGElectron; }

    private:
        bool _passChargeConsistency = false;
        bool _passDoubleEGEmulation = false;
        bool _passConversionVeto = false;
        unsigned _numberOfMissingHits = 0;
    
        //several versions of electron MVA
        float _electronMVASummer16GP = 0;
        float _electronMVASummer16HZZ = 0;
        float _electronMVAFall17Iso = 0;
        float _electronMVAFall17NoIso = 0;

        //pseudorapidity of the supercluster
        double _etaSuperCluster = 0;

        //cut based POG ID working points (include isolation) 
        bool _isVetoPOGElectron = false;
        bool _isLoosePOGElectron = false;
        bool _isMediumPOGElectron = false;
        bool _isTightPOGElectron = false;
        
        virtual Electron* clone() const & override{ return new Electron( *this ); }
        virtual Electron* clone() && override{ return new Electron( std::move( *this ) ); }
};
#endif 
