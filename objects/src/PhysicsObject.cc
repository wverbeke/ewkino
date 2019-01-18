#include "../interface/PhysicsObject.h"


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
