#ifndef DMesonCollection_H
#define DMesonCollection_H

//include c++ library classes 
#include <vector>
#include <memory>
#include <algorithm>

//include other parts of framework
#include "../../objects/interface/DMeson.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "PhysicsObjectCollection.h"

class DMesonCollection : public PhysicsObjectCollection< DMeson > {

    public:
        DMesonCollection( const TreeReader& );

        //select dmesons
        void selectGoodDMesons();
        DMesonCollection goodDMesonCollection() const;

    private:
        
        //build DMesonCollection of dmesons satisfying a certain requirement
        DMesonCollection buildSubCollection( bool (DMeson::*passSelection)() const ) const;
        DMesonCollection( const std::vector< std::shared_ptr< DMeson > >& dmesonVector ) : PhysicsObjectCollection< DMeson >( dmesonVector ) {}
    
};

#endif 
