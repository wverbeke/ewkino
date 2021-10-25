/*
Dummy reweighter returning unity for weight, weightUp and weightDown
*/

#include "../interface/ReweighterEmpty.h"

double ReweighterEmpty::weight( const Event& event ) const{
    // dummy condition on event to avoid compilation warning
    if( event.isData() ) return 1.;
    return 1.;
}


double ReweighterEmpty::weightDown( const Event& event ) const{
    // dummy condition on event to avoid compilation warning
    if( event.isData() ) return 1.;
    return 1.;
}


double ReweighterEmpty::weightUp( const Event& event ) const{
    // dummy condition on event to avoid compilation warning
    if( event.isData() ) return 1.;
    return 1.;
}
