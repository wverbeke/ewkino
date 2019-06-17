#ifndef JetCollection_H
#define JetCollection_H

//include c++ library classes 
#include <vector>
#include <memory>
#include <algorithm>

//include other parts of framework
#include "../../objects/interface/Jet.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "PhysicsObjectCollection.h"



class JetCollection : public PhysicsObjectCollection< Jet > {

    public:
        JetCollection( const TreeReader& );
};



#endif 
