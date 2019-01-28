#ifndef TauSelector_H
#define TauSelector_H

//include other parts of code 
#include "LeptonSelector.h"

class Tau;

class TauSelector : public LeptonSelector {

    public:
        TauSelector( const Tau& tau ) : tauPtr( &tau ) {}

    private:
        const Tau* const tauPtr;

};


#endif 
