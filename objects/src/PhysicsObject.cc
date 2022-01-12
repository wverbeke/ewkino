#include "../interface/PhysicsObject.h"


PhysicsObject::PhysicsObject( double transverseMomentum, double pseudoRapidity, 
				double azimuthalAngle, double energy, 
				const bool objectIs2016,
				const bool objectIs2016PreVFP,
				const bool objectIs2016PostVFP,
				const bool objectIs2017,
				const bool objectIs2018 ) :
    vector( LorentzVector( transverseMomentum, pseudoRapidity, azimuthalAngle, energy ) )
{
    // check if at least one era boolean is true
    if( !(objectIs2016 || objectIs2016PreVFP || objectIs2016PostVFP 
	|| objectIs2017 || objectIs2018) ){
	std::string msg = "ERROR in PhysicsObject constructor: no valid data taking year was found.";
	throw std::runtime_error( msg );
    }
    is2016Object = objectIs2016;
    is2016PreVFPObject = objectIs2016PreVFP;
    is2016PostVFPObject = objectIs2016PostVFP;
    is2017Object = objectIs2017;
    is2018Object = objectIs2018;
}


void PhysicsObject::setLorentzVector( double transverseMomentum, double pseudoRapidity, 
					double azimuthalAngle, double energyValue ){
    *this = PhysicsObject( transverseMomentum, pseudoRapidity, azimuthalAngle, energyValue, 
		is2016Object, is2016PreVFPObject, is2016PostVFPObject, is2017Object, is2018Object );
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
