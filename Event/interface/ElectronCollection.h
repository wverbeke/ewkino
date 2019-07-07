#ifndef ElectronCollection_H
#define ElectronCollection_H

//include other parts of code 
#include "PhysicsObjectCollection.h"
#include "../../objects/interface/Electron.h"
#include "../../TreeReader/interface/TreeReader.h"

class LeptonCollection;

class ElectronCollection : public PhysicsObjectCollection< Electron > {

    friend class LeptonCollection;

    public:
        ElectronCollection( const TreeReader& );

    private:
        ElectronCollection( const std::vector< std::shared_ptr< Electron > >& electronVector ) : PhysicsObjectCollection< Electron >( electronVector ) {}

};
#endif 
