#ifndef TauSelector_H
#define TauSelector_H

//include c++ library classes
#include <utility>

//include other parts of code 
#include "LeptonSelector.h"
#include "Tau.h"

class Tau;

class TauSelector : public LeptonSelector {

    public:
        TauSelector( const Tau* tau ) : tauPtr( tau ) {}

    private:
        const Tau* const tauPtr;

        virtual bool isLooseBase() const override;
        virtual bool isLoose2016() const override;
	virtual bool isLoose2016PreVFP() const override;
        virtual bool isLoose2016PostVFP() const override;
        virtual bool isLoose2017() const override;
        virtual bool isLoose2018() const override;

        virtual bool isFOBase() const override;
        virtual bool isFO2016() const override;
	virtual bool isFO2016PreVFP() const override;
        virtual bool isFO2016PostVFP() const override;
        virtual bool isFO2017() const override;
        virtual bool isFO2018() const override;

        virtual bool isTightBase() const override;
        virtual bool isTight2016() const override;
	virtual bool isTight2016PreVFP() const override;
        virtual bool isTight2016PostVFP() const override;
        virtual bool isTight2017() const override;
        virtual bool isTight2018() const override;

        virtual double coneCorrection() const override;

        virtual bool is2016() const override{ return tauPtr->is2016(); }
	virtual bool is2016PreVFP() const override{ return tauPtr->is2016PreVFP(); }
        virtual bool is2016PostVFP() const override{ return tauPtr->is2016PostVFP(); }
        virtual bool is2017() const override{ return tauPtr->is2017(); }
	virtual bool is2018() const override{ return tauPtr->is2018(); }

        virtual LeptonSelector* clone() const & override{ return new TauSelector( *this ); }
        virtual LeptonSelector* clone() && override{ return new TauSelector( std::move( *this ) ); }
};


#endif 
