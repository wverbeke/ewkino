#ifndef Electron_H
#define Electron_H

//include other parts of code 
#include "PhysicsObject.h"
#include "LightLepton.h"

template< typename ObjectType > class PhysicsObjectCollection;

class Electron : public LightLepton{

    friend class PhysicsObjectCollection<Electron>;

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

        bool passElectronMVAFall17NoIsoLoose() const{ return _passElectronMVAFall17NoIsoLoose; }
        bool passElectronMVAFall17NoIsoWP90() const{ return _passElectronMVAFall17NoIsoWP90; }
        bool passElectronMVAFall17NoIsoWP80() const{ return _passElectronMVAFall17NoIsoWP80; }

        double etaSuperCluster() const{ return _etaSuperCluster; }
        double hOverE() const{ return _hOverE; }
        double inverseEMinusInverseP() const{ return _inverseEMinusInverseP; }
        double sigmaIEtaEta() const{ return _sigmaIEtaEta; }

        bool isVetoPOGElectron() const{ return _isVetoPOGElectron; }
        bool isLoosePOGElectron() const{ return _isLoosePOGElectron; }
        bool isMediumPOGElectron() const{ return _isMediumPOGElectron; }
        bool isTightPOGElectron() const{ return _isTightPOGElectron; }

	// create new Electron instances with varied momentum/energy scale
	Electron variedElectron( const double, const double ) const;
	virtual Electron electronScaleUp() const;
	virtual Electron electronScaleDown() const;
	virtual Electron electronResUp() const;
	virtual Electron electronResDown() const;

        virtual std::ostream& print( std::ostream& os = std::cout ) const override;

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

        //precomputed electron MVA decisions
        bool _passElectronMVAFall17NoIsoLoose;
        bool _passElectronMVAFall17NoIsoWP90;
        bool _passElectronMVAFall17NoIsoWP80;

        //pseudorapidity of the supercluster
        double _etaSuperCluster = 0;

        //cluster id values
        double _hOverE = 0;
        double _inverseEMinusInverseP = 0;
        double _sigmaIEtaEta = 0;

        //cut based POG ID working points (include isolation) 
        bool _isVetoPOGElectron = false;
        bool _isLoosePOGElectron = false;
        bool _isMediumPOGElectron = false;
        bool _isTightPOGElectron = false;

	// varied momentum/energy scales
	double _pt_ScaleUp = 0;
	double _pt_ScaleDown = 0;
	double _pt_ResUp = 0;
	double _pt_ResDown = 0;
	double _e_ScaleUp = 0;
	double _e_ScaleDown = 0;
	double _e_ResUp = 0;
	double _e_ResDown = 0;
        
        virtual Electron* clone() const & override{ return new Electron( *this ); }
        virtual Electron* clone() && override{ return new Electron( std::move( *this ) ); }
};
#endif 
