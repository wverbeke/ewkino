#ifndef LeptonCollection_H
#define LeptonCollection_H

//include other parts of code 
#include "../../objects/interface/Lepton.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "PhysicsObjectCollection.h"



class LeptonCollection : public PhysicsObjectCollection< Lepton > {
    
    public:
        LeptonCollection( const TreeReader& );
};



#endif
