#include "../interface/Electron.h"

//include other parts of framework
#include "../interface/ElectronSelector.h"


Electron::Electron( const TreeReader& treeReader, const unsigned leptonIndex ):
    LightLepton( treeReader, leptonIndex, new ElectronSelector( this ) ),
    _passChargeConsistency( treeReader._lElectronChargeConst[leptonIndex] ),
    _passDoubleEGEmulation( treeReader._lElectronPassEmu[leptonIndex] ),
    _passConversionVeto( treeReader._lElectronPassConvVeto[leptonIndex] ),
    _numberOfMissingHits( treeReader._lElectronMissingHits[leptonIndex] ),
    _electronMVASummer16GP( treeReader._lElectronSummer16MvaGP[leptonIndex] ),
    _electronMVASummer16HZZ( treeReader._lElectronSummer16MvaHZZ[leptonIndex] ),
    _electronMVAFall17Iso( treeReader._lElectronMvaFall17Iso[leptonIndex] ),
    _electronMVAFall17NoIso( treeReader._lElectronMvaFall17NoIso[leptonIndex] ),
    _passElectronMVAFall17NoIsoLoose( treeReader._lElectronPassMVAFall17NoIsoWPLoose[leptonIndex] ), 
    _passElectronMVAFall17NoIsoWP90( treeReader._lElectronPassMVAFall17NoIsoWP90[leptonIndex] ),
    _passElectronMVAFall17NoIsoWP80( treeReader._lElectronPassMVAFall17NoIsoWP80[leptonIndex] ),
    _etaSuperCluster( treeReader._lEtaSC[leptonIndex] ),
    _hOverE( treeReader._lElectronHOverE[leptonIndex] ),
    _inverseEMinusInverseP( treeReader._lElectronEInvMinusPInv[leptonIndex] ),
    _sigmaIEtaEta( treeReader._lElectronSigmaIetaIeta[leptonIndex] ),
    _isVetoPOGElectron( treeReader._lPOGVeto[leptonIndex] ),
    _isLoosePOGElectron( treeReader._lPOGLoose[leptonIndex] ),
    _isMediumPOGElectron( treeReader._lPOGMedium[leptonIndex] ),
    _isTightPOGElectron( treeReader._lPOGTight[leptonIndex] ),
    _pt_ScaleUp( treeReader._lPtScaleUp[leptonIndex] ),
    _pt_ScaleDown( treeReader._lPtScaleDown[leptonIndex] ),
    _pt_ResUp( treeReader._lPtResUp[leptonIndex] ),
    _pt_ResDown( treeReader._lPtResDown[leptonIndex] ),
    _e_ScaleUp( treeReader._lEScaleUp[leptonIndex] ),
    _e_ScaleDown( treeReader._lEScaleDown[leptonIndex] ),
    _e_ResUp( treeReader._lEResUp[leptonIndex] ),
    _e_ResDown( treeReader._lEResDown[leptonIndex] )
{
    
    //apply electron energy correction
    setLorentzVector( treeReader._lPtCorr[ leptonIndex ], eta(), phi(), 
		      treeReader._lECorr[ leptonIndex ] );

    //make sure also non-cone-corrected pt is set to the corrected value
    _uncorrectedPt = pt();
    _uncorrectedE = energy();
}


Electron::Electron( const Electron& rhs ) :
	LightLepton( rhs, new ElectronSelector( this ) ),
	_passChargeConsistency( rhs._passChargeConsistency ),
	_passDoubleEGEmulation( rhs._passDoubleEGEmulation ),
	_passConversionVeto( rhs._passConversionVeto ),
	_numberOfMissingHits( rhs._numberOfMissingHits ),
	_electronMVASummer16GP( rhs._electronMVASummer16GP ),
	_electronMVASummer16HZZ( rhs._electronMVASummer16HZZ ),
	_electronMVAFall17Iso( rhs._electronMVAFall17Iso ),
	_electronMVAFall17NoIso( rhs._electronMVAFall17NoIso ),
	_passElectronMVAFall17NoIsoLoose( rhs._passElectronMVAFall17NoIsoLoose ),
	_passElectronMVAFall17NoIsoWP90( rhs._passElectronMVAFall17NoIsoWP90 ),
	_passElectronMVAFall17NoIsoWP80( rhs._passElectronMVAFall17NoIsoWP80 ),
	_etaSuperCluster( rhs._etaSuperCluster ),
	_hOverE( rhs._hOverE ),
	_inverseEMinusInverseP( rhs._inverseEMinusInverseP ),
	_sigmaIEtaEta( rhs._sigmaIEtaEta ),
	_isVetoPOGElectron( rhs._isVetoPOGElectron ),
	_isLoosePOGElectron( rhs._isLoosePOGElectron ),
	_isMediumPOGElectron( rhs._isMediumPOGElectron ),
	_isTightPOGElectron( rhs._isTightPOGElectron ),
	_pt_ScaleUp( rhs._pt_ScaleUp ),
	_pt_ScaleDown( rhs._pt_ScaleDown ),
	_pt_ResUp( rhs._pt_ResUp ),
	_pt_ResDown( rhs._pt_ResDown ),
	_e_ScaleUp( rhs._e_ScaleUp ),
        _e_ScaleDown( rhs._e_ScaleDown ),     
        _e_ResUp( rhs._e_ResUp ),
        _e_ResDown( rhs._e_ResDown )
	{}


