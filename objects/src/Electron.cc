#include "../interface/Electron.h"

//include other parts of framework
#include "../interface/ElectronSelector.h"


Electron::Electron( const TreeReader& treeReader, const unsigned leptonIndex ):
    LightLepton( treeReader, leptonIndex, new ElectronSelector( this ) ),
    _passChargeConsistency( treeReader._lElectronChargeConst[leptonIndex] ),
    _passDoubleEGEmulation( treeReader._lElectronPassEmu[leptonIndex] ),
    _passConversionVeto( treeReader._lElectronPassConvVeto[leptonIndex] ),
    _numberOfMissingHits( treeReader._lElectronMissingHits[leptonIndex] ),
    _electronMVASpring16GP( treeReader._lElectronMva[leptonIndex] ),
    _electronMVASpring16HZZ( treeReader._lElectronMvaHZZ[leptonIndex] ),
    _electronMVAFall17Iso( treeReader._lElectronMvaFall17Iso[leptonIndex] ),
    _electronMVAFall17NoIso( treeReader._lElectronMvaFall17NoIso[leptonIndex] ),
    _etaSuperCluster( treeReader._lEtaSC[leptonIndex] ),
    _isVetoPOGElectron( treeReader._lPOGVeto[leptonIndex] ),
    _isLoosePOGElectron( treeReader._lPOGLoose[leptonIndex] ),
    _isMediumPOGElectron( treeReader._lPOGMedium[leptonIndex] ),
    _isTightPOGElectron( treeReader._lPOGTight[leptonIndex] )
    {}
