#include "../interface/JetCollection.h"


//include other parts of framework
#include "../interface/LeptonCollection.h"


JetCollection::JetCollection( const TreeReader& treeReader ){
    for( unsigned j = 0; j < treeReader._nJets; ++j ){
        this->push_back( Jet( treeReader, j ) ); 
    }
}


void JetCollection::selectGoodJets(){
    selectObjects( &Jet::isGood );
}


void JetCollection::cleanJetsFromLeptons( const LeptonCollection& leptonCollection, bool (Lepton::*passSelection)() const, const double coneSize ){
    std::vector< const_iterator > objectsToDelete;
    for( const_iterator jetIt = cbegin(); jetIt != cend(); ++jetIt ){
        Jet& jet = **jetIt;
        for( LeptonCollection::const_iterator lIt = leptonCollection.cbegin(); lIt != leptonCollection.cend(); ++lIt ){
            Lepton& lepton = **lIt;

            //lepton must pass specified selection
            if( !(lepton.*passSelection)() ) continue;

            //remove jet if it overlaps with a selected lepton
            if( deltaR( jet, lepton ) < coneSize ){
                objectsToDelete.push_back( jetIt );
                break;
            }
        }
    }
    erase( objectsToDelete );
}


void JetCollection::cleanJetsFromLooseLeptons( const LeptonCollection& leptonCollection, const double coneSize ){
    cleanJetsFromLeptons( leptonCollection, &Lepton::isLoose, coneSize );
} 


void JetCollection::cleanJetsFromFOLeptons( const LeptonCollection& leptonCollection, const double coneSize ){
    cleanJetsFromLeptons( leptonCollection, &Lepton::isFO, coneSize );
} 


void JetCollection::cleanJetsFromTightLeptons( const LeptonCollection& leptonCollection, const double coneSize ){
    cleanJetsFromLeptons( leptonCollection, &Lepton::isTight, coneSize );
}


JetCollection JetCollection::buildSubCollection( bool (Jet::*passSelection)() const ) const{
    std::vector< std::shared_ptr< Jet > > jetVector;
    for( const auto& jetPtr : *this ){
        if( (*jetPtr.*passSelection)() ){
            jetVector.push_back( jetPtr );
        }
    }
    return JetCollection( jetVector );
}


JetCollection JetCollection::looseBTagCollection() const{
    return buildSubCollection( &Jet::isBTaggedLoose );
}


JetCollection JetCollection::mediumBTagCollection() const{
    return buildSubCollection( &Jet::isBTaggedMedium );
}


JetCollection JetCollection::tightBTagCollection() const{
    return buildSubCollection( &Jet::isBTaggedTight );
}
