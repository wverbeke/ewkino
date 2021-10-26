#ifndef DMeson_H
#define DMeson_H

// include other parts of code 
#include "PhysicsObject.h"
#include "Track.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "../../Tools/interface/stringTools.h"

template< typename objectType > class PhysicsObjectCollection;
class DMesonSelector;

class DMeson : public PhysicsObject{
    
    friend class PhysicsObjectCollection< DMeson >;

    public:
        DMeson( const TreeReader&, const unsigned );

        DMeson( const DMeson& );
        DMeson( DMeson&& ) noexcept;

        ~DMeson();

        DMeson& operator=( const DMeson& );
        DMeson& operator=( DMeson&& ) noexcept;

        double invMass() const{ return _invMass; }
        double type() const{ return _type; }
        double isolation() const{ return _isolation; }
        double dR() const{ return _dR; }
        bool hasFastGenMatch() const{ return _hasFastGenMatch; }
        bool hasGenMatch() const{ return _hasGenMatch; }
	double intResMass() const{ return _intResMass; }
        double intResMassDiff() const{ return _intResMassDiff; }
        double intResX() const{ return _intResX; }
        double intResY() const{ return _intResY; }
        double intResZ() const{ return _intResZ; }
        double intResVtxNormChi2() const{ return _intResVtxNormChi2; }
	Track* firstTrackPtr() const{ return _firstTrackPtr; }
	Track* secondTrackPtr() const{ return _secondTrackPtr; }
	Track* thirdTrackPtr() const{ return _thirdTrackPtr; }

	// for selections
	bool isGood() const override;
	bool passCut( double (DMeson::*property)() const,
                      double minValue, double maxValue ) const;
        bool passCut( std::tuple< double (DMeson::*)() const,
                                  double, double > ) const;
        bool passCuts( std::vector< std::tuple< double (DMeson::*)() const,
                                                double, double > > ) const;
        
	// print dmeson information
        virtual std::ostream& print( std::ostream& ) const override;

    private:
        double _invMass = 0;
        double _type = 0;
        double _isolation = 0;
        double _dR = 0;
        bool _hasFastGenMatch = false;
        bool _hasGenMatch = false;
	double _intResMass = 0;
	double _intResMassDiff = 0;
	double _intResX = 0;
	double _intResY = 0;
	double _intResZ = 0;
	double _intResVtxNormChi2 = 0;
	Track* _firstTrackPtr = nullptr;
	Track* _secondTrackPtr = nullptr;
	Track* _thirdTrackPtr = nullptr;
        

        // dmeson selector 
        DMesonSelector* selector;

        DMeson* clone() const & { return new DMeson(*this); }
        DMeson* clone() && { return new DMeson( std::move( *this ) ); }

        void copyNonPointerAttributes( const DMeson& );
};

#endif
