#include "../interface/TauCollection.h"


TauCollection::TauCollection( const TreeReader& treeReader ){
    for( unsigned t = treeReader._nLight; t < treeReader._nL; ++t){
        push_back( Tau( treeReader, t ) );
    }
}
