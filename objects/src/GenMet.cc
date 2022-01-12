#include "../interface/GenMet.h"

//include other parts of framework
#include "../../TreeReader/interface/TreeReader.h"


GenMet::GenMet( const TreeReader& treeReader ) :
    PhysicsObject( treeReader._gen_met, 0., treeReader._gen_metPhi, treeReader._gen_met,
		    treeReader.is2016(), treeReader.is2016PreVFP(), treeReader.is2016PostVFP(),
		    treeReader.is2017(), treeReader.is2018() )
{}
