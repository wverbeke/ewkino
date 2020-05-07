#ifndef ElectronSelector_H
#define ElectronSelector_H

//include c++ library classes 
#include <utility>

//include other parts of code 
#include "LeptonSelector.h"
#include "Electron.h"

class Electron;

class ElectronSelector : public LeptonSelector {
    
    public:
        ElectronSelector( const Electron* const ePtr ) : electronPtr( ePtr ) {}

    private:
        const Electron* const electronPtr;

        virtual bool isLooseBase() const override;
        virtual bool isLoose2016() const override;
        virtual bool isLoose2017() const override;
        virtual bool isLoose2018() const override;

        virtual bool isFOBase() const override;
        virtual bool isFO2016() const override;
        virtual bool isFO2017() const override;
        virtual bool isFO2018() const override;
        virtual bool isFOBasetZq() const override;
        virtual bool isFO2016tZq() const override;
        virtual bool isFO2017tZq() const override;
	virtual bool isFO2018tZq() const override;

        virtual bool isTightBase() const override;
        virtual bool isTight2016() const override;
        virtual bool isTight2017() const override;
        virtual bool isTight2018() const override;
        virtual bool isTightBasetZq() const override;
        virtual bool isTight2016tZq() const override;
        virtual bool isTight2017tZq() const override;
	virtual bool isTight2018tZq() const override;

        virtual double coneCorrection() const override;

        virtual bool is2016() const override{ return electronPtr->is2016(); }
        virtual bool is2017() const override{ return electronPtr->is2017(); }

        virtual LeptonSelector* clone() const & override{ return new ElectronSelector( *this ); }
        virtual LeptonSelector* clone() && override{ return new ElectronSelector( std::move( *this ) ); }
};

#endif
