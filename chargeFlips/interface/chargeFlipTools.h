
//include c++ library classes
#include <memory>

//include ROOT classes
#include "TH2D.h"

//include other parts of framework
#include "../../Event/interface/Event.h"

namespace chargeFlips{
    double chargeFlipWeight( const Event&, const std::shared_ptr< TH2 >& );
}
