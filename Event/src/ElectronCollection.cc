#include "../interface/ElectronCollection.cc"


ElectronCollection::ElectronCollection( const TreaReader& ){
    for( unsigned e = treeReader._nMu; e < treeReader._nLight; ++ e){
        push_back( Electron( treeReader, e ) );
    }
}
