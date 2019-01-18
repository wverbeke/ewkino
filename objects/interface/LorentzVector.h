#ifndef LorentzVector_H
#define LorentzVector_H

//include c++ library classes 
#include <iostream>

class LorentzVector{

    //non-class members that are part of the interface 
    friend std::ostream& operator<<( std::ostream&, const LorentzVector& ); 

    friend double deltaEta( const LorentzVector&, const LorentzVector& );
    friend double deltaPhi( const LorentzVector&, const LorentzVector& );
    friend double deltaR( const LorentzVector&, const LorentzVector& );

    public:
        LorentzVector() = default;
        LorentzVector( const double pt, const double eta, const double phi, const double energy );
        
        double pt() const { return transverseMomentum; }
        double eta() const { return pseudoRapidity; }
        double phi() const { return azimuthalAngle; }
        double energy() const { return energyValue; }
        double mass() const;
        
        double px() const { return xMomentum; }
        double py() const { return yMomentum; }
        double pz() const { return zMomentum; }

        LorentzVector& operator+=( const LorentzVector& rhs );
        LorentzVector& operator-=( const LorentzVector& rhs );
        LorentzVector operator-() const;

    private:
        double transverseMomentum = 0;
        double pseudoRapidity = 0;
        double azimuthalAngle = 0;
        double energyValue = 0;

        double xMomentum = 0;
        double yMomentum = 0;
        double zMomentum = 0;

        //set default values for azimuthal angle and pseudorapidity when the momenta of the Lorentz vector are 0
        void setZeroValues();

        //make sure phi is in the interval ]-pi, pi]
        void normalizePhi();

        //compute pseudorapidity given px, py and pz
        double computePseudoRapidity() const;

        //compute azimuthal angle given px and py
        double computeAzimuthalAngle() const;
};

LorentzVector operator+( const LorentzVector&, const LorentzVector& );
LorentzVector operator-( const LorentzVector&, const LorentzVector& );
std::ostream& operator<<( std::ostream&, const LorentzVector& );

double deltaEta( const LorentzVector&, const LorentzVector& );
double deltaPhi( const LorentzVector&, const LorentzVector& );
double deltaR( const LorentzVector&, const LorentzVector& );

#endif
