#ifndef LightLeptonCollection_H
#define LightLeptonCollection_H

//include other parts of framework
#include "PhysicsObjectCollection.h"
#include "../../objects/interface/LightLepton.h"

class LeptonCollection;
class TreeReader;

class LightLeptonCollection : public PhysicsObjectCollection< LightLepton > {

    friend class LeptonCollection;

    public:
        LightLeptonCollection( const TreeReader& );

    private:
        LightLeptonCollection( const std::vector< std::shared_ptr< LightLepton > >& lightLeptonVector ) : PhysicsObjectCollection< LightLepton >( lightLeptonVector ) {}
};


#endif
