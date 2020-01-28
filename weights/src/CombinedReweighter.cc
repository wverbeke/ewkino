#include "../interface/CombinedReweighter.h"

//include c++ library classes
#include <stdexcept>


void CombinedReweighter::addReweighter( const std::string& name, const std::shared_ptr< Reweighter >& reweighter ){
    reweighterMap[ name ] = reweighter;
    reweighterVector.push_back( reweighter );
}


std::map< std::string, std::shared_ptr< Reweighter > >::const_iterator findAndCheckReweighter( const std::string& name, const std::map< std::string, std::shared_ptr< Reweighter > >& reweighterMap ){
    auto it = reweighterMap.find( name );
    if( it == reweighterMap.cend() ){
        throw std::invalid_argument( "Requested to access Reweighter '" + name + "', but no Reweighter of that name is present.");
    }
    return it;
}


void CombinedReweighter::eraseReweighter( const std::string& name ){
    auto it = findAndCheckReweighter( name, reweighterMap );

    //use the address to delete the corresponding element in the vector
    Reweighter* address = it->second.get();

    //remove from map
    reweighterMap.erase( it );

    //remove from vector
    for( auto it = reweighterVector.begin(); it != reweighterVector.end(); ++it ){
        if( it->get() == address ){
            reweighterVector.erase( it );

            //break is needed to avoid problems with invalid iterators after calling erase
            break;
        }
    }
}


const Reweighter* CombinedReweighter::operator[]( const std::string& name ) const{
    auto it = findAndCheckReweighter( name, reweighterMap );
    return it->second.get();
}


double CombinedReweighter::totalWeight( const Event& event ) const{
    double weight = 1.;
    for( const auto& r : reweighterVector ){
        weight *= r->weight( event );
    }
    return weight;
}
