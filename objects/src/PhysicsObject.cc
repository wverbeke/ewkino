#include "../interface/PhysicsObject.h"


void PhysicsObject::setLorentzVector( double transverseMomentum, double pseudoRapidity, double azimuthalAngle, double energyValue ){
    *this = PhysicsObject( transverseMomentum, pseudoRapidity, azimuthalAngle, energyValue );
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
