#ifndef ElectronSelector_H
#define ElectronSelector_H

//include c++ library classes 
#include <utility>

//include other parts of code 
#include "LeptonSelector.h"

class Electron;

class ElectronSelector : public LeptonSelector {
    
    public:
        ElectronSelector( const Electron* const ePtr ) : electronPtr( ePtr ) {}

        virtual bool isLoose() const override;
        virtual bool isFO() const override;
        virtual bool isGood() const override;
        virtual bool isTight() const override;

    private:
        const Electron* const electronPtr;

        virtual LeptonSelector* clone() const & override{ return new ElectronSelector( *this ); }
        virtual LeptonSelector* clone() && override{ return new ElectronSelector( std::move( *this ) ); }
};
#endif
