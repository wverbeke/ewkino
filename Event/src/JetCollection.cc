#include "../interface/JetCollection.h"


JetCollection::JetCollection( const TreeReader& treeReader ){
    for( unsigned j = 0; j < treeReader._nJets; ++j ){
        this->push_back( Jet( treeReader, j ) ); 
    }
}
