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


void LeptonCollection::clean( bool (Lepton::*isFlavorToClean)() const, bool (Lepton::*isFlavorToCleanFrom)() const, bool (Lepton::*passSelection)() const, const double coneSize ){
    std::vector< const_iterator > objectsToDelete;
    for( const_iterator l1It = cbegin(); l1It != cend(); ++l1It ){
        Lepton& l1 = **l1It;
        if( (l1.*isFlavorToClean)() ){
            for( const_iterator l2It = cbegin(); l2It != cend(); ++l2It ){

                //prevent comparing same objects 
                if( l1It == l2It ) continue;

                Lepton& l2 = **l2It;

                //make sure l2 passes required selection for cleaning 
                if( ! ( (l2.*isFlavorToCleanFrom)() && (l2.*passSelection)() ) ) continue;

                //clean within given cone size
                if( deltaR( l1, l2 ) < coneSize ){
                    objectsToDelete.push_back( l1It );
                }
            }
        }
    }
    erase( objectsToDelete );
}


void LeptonCollection::cleanElectronsFromMuons( bool (Lepton::*passSelection)() const, const double coneSize ){
    return clean( &Lepton::isElectron, &Lepton::isMuon, passSelection, coneSize );
}


void LeptonCollection::cleanTausFromLightLeptons( bool (Lepton::*passSelection)() const, const double coneSize ){
    return clean( &Lepton::isTau, &Lepton::isLightLepton, passSelection, coneSize );
}


void LeptonCollection::cleanElectronsFromLooseMuons( const double coneSize ){
    return cleanElectronsFromMuons( &Lepton::isLoose, coneSize );
}


void LeptonCollection::cleanElectronsFromFOMuons( const double coneSize ){
    return cleanElectronsFromMuons( &Lepton::isFO, coneSize );
}


void LeptonCollection::cleanTausFromLooseLightLeptons( const double coneSize ){
    return cleanTausFromLightLeptons( &Lepton::isLoose, coneSize );
}


void LeptonCollection::cleanTausFromFOLightLeptons( const double coneSize ){
    return cleanTausFromLightLeptons( &Lepton::isFO, coneSize );
}
