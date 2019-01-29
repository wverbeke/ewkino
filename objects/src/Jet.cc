#include "../interface/Jet.h"

//include c++ library classes 
#include <cmath>


Jet::Jet( const TreeReader& treeReader, const unsigned jetIndex ):
    PhysicsObject( treeReader._jetPt[jetIndex], treeReader._jetEta[jetIndex], treeReader._jetPhi[jetIndex], treeReader._jetE[jetIndex] ),
    _deepCSV( treeReader._jetDeepCsv_b[jetIndex] + treeReader._jetDeepCsv_bb[jetIndex] ),
    _isLoose( treeReader._jetIsLoose[jetIndex] ),
    _isTight( treeReader._jetIsTight[jetIndex] ),
    _isTightLeptonVeto( treeReader._jetIsTightLepVeto[jetIndex] ),
    _hadronFlavor( treeReader._jetHadronFlavor[jetIndex] ),
    _pt_JECDown( treeReader._jetPt_JECDown[jetIndex] ),
    _pt_JECUp( treeReader._jetPt_JECUp[jetIndex] )
{
    //catch potential invalid values of deepCSV 
    if( std::isnan( _deepCSV ) ){
        _deepCSV = 0.;

    //set minimum value to 0 to avoid default values
    } else if( _deepCSV < 0 ){
        _deepCSV = 0.;
    }
}


Jet Jet::variedJet(const double newPt) const{
    Jet variedJet = *this;
    variedJet.setLorentzVector( newPt, eta(), phi(), energy()*( newPt / pt() ) );
    return variedJet;
} 


Jet Jet::JetJECDown() const{
    return variedJet( _pt_JECDown );
}


Jet Jet::JetJECUp() const{
    return variedJet( _pt_JECUp );
}
