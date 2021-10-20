#ifndef DMeson_H
#define DMeson_H

// include other parts of code 
#include "PhysicsObject.h"
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

	// for selections
	bool isGood() const override;
        
	// print dmeson information
        virtual std::ostream& print( std::ostream& ) const override;

    private:
        double _invMass = 0;
        double _type = 0;
        double _isolation = 0;
        double _dR = 0;
        bool _hasFastGenMatch = false;
        bool _hasGenMatch = false;
        
        // dmeson selector 
        DMesonSelector* selector;

        DMeson* clone() const & { return new DMeson(*this); }
        DMeson* clone() && { return new DMeson( std::move( *this ) ); }

        void copyNonPointerAttributes( const DMeson& );
};

#endif
