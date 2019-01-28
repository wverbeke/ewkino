#ifndef ElectronSelector_H
#define ElectronSelector_H

//include other parts of code 
#include "LeptonSelector.h"

class Electron;

class ElectronSelector : public LeptonSelector {
    
    public:
        ElectronSelector( const Electron& electron ) : electronPtr( &electron ) {}

        virtual bool isLoose() const override;
        virtual bool isGood() const override;
        virtual bool isTight() const override;

    private:
        const Electron* const electronPtr;

};
#endif
