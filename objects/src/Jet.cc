#include "../interface/Jet.h"

//include c++ library classes 
#include <cmath>
#include <stdexcept>
#include <string>

//include other parts of framework
#include "../interface/JetSelector.h"


Jet::Jet( const TreeReader& treeReader, const unsigned jetIndex ):
    PhysicsObject( treeReader._jetPt[jetIndex], treeReader._jetEta[jetIndex], treeReader._jetPhi[jetIndex], treeReader._jetE[jetIndex], treeReader.is2016(), treeReader.is2017() ),
    _deepCSV( treeReader._jetDeepCsv_b[jetIndex] + treeReader._jetDeepCsv_bb[jetIndex] ),
    _deepFlavor( treeReader._jetDeepFlavor_b[jetIndex] + treeReader._jetDeepFlavor_bb[jetIndex] + treeReader._jetDeepFlavor_lepb[jetIndex] ),
    _isTight( treeReader._jetIsTight[jetIndex] ),
    _isTightLeptonVeto( treeReader._jetIsTightLepVeto[jetIndex] ),

    //WARNING : is hadron flavor defined for jets in data?
    _hadronFlavor( treeReader._jetHadronFlavor[jetIndex] ),
    _pt_JECDown( treeReader._jetPt_JECDown[jetIndex] ),
    _pt_JECUp( treeReader._jetPt_JECUp[jetIndex] ),
    selector( new JetSelector( this ) )
{
    //catch potential invalid values of deepCSV and deepFlavor
    if( std::isnan( _deepCSV ) ){
        _deepCSV = 0.;

    //set minimum value to 0 to avoid default values
    } else if( _deepCSV < 0 ){
        _deepCSV = 0.;
    }

    if( std::isnan( _deepFlavor ) ){
        _deepFlavor = 0.;
    } else if( _deepFlavor < 0 ){
        _deepFlavor = 0.;
    }

    //check that _hadronFlavor has a known value
    if( ! ( ( _hadronFlavor == 0 ) || ( _hadronFlavor == 4 ) || ( _hadronFlavor == 5 ) ) ){
        throw std::invalid_argument( "jet hadronFlavor is '" + std::to_string( _hadronFlavor ) + "' while it should be 0, 4 or 5." );
    }
}


Jet::Jet( const Jet& rhs ) : 
    PhysicsObject( rhs ),
    _deepCSV( rhs._deepCSV ),
    _deepFlavor( rhs._deepFlavor ),
    _isTight( rhs._isTight ),
    _isTightLeptonVeto( rhs._isTightLeptonVeto ),
    _hadronFlavor( rhs._hadronFlavor ),
    _pt_JECDown( rhs._pt_JECDown ),
    _pt_JECUp( rhs._pt_JECUp ),
    selector( new JetSelector( this ) )
    {}


Jet::Jet( Jet&& rhs ) noexcept :
    PhysicsObject( std::move( rhs ) ),
	_deepCSV( rhs._deepCSV ),
    _deepFlavor( rhs._deepFlavor ),
    _isTight( rhs._isTight ),
    _isTightLeptonVeto( rhs._isTightLeptonVeto ),
    _hadronFlavor( rhs._hadronFlavor ),
    _pt_JECDown( rhs._pt_JECDown ),
    _pt_JECUp( rhs._pt_JECUp ),
	selector( new JetSelector( this ) )
{}


Jet::~Jet(){
    delete selector;
}


void Jet::copyNonPointerAttributes( const Jet& rhs ){
    _deepCSV = rhs._deepCSV;
    _deepFlavor = rhs._deepFlavor;
    _isTight = rhs._isTight;
    _isTightLeptonVeto = rhs._isTightLeptonVeto;
    _hadronFlavor = rhs._hadronFlavor;
    _pt_JECDown = rhs._pt_JECDown;
    _pt_JECUp = rhs._pt_JECUp;
}


Jet& Jet::operator=( const Jet& rhs ){

    PhysicsObject::operator=(rhs);

    //note that the jet selector does not have to be changed, it will just keep pointing to this object 
    if( this != &rhs ){
        copyNonPointerAttributes( rhs );
    }
    return *this;
}


Jet& Jet::operator=( Jet&& rhs ) noexcept {

    PhysicsObject::operator=( std::move(rhs) );

    if( this != &rhs ){
        copyNonPointerAttributes( rhs );

        //current selector can still keep pointing to this object
    }
    return *this;
}


Jet Jet::variedJet(const double newPt) const{
    Jet variedJet( *this );
    variedJet.setLorentzVector( newPt, eta(), phi(), energy()*( newPt / pt() ) );
    return variedJet;
} 


Jet Jet::JetJECDown() const{
    return variedJet( _pt_JECDown );
}


Jet Jet::JetJECUp() const{
    return variedJet( _pt_JECUp );
}


bool Jet::isGood() const{
    return selector->isGood();
}


bool Jet::inBTagAcceptance() const{
    return selector->inBTagAcceptance();
}


bool Jet::isBTaggedLoose() const{
    return selector->isBTaggedLoose();
}


bool Jet::isBTaggedMedium() const{
    return selector->isBTaggedMedium();
}


bool Jet::isBTaggedTight() const{
    return selector->isBTaggedTight();
}


std::ostream& Jet::print( std::ostream& os ) const{
    os << "Jet : ";
    PhysicsObject::print( os );
    os << " / deepCSV = " << _deepCSV << " / deepFlavor = " << _deepFlavor << " / isTight = " << _isTight << " / isTightLeptonVeto = " << _isTightLeptonVeto << " / hadronFlavor = " << _hadronFlavor << " / pt_JECDown = " << _pt_JECDown << " / pt_JECUp = " << _pt_JECUp;
    return os;
}
