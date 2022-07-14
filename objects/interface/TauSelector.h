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

        virtual bool isLooseBase_v1() const override;
        virtual bool isLoose2016_v1() const override;
	virtual bool isLoose2016PreVFP_v1() const override;
        virtual bool isLoose2016PostVFP_v1() const override;
        virtual bool isLoose2017_v1() const override;
        virtual bool isLoose2018_v1() const override;

	virtual bool isLooseBase_v2() const override;
        virtual bool isLoose2016_v2() const override;
        virtual bool isLoose2016PreVFP_v2() const override;
        virtual bool isLoose2016PostVFP_v2() const override;
        virtual bool isLoose2017_v2() const override;
        virtual bool isLoose2018_v2() const override;

        virtual bool isFOBase_v1() const override;
        virtual bool isFO2016_v1() const override;
	virtual bool isFO2016PreVFP_v1() const override;
        virtual bool isFO2016PostVFP_v1() const override;
        virtual bool isFO2017_v1() const override;
        virtual bool isFO2018_v1() const override;

	virtual bool isFOBase_v2() const override;
        virtual bool isFO2016_v2() const override;
        virtual bool isFO2016PreVFP_v2() const override;
        virtual bool isFO2016PostVFP_v2() const override;
        virtual bool isFO2017_v2() const override;
        virtual bool isFO2018_v2() const override;

	virtual bool isFORunTime(double,double,int) const override;

        virtual bool isTightBase_v1() const override;
        virtual bool isTight2016_v1() const override;
	virtual bool isTight2016PreVFP_v1() const override;
        virtual bool isTight2016PostVFP_v1() const override;
        virtual bool isTight2017_v1() const override;
        virtual bool isTight2018_v1() const override;

	virtual bool isTightBase_v2() const override;
        virtual bool isTight2016_v2() const override;
        virtual bool isTight2016PreVFP_v2() const override;
        virtual bool isTight2016PostVFP_v2() const override;
        virtual bool isTight2017_v2() const override;
        virtual bool isTight2018_v2() const override;

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
