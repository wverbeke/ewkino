#ifndef PhysicsObject_H
#define PhysicsObject_H

//include c++ library classes 
#include <ostream>

//include other parts of code 
#include "LorentzVector.h"

class PhysicsObject{

    friend double deltaEta( const PhysicsObject&, const PhysicsObject& );
    friend double deltaPhi( const PhysicsObject&, const PhysicsObject& );
    friend double deltaR( const PhysicsObject&, const PhysicsObject& );
    friend double mt( const PhysicsObject&, const PhysicsObject& );

    public:
        
        PhysicsObject() = default;
        PhysicsObject( double transverseMomentum, double pseudoRapidity, 
			double azimuthalAngle, double energy, 
			const bool objectIs2016PreVFP = false, 
			const bool objectIs2016PostVFP = false,
			const bool objectIs2017 = false );

        //define both copy and move constructors and assignment operators 
        PhysicsObject( const PhysicsObject& ) = default;
        PhysicsObject( PhysicsObject&& ) noexcept = default; 

        PhysicsObject& operator=( const PhysicsObject& ) = default;
        PhysicsObject& operator=( PhysicsObject&& ) noexcept = default;


        double pt() const{ return vector.pt(); }
        double eta() const{ return vector.eta(); }
        double absEta() const{ return vector.absEta(); }
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

        bool is2016PreVFP() const{ return is2016PreVFPObject; }
	bool is2016PostVFP() const{ return is2016PostVFPObject; }
	bool is2016() const{ return (is2016PreVFP() || is2016PostVFP()); }
        bool is2017() const{ return is2017Object; }
        bool is2018() const{ return !( is2016() || is2017() ); }

        virtual ~PhysicsObject() = default;

        virtual std::ostream& print( std::ostream& os = std::cout ) const;

    protected: 

        //set the lorentzvector to new values 
        void setLorentzVector( double, double, double, double );

    private:
        LorentzVector vector; 

        bool is2016PreVFPObject = false;
	bool is2016PostVFPObject = false;
        bool is2017Object = false;

        //virtual PhysicsObject* clone() const &{ return new PhysicsObject( *this ); } 
        //virtual PhysicsObject* clone() &&{ return new PhysicsObject( std::move( *this ) ); }
};

PhysicsObject operator+( const PhysicsObject&, const PhysicsObject& );
PhysicsObject operator-( const PhysicsObject&, const PhysicsObject& );

double deltaEta( const PhysicsObject& , const PhysicsObject& );
double deltaPhi( const PhysicsObject& , const PhysicsObject& );
double deltaR( const PhysicsObject& , const PhysicsObject& );
double mt( const PhysicsObject&, const PhysicsObject& );

std::ostream& operator<<( std::ostream&, const PhysicsObject& );

#endif 
