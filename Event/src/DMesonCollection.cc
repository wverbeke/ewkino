#include "../interface/DMesonCollection.h"

// constructor
DMesonCollection::DMesonCollection( const TreeReader& treeReader ){
    for( unsigned j = 0; j < treeReader._nDs; ++j ){
	this->push_back( DMeson( treeReader, j ) ); 
    }
}


// helper functions
DMesonCollection DMesonCollection::buildSubCollection(
        bool (DMeson::*passSomeSelection)() const ) const{
    std::vector< std::shared_ptr< DMeson > > dmesonVector;
    for( const auto& dmesonPtr : *this ){
        if( ( *dmesonPtr.*passSomeSelection )() ){
            // dmesons are shared between collections!
            dmesonVector.push_back( dmesonPtr );
        }
    }
    return DMesonCollection( dmesonVector );
}


// D meson selection: standard "good" selection
void DMesonCollection::selectGoodDMesons(){
    selectObjects( &DMeson::isGood );
}


DMesonCollection DMesonCollection::goodDMesonCollection() const{
    return buildSubCollection( &DMeson::isGood );
}


// D meson selection: runtime specified cuts
void DMesonCollection::selectPassingDMesons( std::vector< std::tuple< double (DMeson::*)() const,
					     double, double > > cuts ){
    // copied from PhysicsObjectCollection::selectObjects
    for( DMesonCollection::const_iterator it = this->cbegin(); it != this->cend(); ){
        if( !( (**it).passCuts(cuts) ) ){ it = erase( it ); } 
	else { ++it; }
    }
}


DMesonCollection DMesonCollection::passingDMesonCollection( std::vector< std::tuple<
                                          double (DMeson::*)() const,
                                          double, double > > cuts ){
    // copied from DMesonCollection::buildSubCollection
    std::vector< std::shared_ptr< DMeson > > dmesonVector;
    for( const auto& dmesonPtr : *this ){
        if( dmesonPtr->passCuts(cuts) ){
            // dmesons are shared between collections!
            dmesonVector.push_back( dmesonPtr );
        }
    }
    return DMesonCollection( dmesonVector );   
}

