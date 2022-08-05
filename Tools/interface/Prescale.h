/*
Very simple class that collects information about a prescale measurement
It can either be initialized from the result of a simple root fit, or a txt file containing the output of a measurement using combine
*/

#ifndef Prescale_H
#define Prescale_H

//include c++ library classes
#include <string>

//include other parts of framework
#include "ConstantFit.h"

class ConstantFit;

class Prescale{

    public:
        Prescale() = default;
        Prescale( const ConstantFit& );
        Prescale( const std::string& combineOutputFilePath );

        double value() const{ return _value; }
        double uncertaintyDown() const{ return _uncertaintyDown; }
        double uncertaintyUp() const{ return _uncertaintyUp; }
        double uncertaintySymmetric() const{ return _uncertaintySymmetric; }
    
    private:
        double _value;
        double _uncertaintyDown;
        double _uncertaintyUp;
        double _uncertaintySymmetric;
};
#endif
