#include "../interface/LorentzVector.h"

//include c++ library classes 
#include <cmath>
#include <algorithm>
#include <limits>

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
    double m2 = energyValue*energyValue - transverseMomentum*transverseMomentum - zMomentum*zMomentum;
    if( m2 >= 0 ){
        return std::sqrt( m2 );
    } else {
        return - ( std::sqrt( -m2 ) );
    }
}


double LorentzVector::computeTransverseMomentum() const{
    return std::sqrt( xMomentum*xMomentum + yMomentum*yMomentum );
}


double LorentzVector::computePseudoRapidity() const{
	double momentumMagnitude = std::sqrt( transverseMomentum*transverseMomentum + zMomentum*zMomentum );
	double longitudinalMomentumFraction = zMomentum/ momentumMagnitude;

	//avoid infinite atanh when argument becomes 1
	if( fabs(longitudinalMomentumFraction) == 1. ){
		double epsilon = std::numeric_limits<double>::epsilon();
        if( longitudinalMomentumFraction > 0){
		    longitudinalMomentumFraction -= epsilon;
        } else {
            longitudinalMomentumFraction += epsilon;
        }
	}
	
	//avoid zero division
	return ( zMomentum == 0 ) ? 0 : std::atanh( longitudinalMomentumFraction );
}


double LorentzVector::computeAzimuthalAngle() const{

	//default value for azimuthal angle is 0 when the transverse momentum is 0
    if( transverseMomentum == 0 ){
        return 0;

    //avoid division by zero when computing the azimuthal angle
    } else if( xMomentum == 0 ){
        return ( yMomentum > 0 ) ? M_PI/2 : - M_PI/2;
    } else {
        double angle = std::atan( yMomentum / xMomentum );

        //take into account that atan output always lies in the range ]-pi/2, pi/2]
        if( xMomentum < 0 && yMomentum > 0 ){
            return angle + M_PI;
        } else if( xMomentum < 0 && yMomentum < 0 ){
            return angle - M_PI;
        }
		return angle;
    }
}


LorentzVector& LorentzVector::operator+=( const LorentzVector& rhs ){

    //add the four-vectors 
    xMomentum += rhs.xMomentum;
    yMomentum += rhs.yMomentum;
    zMomentum += rhs.zMomentum;
    energyValue += rhs.energyValue;

    //compute the transverse momentum and total momentum
    transverseMomentum = computeTransverseMomentum();

	//Warning: xMomentum, yMomentum, zMomentum and transverseMomentum must be computed before computing pseudoRapidity and azimuthalAngle
    //compute pseudorapidity
    pseudoRapidity = computePseudoRapidity();
        
	//compute azimuthal angle
	azimuthalAngle = computeAzimuthalAngle();

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
	neg.azimuthalAngle = -( azimuthalAngle < 0 ) ? azimuthalAngle + M_PI : azimuthalAngle - M_PI;
    neg.energyValue = -energyValue;
    neg.xMomentum = -xMomentum;
    neg.yMomentum = -yMomentum;
    neg.zMomentum = -zMomentum;
    
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
    os << "(pT = " << rhs.transverseMomentum << ", eta = " << rhs.pseudoRapidity << ", phi = " << rhs.azimuthalAngle << ", energy = " << rhs.energyValue << ")";
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


double mt( const LorentzVector& lhs, const LorentzVector& rhs ){
    return sqrt(2*lhs.pt()*rhs.pt()*( 1 - std::cos( lhs.phi()-rhs.phi() ) ) );
}


LorentzVector lorentzVectorPxPyPzEnergy( const double px, const double py, const double pz, const double energy ){
    LorentzVector ret;
    ret.xMomentum = px;
    ret.yMomentum = py;
    ret.zMomentum = pz;
    ret.energyValue = energy;
    ret.transverseMomentum = ret.computeTransverseMomentum();
    ret.pseudoRapidity = ret.computePseudoRapidity();
    ret.azimuthalAngle = ret.computeAzimuthalAngle();
    return ret;
}

