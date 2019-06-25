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


MuonCollection LeptonCollection::muonCollection() const{
    std::vector< std::shared_ptr< Muon > > muonVector;
    for( const auto& leptonPtr : *this ){
        if( leptonPtr->isMuon() ){
            muonVector.push_back( std::static_pointer_cast< Muon >( leptonPtr ) );
        }
    }
    return MuonCollection( muonVector );
}


ElectronCollection LeptonCollection::electronCollection() const{
    std::vector< std::shared_ptr< Electron > > electronVector;
    for( const auto& leptonPtr : *this ){
        if( leptonPtr->isElectron() ){
            electronVector.push_back( std::static_pointer_cast< Electron >( leptonPtr ) );
        }
    }
    return ElectronCollection( electronVector );
}


TauCollection LeptonCollection::tauCollection() const{
    std::vector< std::shared_ptr< Tau > > tauVector;
    for( const auto& leptonPtr : *this  ){
        if( leptonPtr->isTau() ){
            tauVector.push_back( std::static_pointer_cast< Tau >( leptonPtr ) );
        }
    }
    return TauCollection( tauVector );
}


void LeptonCollection::selectLooseLeptons(){
    selectObjects( &Lepton::isLoose );
}


void LeptonCollection::selectFOLeptons(){
    selectObjects( &Lepton::isFO );
}


void LeptonCollection::selectTightLeptons(){
    selectObjects( &Lepton::isTight );
}
