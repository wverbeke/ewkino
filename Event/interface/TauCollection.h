#ifndef TauCollection_H
#define TauCollection_H

//include other parts of framework
#include "PhysicsObjectCollection.h"
#include "../../objects/interface/Tau.h"
#include "../../TreeReader/interface/TreeReader.h"

class LeptonCollection;

class TauCollection : PhysicsObjectCollection< Tau > {

    friend class LeptonCollection;
    
    public:
        TauCollection( const TreeReader& );

    private:
        TauCollection( const std::vector< std::shared_ptr< Tau > >& tauVector ) : PhysicsObjectCollection< Tau >( tauVector ) {}
};
#endif
