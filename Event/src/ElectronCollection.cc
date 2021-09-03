#include "../interface/ElectronCollection.h"


ElectronCollection::ElectronCollection( const TreeReader& treeReader ){
    for( unsigned e = treeReader._nMu; e < treeReader._nLight; ++ e){
        push_back( Electron( treeReader, e ) );
    }
}

ElectronCollection ElectronCollection::buildVariedCollection( 
		    Electron (Electron::*variedElectron)() const ) const{
    std::vector< std::shared_ptr< Electron > > electronVector;
    for( const auto& electronPtr : *this ){
        electronVector.push_back( std::make_shared< Electron >( (*electronPtr.*variedElectron)() ) );
    }
    return ElectronCollection( electronVector );
}

ElectronCollection ElectronCollection::electronScaleDownCollection() const{
    return buildVariedCollection( &Electron::electronScaleDown );
}

ElectronCollection ElectronCollection::electronScaleUpCollection() const{
    return buildVariedCollection( &Electron::electronScaleUp );
}

ElectronCollection ElectronCollection::electronResDownCollection() const{
    return buildVariedCollection( &Electron::electronResDown );
}

ElectronCollection ElectronCollection::electronResUpCollection() const{
    return buildVariedCollection( &Electron::electronResUp );
}

