#ifndef DMesonSelector_H
#define DMesonSelector_H

// include other parts of framework
#include "DMeson.h"

class DMesonSelector{

    public:
        DMesonSelector( DMeson* dmesonPtr ) : dmesonPtr( dmesonPtr ){}
        
        bool passSelection() const{
            if( !passSelectionBase() ) return false;
            if( dmesonPtr->is2016() ) return passSelection2016();
            else if( dmesonPtr->is2017() ) return passSelection2017();
            else return passSelection2018();
        }

    private:
        DMeson* dmesonPtr;

        bool passSelectionBase() const;
        bool passSelection2016() const;
        bool passSelection2017() const;
        bool passSelection2018() const;

};

#endif
