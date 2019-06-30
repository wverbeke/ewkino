#ifndef TauSelector_H
#define TauSelector_H

//include c++ library classes
#include <utility>

//include other parts of code 
#include "LeptonSelector.h"

class Tau;

class TauSelector : public LeptonSelector {

    public:
        TauSelector( const Tau* tau ) : tauPtr( tau ) {}

        virtual bool isLoose() const override;
        virtual bool isFO() const override;
        virtual bool isGood() const override;
        virtual bool isTight() const override;

    private:
        const Tau* const tauPtr;

        virtual LeptonSelector* clone() const & override{ return new TauSelector( *this ); }
        virtual LeptonSelector* clone() && override{ return new TauSelector( std::move( *this ) ); }

};


#endif 
