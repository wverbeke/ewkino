#include "../interface/Tau.h"


Tau::Tau( const TreeReader& treeReader, const unsigned leptonIndex ) :
    Lepton( treeReader, leptonIndex ),
    _passMuonVeto( treeReader._tauMuonVeto[leptonIndex] ),
    _passElectronVeto( treeReader._tauEleVeto[leptonIndex] ),
    _passDecayModeFindingNew( treeReader._decayModeFindingNew[leptonIndex] )
    {} 
        
