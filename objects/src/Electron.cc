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
    _etaSuperCluster( treeReader._lEtaSC[leptonIndex] ),
    _isVetoPOGElectron( treeReader._lPOGVeto[leptonIndex] ),
    _isLoosePOGElectron( treeReader._lPOGLoose[leptonIndex] ),
    _isMediumPOGElectron( treeReader._lPOGMedium[leptonIndex] ),
    _isTightPOGElectron( treeReader._lPOGTight[leptonIndex] )
    {}


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
	_etaSuperCluster( rhs._etaSuperCluster ),
	_isVetoPOGElectron( rhs._isVetoPOGElectron ),
	_isLoosePOGElectron( rhs._isLoosePOGElectron ),
	_isMediumPOGElectron( rhs._isMediumPOGElectron ),
	_isTightPOGElectron( rhs._isTightPOGElectron )
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
    _etaSuperCluster( rhs._etaSuperCluster ),
    _isVetoPOGElectron( rhs._isVetoPOGElectron ),
    _isLoosePOGElectron( rhs._isLoosePOGElectron ),
    _isMediumPOGElectron( rhs._isMediumPOGElectron ),
    _isTightPOGElectron( rhs._isTightPOGElectron )
    {}
