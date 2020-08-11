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
        virtual bool isLoose2017() const override;
        virtual bool isLoose2018() const override;

        virtual bool isFOBasetZq() const override;
        virtual bool isFO2016tZq() const override;
        virtual bool isFO2017tZq() const override;
        virtual bool isFO2018tZq() const override;
        virtual bool isFOBasettH() const override;
        virtual bool isFO2016ttH() const override;
        virtual bool isFO2017ttH() const override;
        virtual bool isFO2018ttH() const override;
        virtual bool isFOBaseOldtZq() const override;
        virtual bool isFO2016OldtZq() const override;
        virtual bool isFO2017OldtZq() const override;
        virtual bool isFO2018OldtZq() const override;

        virtual bool isTightBasetZq() const override;
        virtual bool isTight2016tZq() const override;
        virtual bool isTight2017tZq() const override;
        virtual bool isTight2018tZq() const override;
        virtual bool isTightBasettH() const override;
        virtual bool isTight2016ttH() const override;
        virtual bool isTight2017ttH() const override;
        virtual bool isTight2018ttH() const override;
        virtual bool isTightBaseOldtZq() const override;
        virtual bool isTight2016OldtZq() const override;
        virtual bool isTight2017OldtZq() const override;
        virtual bool isTight2018OldtZq() const override;

        virtual double coneCorrection() const override;

        virtual bool is2016() const override{ return tauPtr->is2016(); }
        virtual bool is2017() const override{ return tauPtr->is2017(); }

        virtual LeptonSelector* clone() const & override{ return new TauSelector( *this ); }
        virtual LeptonSelector* clone() && override{ return new TauSelector( std::move( *this ) ); }
};


#endif 
