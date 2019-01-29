#ifndef PhysicsObject_H
#define PhysicsObject_H

//include c++ library classes 

//include other parts of code 
#include "LorentzVector.h"

class PhysicsObject{

    public:
        
        PhysicsObject() = default;
        PhysicsObject( double transverseMomentum, double pseudoRapidity, double azimuthalAngle, double energy ):
            vector( LorentzVector( transverseMomentum, pseudoRapidity, azimuthalAngle, energy ) ) {} 

        //define both copy and move constructors and assignment operators 
        PhysicsObject( const PhysicsObject& ) = default;
        PhysicsObject( PhysicsObject&& ) = default; 

        PhysicsObject& operator=( const PhysicsObject& ) = default;
        PhysicsObject& operator=( PhysicsObject&& ) = default;


        double pt() const{ return vector.pt(); }
        double eta() const{ return vector.eta(); }
        double phi() const{ return vector.phi(); }
        double energy() const{ return vector.energy(); }
        double mass() const{ return vector.mass(); }
        double px() const{ return vector.px(); }
        double py() const{ return vector.py(); }
        double pz() const{ return vector.pz(); }

        virtual bool isGood() const{ return true; }

        PhysicsObject operator-() const;
        PhysicsObject& operator+=( const PhysicsObject& );
        PhysicsObject& operator-=( const PhysicsObject& );

        virtual ~PhysicsObject() = default;

    protected: 

        //set the lorentzvector to new values 
        void setLorentzVector(double, double, double, double);

    private:
        LorentzVector vector; 

};

PhysicsObject operator+(const PhysicsObject&, const PhysicsObject&);
PhysicsObject operator-(const PhysicsObject&, const PhysicsObject&);
#endif 
