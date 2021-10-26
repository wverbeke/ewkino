#ifndef Track_H
#define Track_H

// include other parts of code 
#include "PhysicsObject.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Tools/interface/stringTools.h"

class Track : public PhysicsObject{
    
    public:
        Track( double pt, double eta, double phi,
		double x, double y, double z );

        Track( const Track& );
        Track( Track&& ) noexcept;

        ~Track();

        Track& operator=( const Track& );
        Track& operator=( Track&& ) noexcept;

        double x() const{ return _x; }
        double y() const{ return _y; }
        double z() const{ return _z; }

	// print dmeson information
        virtual std::ostream& print( std::ostream& ) const override;

    private:
	double _x = 0;
	double _y = 0;
	double _z = 0;
	
        Track* clone() const & { return new Track(*this); }
        Track* clone() && { return new Track( std::move( *this ) ); }

        void copyNonPointerAttributes( const Track& );
};

#endif
