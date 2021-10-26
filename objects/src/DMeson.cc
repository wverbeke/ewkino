#include "../interface/DMeson.h"

//include c++ library classes 
#include <cmath>
#include <stdexcept>
#include <string>

//include other parts of framework
#include "../interface/DMesonSelector.h"


DMeson::DMeson( const TreeReader& treeReader, const unsigned dIndex ):
    PhysicsObject( 
        treeReader._DPt[dIndex], 
        treeReader._DEta[dIndex], 
        treeReader._DPhi[dIndex], 
        treeReader._DPt[dIndex], 
	// (to do: decide if this approximation is good enough, 
	// or if an additional variable _DE in the ntuplizer is needed.)
        treeReader.is2016(), 
        treeReader.is2017() 
    ),
    _invMass( treeReader._DInvMass[dIndex] ),
    _type( treeReader._DType[dIndex] ),
    _isolation( treeReader._DIsolation[dIndex] ),
    _dR( treeReader._DDR[dIndex] ),
    _hasFastGenMatch( treeReader._DHasFastGenMatch[dIndex] ),
    _hasGenMatch( treeReader._DHasGenMatch[dIndex] ),
    _intResMass( treeReader._DIntResMass[dIndex] ),
    _intResMassDiff( treeReader._DIntResMassDiff[dIndex] ),
    _intResX( treeReader._DIntResX[dIndex] ),
    _intResY( treeReader._DIntResY[dIndex] ),
    _intResZ( treeReader._DIntResZ[dIndex] ),
    _intResVtxNormChi2( treeReader._DIntResVtxNormChi2[dIndex] ),
    selector( new DMesonSelector( this ) )
    {}


DMeson::DMeson( const DMeson& rhs ) : 
    PhysicsObject( rhs ),
    _invMass( rhs._invMass ),
    _type( rhs._type ),
    _isolation( rhs._isolation ),
    _dR( rhs._dR ),
    _hasFastGenMatch( rhs._hasFastGenMatch ),
    _hasGenMatch( rhs._hasGenMatch ),
    _intResMass( rhs._intResMass ),
    _intResMassDiff( rhs._intResMassDiff ),
    _intResX( rhs._intResX ),
    _intResY( rhs._intResY ),
    _intResZ( rhs._intResZ ),
    _intResVtxNormChi2( rhs._intResVtxNormChi2 ),
    selector( new DMesonSelector( this ) )
    {}


DMeson::DMeson( DMeson&& rhs ) noexcept :
    PhysicsObject( std::move( rhs ) ),
    _invMass( rhs._invMass ),
    _type( rhs._type ),
    _isolation( rhs._isolation ),
    _dR( rhs._dR ),
    _hasFastGenMatch( rhs._hasFastGenMatch ),
    _hasGenMatch( rhs._hasGenMatch ),
    _intResMass( rhs._intResMass ),
    _intResMassDiff( rhs._intResMassDiff ),
    _intResX( rhs._intResX ),
    _intResY( rhs._intResY ),
    _intResZ( rhs._intResZ ),
    _intResVtxNormChi2( rhs._intResVtxNormChi2 ),
    selector( new DMesonSelector( this ) )
    {}


DMeson::~DMeson(){
    delete selector;
}


void DMeson::copyNonPointerAttributes( const DMeson& rhs ){
    _invMass = rhs._invMass;
    _type = rhs._type;
    _isolation = rhs._isolation;
    _dR = rhs._dR;
    _hasFastGenMatch = rhs._hasFastGenMatch;
    _hasGenMatch = rhs._hasGenMatch;
    _intResMass = rhs._intResMass;
    _intResMassDiff = rhs._intResMassDiff;
    _intResX = rhs._intResX;
    _intResY = rhs._intResY;
    _intResZ = rhs._intResZ;
    _intResVtxNormChi2 = rhs._intResVtxNormChi2;
}


DMeson& DMeson::operator=( const DMeson& rhs ){

    PhysicsObject::operator=(rhs);

    // note that the dmeson selector does not have to be changed, 
    // it will just keep pointing to this object 
    if( this != &rhs ){
        copyNonPointerAttributes( rhs );
    }
    return *this;
}


DMeson& DMeson::operator=( DMeson&& rhs ) noexcept {

    PhysicsObject::operator=( std::move(rhs) );

    // note that the dmeson selector does not have to be changed, 
    // it will just keep pointing to this object 
    if( this != &rhs ){
        copyNonPointerAttributes( rhs );
    }
    return *this;
}


// for selections

bool DMeson::isGood() const{
    return selector->isGood();
}

bool DMeson::passCut( double (DMeson::*property)() const,
                      double minValue, double maxValue ) const{
    return selector->passCut( property, minValue, maxValue );
}

bool DMeson::passCut( std::tuple< double (DMeson::*)() const,
                          double, double > cut ) const{
    return selector->passCut( cut );
}

bool DMeson::passCuts( std::vector< std::tuple< double (DMeson::*)() const,
                                        double, double > > cuts ) const{
    return selector->passCuts( cuts );
}


// for printing info

std::ostream& DMeson::print( std::ostream& os ) const{
    os << "DMeson : ";
    PhysicsObject::print( os );
    os << " / invMass = " << _invMass;
    os << " / type = " << _type;
    os << " / isolation = " << _isolation;
    os << " / dR = " << _dR;
    os << " / hasFastGenMatch = " << _hasFastGenMatch;
    os << " / hasGenMatch = " << _hasGenMatch;
    return os;
}
