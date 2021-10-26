#include "../interface/Track.h"

//include c++ library classes 
#include <cmath>
#include <stdexcept>
#include <string>


Track::Track( double pt, double eta, double phi,
		double x, double y, double z ):
    PhysicsObject( pt, eta, phi, pt ), 
    _x( x ),
    _y( y ),
    _z( z )
    {}


Track::Track( const Track& rhs ) : 
    PhysicsObject( rhs ),
    _x( rhs._x ),
    _y( rhs._y ),
    _z( rhs._z )
    {}


Track::Track( Track&& rhs ) noexcept :
    PhysicsObject( std::move( rhs ) ),
    _x( rhs._x ),
    _y( rhs._y ),
    _z( rhs._z )
    {}


Track::~Track(){
}


void Track::copyNonPointerAttributes( const Track& rhs ){
    _x = rhs._x;
    _y = rhs._y;
    _z = rhs._z;
}


Track& Track::operator=( const Track& rhs ){

    PhysicsObject::operator=(rhs);

    // note that the dmeson selector does not have to be changed, 
    // it will just keep pointing to this object 
    if( this != &rhs ){
        copyNonPointerAttributes( rhs );
    }
    return *this;
}


Track& Track::operator=( Track&& rhs ) noexcept {

    PhysicsObject::operator=( std::move(rhs) );

    // note that the dmeson selector does not have to be changed, 
    // it will just keep pointing to this object 
    if( this != &rhs ){
        copyNonPointerAttributes( rhs );
    }
    return *this;
}


// for printing info

std::ostream& Track::print( std::ostream& os ) const{
    os << "Track : ";
    PhysicsObject::print( os );
    os << " / x = " << _x;
    os << " / y = " << _y;
    os << " / x = " << _z;
    return os;
}
