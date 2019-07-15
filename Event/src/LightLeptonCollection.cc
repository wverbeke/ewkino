#include "../interface/LightLeptonCollection.h"

//include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"


LightLeptonCollection::LightLeptonCollection( const TreeReader& treeReader ){
    for( unsigned m = 0; m < _nMu; ++m ){
        push_back( Muon( treeReader, m ) );
    }
    for( unsigned e = _nMu; e < _nLight; ++e ){
        push_back( Electron( treeReader, e ) );
    }
}
