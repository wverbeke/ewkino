#ifndef mt2_H
#define mt2_H

/*
Wrapper to compute mt2ll, calling the code from https://arxiv.org/abs/1411.4312, which is included in this directory as 'lester_mt2_bisect.h'
Even though its short, do the computation in a separate cc file to avoid having to recompile the included algorithm each time.
*/

#include "../../objects/interface/PhysicsObject.h"
#include "../../objects/interface/Met.h"

namespace mt2{
    double mt2( const PhysicsObject&, const PhysicsObject&, const Met& );
}

#endif 
