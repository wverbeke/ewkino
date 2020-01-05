#include "../interface/ReweighterPrefire.h"

double ReweighterPrefire::weight( const Event& event ) const{
    return event.generatorInfo().prefireWeight();
}


double ReweighterPrefire::weightDown( const Event& event ) const{
    return event.generatorInfo().prefireWeightDown();
}


double ReweighterPrefire::weightUp( const Event& event ) const{
    return event.generatorInfo().prefireWeightUp();
}
