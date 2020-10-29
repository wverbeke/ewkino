#include "../interface/mt2.h"
#include "../interface/lester_mt2_bisect.h"


double mt2::mt2( const PhysicsObject& lhs, const PhysicsObject& rhs, const Met& met ){

    //invisible particles are assumed to have mass 0 (i.e. neutrinos)
    return asymm_mt2_lester_bisect::get_mT2( lhs.mass(), lhs.px(), lhs.py(), rhs.mass(), rhs.px(), rhs.py(), met.px(), met.py(), 0, 0);
}