Electron::Electron( Electron&& rhs ) noexcept : 
	LightLepton( std::move( rhs ), new ElectronSelector( this ) ),
	_passChargeConsistency( rhs._passChargeConsistency ),
    _passDoubleEGEmulation( rhs._passDoubleEGEmulation ),
    _passConversionVeto( rhs._passConversionVeto ),
    _numberOfMissingHits( rhs._numberOfMissingHits ),
    _electronMVASummer16GP( rhs._electronMVASummer16GP ),
    _electronMVASummer16HZZ( rhs._electronMVASummer16HZZ ),
    _electronMVAFall17Iso( rhs._electronMVAFall17Iso ),
    _electronMVAFall17NoIso( rhs._electronMVAFall17NoIso ),
    _passElectronMVAFall17NoIsoLoose( rhs._passElectronMVAFall17NoIsoLoose ),
    _passElectronMVAFall17NoIsoWP90( rhs._passElectronMVAFall17NoIsoWP90 ),
    _passElectronMVAFall17NoIsoWP80( rhs._passElectronMVAFall17NoIsoWP80 ),
    _etaSuperCluster( rhs._etaSuperCluster ),
    _hOverE( rhs._hOverE ),
    _inverseEMinusInverseP( rhs._inverseEMinusInverseP ),
    _sigmaIEtaEta( rhs._sigmaIEtaEta ),
    _isVetoPOGElectron( rhs._isVetoPOGElectron ),
    _isLoosePOGElectron( rhs._isLoosePOGElectron ),
    _isMediumPOGElectron( rhs._isMediumPOGElectron ),
    _isTightPOGElectron( rhs._isTightPOGElectron ),
    _pt_ScaleUp( rhs._pt_ScaleUp ),
    _pt_ScaleDown( rhs._pt_ScaleDown ),     
    _pt_ResUp( rhs._pt_ResUp ),
    _pt_ResDown( rhs._pt_ResDown ),
    _e_ScaleUp( rhs._e_ScaleUp ),
    _e_ScaleDown( rhs._e_ScaleDown ),  
    _e_ResUp( rhs._e_ResUp ),
    _e_ResDown( rhs._e_ResDown )
    {}


std::ostream& Electron::print( std::ostream& os ) const{
    os << "Electron : ";
    LightLepton::print( os );
    os << " / passChargeConsistency = " << _passChargeConsistency;
    os << " / passDoubleEGEmulation = " << _passDoubleEGEmulation;
    os << " / passConversionVeto = " << _passConversionVeto;
    os << " / numberOfMissingHits = " << _numberOfMissingHits;
    os << " / electronMVASummer16GP = " << _electronMVASummer16GP;
    os << " / electronMVASummer16HZZ = " << _electronMVASummer16HZZ;
    os << " / electronMVAFall17Iso = " << _electronMVAFall17Iso;
    os << " / electronMVAFall17NoIso = " << _electronMVAFall17NoIso;
    os << " / passElectronMVAFall17NoIsoLoose = " << _passElectronMVAFall17NoIsoLoose;
    os << " / passElectronMVAFall17NoIsoWP90 = " << _passElectronMVAFall17NoIsoWP90;
    os << " / passElectronMVAFall17NoIsoWP80 = " << _passElectronMVAFall17NoIsoWP80;
    os << " / etaSuperCluster = " << _etaSuperCluster;
    os << " / hOverE = " << _hOverE;
    os << " / inverseEMinusInverseP = " << _inverseEMinusInverseP;
    os << " / sigmaIEtaEta = " << _sigmaIEtaEta;
    if( _isTightPOGElectron ){
        os << " / tight POG electron";
    } else if( _isMediumPOGElectron ){
        os << " / medium POG electron";
    } else if( _isLoosePOGElectron ){
        os << " / loose POG electron";
    } else if( _isVetoPOGElectron ){
        os << " / veto POG electron";
    } else{
        os << " / fails POG electron selection";
    }
    os << " / ptScaleUp " << _pt_ScaleUp;
    os << " / ptScaleDown " << _pt_ScaleDown;
    os << " / ptResUp " << _pt_ResUp;
    os << " / ptResDown " << _pt_ResDown;
    os << " / eScaleUp " << _e_ScaleUp;
    os << " / eScaleDown " << _e_ScaleDown;
    os << " / eResUp " << _e_ResUp;
    os << " / eResDown " << _e_ResDown;

    return os;    
}

Electron Electron::variedElectron(const double newPt, const double newE) const{
    Electron variedElectron( *this );
    // the variations must be determined relative w.r.t. the non-cone-corrected pt!
    double relNewPt = newPt/uncorrectedPt();
    double relNewE = newE/uncorrectedE();    
    variedElectron.setLorentzVector( relNewPt*pt(), eta(), phi(), relNewE*energy() );
    return variedElectron;
}


Electron Electron::electronScaleDown() const{
    return variedElectron( _pt_ScaleDown, _e_ScaleDown );
}


Electron Electron::electronScaleUp() const{
    return variedElectron( _pt_ScaleUp, _e_ScaleUp );
}


Electron Electron::electronResDown() const{
    return variedElectron( _pt_ResDown, _e_ResDown );
}


Electron Electron::electronResUp() const{
    return variedElectron( _pt_ResUp, _e_ResUp );
}

