/*
Class that does a constant fit to a histogram and stores the value, chi2 and uncertainty of the fit
*/

#ifndef ConstantFit_H
#define ConstantFit_H

//include c++ library classes
#include <memory>

//include ROOT classes 
#include "TH1.h"



class ConstantFit{

    public:
        ConstantFit() = default;
        ConstantFit( TH1* );
        ConstantFit( const std::shared_ptr< TH1 >& );
        ConstantFit( TH1*, const double min, const double max );
        ConstantFit( const std::shared_ptr< TH1 >&, const double min, const double max );

        
        double value() const{ return _value; }
        double uncertainty() const{ return _uncertainty; }
        double normalizedChi2() const{ return _normalizedChi2; }


    private:
        double _value = 0;
        double _uncertainty = 0;
        double _normalizedChi2 = 0;
};

#endif 
