#include "../interface/LeptonCollection.h"

//include other parts of code 
#include "../../objects/interface/Muon.h"
#include "../../objects/interface/Electron.h"
#include "../../objects/interface/Tau.h"


LeptonCollection::LeptonCollection( const TreeReader& treeReader ){

    //add muons to lepton collection
    for( unsigned m = 0; m < treeReader._nMu; ++m){
        push_back( Muon( treeReader, m ) );
    }

    //add electrons to lepton collection
    for( unsigned e = treeReader._nMu; e < treeReader._nLight; ++ e){
        push_back( Electron( treeReader, e ) );
    } 

    //add taus to lepton collection
    for( unsigned t = treeReader._nLight; t < treeReader._nL; ++t){
        push_back( Tau( treeReader, t ) );
    }
}
