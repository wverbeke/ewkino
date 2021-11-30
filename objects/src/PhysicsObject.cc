#include "../interface/PhysicsObject.h"


PhysicsObject::PhysicsObject( double transverseMomentum, double pseudoRapidity, double azimuthalAngle, double energy, const bool objectIs2016PreVFP, const bool objectIs2016PostVFP, const bool objectIs2017 ) :
    vector( LorentzVector( transverseMomentum, pseudoRapidity, azimuthalAngle, energy ) ),
    is2016PreVFPObject( objectIs2016PreVFP ),
    is2016PostVFPObject( objectIs2016PostVFP ),
    is2017Object( objectIs2017 )
    {}


void PhysicsObject::setLorentzVector( double transverseMomentum, double pseudoRapidity, double azimuthalAngle, double energyValue ){
    *this = PhysicsObject( transverseMomentum, pseudoRapidity, azimuthalAngle, energyValue, is2016PreVFPObject, is2016PostVFPObject, is2017Object );
}


PhysicsObject PhysicsObject::operator-() const{
    PhysicsObject neg = *this;
    neg.vector = -neg.vector;
    return neg;
}


PhysicsObject& PhysicsObject::operator+=( const PhysicsObject& rhs ){
    vector += rhs.vector;
    return *this;
}

    
PhysicsObject& PhysicsObject::operator-=( const PhysicsObject& rhs ){
    *this -= rhs;
    return *this;
}


PhysicsObject operator+( const PhysicsObject& lhs, const PhysicsObject& rhs ){
    PhysicsObject ret = lhs;
    ret += rhs;
    return ret;
}
    

PhysicsObject operator-( const PhysicsObject& lhs, const PhysicsObject& rhs ){
    return lhs + (-rhs);
}


double deltaEta( const PhysicsObject& lhs, const PhysicsObject& rhs ){
    return deltaEta( lhs.vector, rhs.vector );
}


double deltaPhi( const PhysicsObject& lhs, const PhysicsObject& rhs ){
    return deltaPhi( lhs.vector, rhs.vector );
}


double deltaR( const PhysicsObject& lhs, const PhysicsObject& rhs ){
    return deltaR( lhs.vector, rhs.vector );
}


double mt( const PhysicsObject& lhs, const PhysicsObject& rhs ){
    return mt( lhs.vector, rhs.vector );
}


std::ostream& PhysicsObject::print( std::ostream& os ) const{
    os << vector;
    return os;
}


std::ostream& operator<<( std::ostream& os, const PhysicsObject& physicsObject ){
    return physicsObject.print( os );
}
