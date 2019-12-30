#ifndef LeptonSelectionHelper_H
#define LeptonSelectionHelper_H

//include other parts of framework
#include "../../objects/interface/Lepton.h"



class LeptonSelectionHelper{
    
    public:
        virtual bool passSelection( const Lepton& ) const = 0;
};



#endif
