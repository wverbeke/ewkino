#include "../interface/DMesonCollection.h"

DMesonCollection::DMesonCollection( const TreeReader& treeReader ){
    for( unsigned j = 0; j < treeReader._nDs; ++j ){
	this->push_back( DMeson( treeReader, j ) ); 
    }
}


void DMesonCollection::selectGoodDMesons(){
    selectObjects( &DMeson::isGood );
}


DMesonCollection DMesonCollection::buildSubCollection( bool (DMeson::*passSomeSelection)() const ) const{
    std::vector< std::shared_ptr< DMeson > > dmesonVector;
    for( const auto& dmesonPtr : *this ){
        if( ( *dmesonPtr.*passSomeSelection )() ){
            // dmesons are shared between collections!
            dmesonVector.push_back( dmesonPtr );
        }
    }
    return DMesonCollection( dmesonVector );
}


DMesonCollection DMesonCollection::goodDMesonCollection() const{
    return buildSubCollection( &DMeson::isGood );
}
