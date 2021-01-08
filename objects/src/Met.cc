#include "../interface/Met.h"


Met::Met( const TreeReader& treeReader,
	    const bool readAllJECVariations, const bool readGroupedJECVariations ):
    PhysicsObject( treeReader._met, 0., treeReader._metPhi, treeReader._met ),
    _pt_JECDown( treeReader._metJECDown ), 
    _phi_JECDown( treeReader._metPhiJECDown ),
    _pt_JECUp( treeReader._metJECUp ),
    _phi_JECUp( treeReader._metPhiJECUp ),
    _pt_UnclDown( treeReader._metUnclDown ),
    _phi_UnclDown( treeReader._metPhiUnclDown ),
    _pt_UnclUp( treeReader._metUnclUp ),
    _phi_UnclUp( treeReader._metPhiUnclUp )
{
    if( readAllJECVariations ){
	for( const auto mapEl: treeReader._corrMETx_JECSourcesUp ){
            std::string key = mapEl.first;
            key = stringTools::removeOccurencesOf(key,"_corrMETx_");
            key = stringTools::removeOccurencesOf(key,"_JECSourcesUp");
            _JECSources.push_back( key ); // assume they are the same for Up/Down and x/y!
        }
	for( const std::string key: _JECSources ){
	    _pxy_JECSourcesUp.insert( {key, std::make_pair( 
		treeReader._corrMETx_JECSourcesUp.at(key), 
		treeReader._corrMETy_JECSourcesUp.at(key))} );
	    _pxy_JECSourcesDown.insert( {key, std::make_pair( 
                treeReader._corrMETx_JECSourcesDown.at(key),   
                treeReader._corrMETy_JECSourcesDown.at(key))} );
	}
    }
    if( readGroupedJECVariations ){
        for( const auto mapEl: treeReader._corrMETx_JECGroupedUp ){
            std::string key = mapEl.first;
            key = stringTools::removeOccurencesOf(key,"_corrMETx_");
            key = stringTools::removeOccurencesOf(key,"_JECGroupedUp");
            _JECGrouped.push_back( key ); // assume they are the same for Up/Down and x/y!
        }
	for( const std::string key: _JECGrouped ){
            _pxy_JECGroupedUp.insert( {key, std::make_pair( 
                treeReader._corrMETx_JECGroupedUp.at(key),  
                treeReader._corrMETy_JECGroupedUp.at(key))} );
            _pxy_JECGroupedDown.insert( {key, std::make_pair(  
                treeReader._corrMETx_JECGroupedDown.at(key),   
                treeReader._corrMETy_JECGroupedDown.at(key))} );
        }
    }
}


Met Met::variedMet( const double pt, const double phi ) const{
    Met variedMet( *this );
    variedMet.setLorentzVector( pt, eta(), phi, pt );
    return variedMet;
}

Met Met::variedMetPxPy( const double px, const double py) const{
    LorentzVector varvector = lorentzVectorPxPyPzEnergy( px, py, 0, sqrt(px*px + py*py));
    return variedMet( varvector.pt(), varvector.phi() );
}

Met Met::MetJECDown() const{
    return variedMet( _pt_JECDown, _phi_JECDown );
}


Met Met::MetJECUp() const{
    return variedMet( _pt_JECUp, _phi_JECUp );
}


Met Met::MetUnclusteredDown() const{
    return variedMet( _pt_UnclDown, _phi_UnclDown );
}


Met Met::MetUnclusteredUp() const{
    return variedMet( _pt_UnclUp, _phi_UnclUp );
}

Met Met::MetJECDown( const std::string source ) const{
    // note: this function checks both all and grouped variations,
    // need to check if there is no overlap in names between them!
    std::pair< double, double > newpxy = std::make_pair( 0,0 );
    for( std::string test: this->_JECSources ){
        if(source==test) newpxy = this->_pxy_JECSourcesDown.at(source);
    }
    for( std::string test: this->_JECGrouped ){
        if(source==test) newpxy = this->_pxy_JECGroupedDown.at(source);
    }
    return variedMetPxPy( newpxy.first, newpxy.second );
}

Met Met::MetJECUp( const std::string source ) const{
    // note: this function checks both all and grouped variations,
    // need to check if there is no overlap in names between them!
    std::pair< double, double > newpxy = std::make_pair( 0,0 );
    for( std::string test: this->_JECSources ){
        if(source==test) newpxy = this->_pxy_JECSourcesUp.at(source);
    }
    for( std::string test: this->_JECGrouped ){
        if(source==test) newpxy = this->_pxy_JECGroupedUp.at(source);
    }
    return variedMetPxPy( newpxy.first, newpxy.second );
}


std::vector< double > Met::metPtVariations() const{
    return {
        pt(),
        MetJECDown().pt(),
        MetJECUp().pt(),
        MetUnclusteredDown().pt(),
        MetUnclusteredUp().pt()
    };
}


double Met::maxPtAnyVariation() const{
    auto variations = metPtVariations();
    return *std::max_element( variations.cbegin(), variations.cend() );
}


double Met::minPtAnyVariation() const{
    auto variations = metPtVariations();
    return *std::min_element( variations.cbegin(), variations.cend() );
}


std::ostream& Met::print( std::ostream& os ) const{
    os << "Met : ";
    os << "(pt = " << pt() << ", phi = " << phi() << ")";
    os << " / pt_JECDown = " << _pt_JECDown << " / phi_JECDown = " << _phi_JECDown << " / pt_JECUp = " << _pt_JECUp << " / phi_JECUp = " << _phi_JECUp << " / pt_UnclDown = " << _pt_UnclDown << " / phi_UnclDown = " << _phi_UnclDown << " / pt_UnclUp = " << _pt_UnclUp << " / phi_UnclUp = " << _phi_UnclUp;
    return os;
}
