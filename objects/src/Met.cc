#include "../interface/Met.h"


Met::Met( const TreeReader& treeReader ):
    PhysicsObject( treeReader._met, 0., treeReader._metPhi, treeReader._met ),
    _pt_JECDown( treeReader._metJECDown ), 
    _phi_JECDown( treeReader._metPhiJECDown ),
    _pt_JECUp( treeReader._metJECUp ),
    _phi_JECUp( treeReader._metPhiJECUp ),
    _pt_UnclDown( treeReader._metUnclDown ),
    _phi_UnclDown( treeReader._metPhiUnclDown ),
    _pt_UnclUp( treeReader._metUnclUp ),
    _phi_UnclUp( treeReader._metPhiUnclUp )
    {}


Met Met::variedMet( const double pt, const double phi ) const{
    Met variedMet( *this );
    variedMet.setLorentzVector( pt, eta(), phi, pt );
    return variedMet;
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
