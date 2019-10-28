#include "../interface/Prescale.h"


//include other parts of framework
#include "../interface/ConstantFit.h"


Prescale::Prescale( const ConstantFit& fit) :
    _value( fit.value() ), _uncertaintyDown( fit.uncertainty() ), _uncertaintyUp( fit.uncertainty() ), _uncertaintySymmetric( fit.uncertainty() )
{}
