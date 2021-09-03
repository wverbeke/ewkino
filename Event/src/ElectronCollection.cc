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

ElectronCollection ElectronCollection::ElectronScaleDownCollection() const{
    return buildVariedCollection( &Electron::ElectronScaleDown );
}

ElectronCollection ElectronCollection::ElectronScaleUpCollection() const{
    return buildVariedCollection( &Electron::ElectronScaleUp );
}

ElectronCollection ElectronCollection::ElectronResDownCollection() const{
    return buildVariedCollection( &Electron::ElectronResDown );
}

ElectronCollection ElectronCollection::ElectronResUpCollection() const{
    return buildVariedCollection( &Electron::ElectronResUp );
}

