#ifndef DMesonCollection_H
#define DMesonCollection_H

// include c++ library classes 
#include <vector>
#include <memory>
#include <algorithm>

// include other parts of framework
#include "../../objects/interface/DMeson.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "PhysicsObjectCollection.h"

class DMesonCollection : public PhysicsObjectCollection< DMeson > {

    public:
        DMesonCollection( const TreeReader& );

        // select D mesons: standard "good" selection
        void selectGoodDMesons();
        DMesonCollection goodDMesonCollection() const;

	// select D mesons: runtime specified cuts
	void selectPassingDMesons( std::vector< std::tuple< double (DMeson::*)() const,
                                   double, double > > );
	DMesonCollection passingDMesonCollection( std::vector< std::tuple< 
						  double (DMeson::*)() const,
                                                  double, double > > );

    private:
        
        // build DMesonCollection of D mesons satisfying a certain requirement
        DMesonCollection buildSubCollection( bool (DMeson::*passSelection)() const ) const;
        DMesonCollection( const std::vector< std::shared_ptr< DMeson > >& dmesonVector ) : 
	    PhysicsObjectCollection< DMeson >( dmesonVector ) {}
    
};

#endif 
