#include "../interface/Tau.h"

//include other parts of framework
#include "../interface/TauSelector.h"


Tau::Tau( const TreeReader& treeReader, const unsigned leptonIndex ) :
    Lepton( treeReader, leptonIndex, new TauSelector( this ) ),
    _passMuonVeto( treeReader._tauMuonVeto[leptonIndex] ),
    _passElectronVeto( treeReader._tauEleVeto[leptonIndex] ),
    _passDecayModeFindingNew( treeReader._decayModeFindingNew[leptonIndex] )
    {} 


Tau::Tau( const Tau& rhs ) :
    Lepton( rhs, new TauSelector( this) ),
	_passMuonVeto( rhs._passMuonVeto ),
	_passElectronVeto( rhs._passElectronVeto ),
	_passDecayModeFindingNew( rhs._passDecayModeFindingNew )
{
    //selector = new TauSelector( this );
}


Tau::Tau( Tau&& rhs ) noexcept :
    Lepton( std::move( rhs ), new TauSelector( this ) ),
	_passMuonVeto( rhs._passMuonVeto ),
    _passElectronVeto( rhs._passElectronVeto ),
    _passDecayModeFindingNew( rhs._passDecayModeFindingNew ) 
{
	//selector = new TauSelector( this );
}
