#include "../interface/LorentzVector.h"

//include c++ library classes 
#include <cmath>
#include <algorithm>

void LorentzVector::setZeroValues(){

    //if there is no transverse momentum, the azimuthal angle is set to 0
    if( transverseMomentum == 0 ){
        azimuthalAngle = 0;

        //if there is no longitudinal, nor transverse momentum, the pseudorapidity is set to 0
        if( zMomentum == 0 ){
            pseudoRapidity = 0;
        } 
    }
}


void LorentzVector::normalizePhi(){

    //azimuthal angle is already in the range ]-pi, pi]
    if( fabs( azimuthalAngle ) <= M_PI ) return;

    //project azimuthal angle to correct range 
    azimuthalAngle = std::fmod( azimuthalAngle, 2*M_PI );
    if( azimuthalAngle > M_PI ){
        azimuthalAngle -= 2*M_PI;
    } else if ( azimuthalAngle < -M_PI ){
        azimuthalAngle += 2*M_PI;
    }
}


LorentzVector::LorentzVector(const double pt, const double eta, const double phi, const double energy):
    transverseMomentum(pt), pseudoRapidity(eta), azimuthalAngle(phi), energyValue( energy ),
    xMomentum( transverseMomentum*std::cos( azimuthalAngle ) ),
    yMomentum( transverseMomentum*std::sin( azimuthalAngle ) ),
    zMomentum( transverseMomentum*std::sinh( pseudoRapidity ) )
{
    setZeroValues();    
    normalizePhi();
}


double LorentzVector::mass() const{
    return std::sqrt( energyValue*energyValue - transverseMomentum*transverseMomentum - zMomentum*zMomentum );
}


LorentzVector& LorentzVector::operator+=( const LorentzVector& rhs ){

    //add the four-vectors 
    xMomentum += rhs.xMomentum;
    yMomentum += rhs.yMomentum;
    zMomentum += rhs.zMomentum;
    energyValue += rhs.energyValue;

    //compute the transverse momentum and total momentum
    transverseMomentum = std::sqrt( xMomentum*xMomentum + yMomentum*yMomentum );
    double momentumMagnitude = std::sqrt( transverseMomentum*transverseMomentum + zMomentum*zMomentum );

    //compute pseudorapidity, and avoid zero division 
    pseudoRapidity = ( zMomentum == 0 ) ? 0 : std::atanh( zMomentum/ momentumMagnitude );
        
    //compute azimuthal angle
    //default value for azimuthal angle is 0 when the transverse momentum is 0
    if( transverseMomentum == 0 ){
        azimuthalAngle = 0;

    //avoid division by zero when computing the azimuthal angle
    } else if( xMomentum == 0 ){  
        azimuthalAngle = ( yMomentum > 0 ) ? M_PI/2 : - M_PI/2;
    } else {
        azimuthalAngle = std::atan( yMomentum / xMomentum );
    }

    return *this;
}


LorentzVector operator+( const LorentzVector& lhs, const LorentzVector& rhs){
    LorentzVector ret = lhs;
    ret += rhs;
    return ret;
}


LorentzVector LorentzVector::operator-() const{
    LorentzVector neg = *this;
    neg.transverseMomentum = transverseMomentum;
    neg.pseudoRapidity = -pseudoRapidity;
    neg.azimuthalAngle = ( azimuthalAngle < 0 ) ? azimuthalAngle + M_PI : azimuthalAngle - M_PI; //transform -azimuthalAngle to range [0, 2*pi[
    neg.energyValue = -energyValue;
    neg.xMomentum = -xMomentum;
    neg.yMomentum = -yMomentum;
    neg.zMomentum = -zMomentum;

    neg.setZeroValues();
    
    return neg;
}


LorentzVector& LorentzVector::operator-=( const LorentzVector& rhs ){
    *this += ( -rhs );
    return *this;
}


LorentzVector operator-( const LorentzVector& lhs, const LorentzVector& rhs ){
    LorentzVector ret = lhs;
    ret -= rhs;
    return ret;
}


std::ostream& operator<<( std::ostream& os, const LorentzVector& rhs ){
    os << "(pT = " << rhs.transverseMomentum << ", eta = " << rhs.pseudoRapidity << ", phi = " << rhs.azimuthalAngle << ", energy = " << rhs.energyValue;
    return os;
}


double deltaEta( const LorentzVector& lhs, const LorentzVector& rhs ){
    return fabs( lhs.pseudoRapidity - rhs.pseudoRapidity );
}


double deltaPhi( const LorentzVector& lhs, const LorentzVector& rhs ){
    double dPhi = fabs( lhs.azimuthalAngle - rhs.azimuthalAngle );
    return std::min( dPhi, 2*M_PI - dPhi );
} 


double deltaR( const LorentzVector& lhs, const LorentzVector& rhs ){
    double dEta = deltaEta( lhs, rhs );
    double dPhi = deltaPhi( lhs, rhs );
    return std::sqrt( dEta*dEta + dPhi*dPhi );
}
