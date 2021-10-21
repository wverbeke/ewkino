#ifndef DMesonSelector_H
#define DMesonSelector_H

// include other parts of framework
#include "DMeson.h"

class DMesonSelector{

    public:
        DMesonSelector( DMeson* dmesonPtr ) : dmesonPtr( dmesonPtr ){}
        
        bool isGood() const{
            if( !isGoodBase() ) return false;
            if( dmesonPtr->is2016() ) return isGood2016();
            else if( dmesonPtr->is2017() ) return isGood2017();
            else return isGood2018();
        }

    private:
        DMeson* dmesonPtr;

        bool isGoodBase() const;
        bool isGood2016() const;
        bool isGood2017() const;
        bool isGood2018() const;

};

#endif