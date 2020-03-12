/*
Class to rebin a machine learning discriminant output (or any other discriminating variable) to bins of a certain background fraction
*/

#ifndef QuantileBinner_H
#define QuantileBinner_H


//include c++ library classes
#include <vector>
//#include <utility>
#include <memory>

//include ROOT classes
#include "TH1D.h"


class QuantileBinner {

    public:
        QuantileBinner( const TH1*, const std::vector< double >& );
        
        std::shared_ptr< TH1D > rebinnedHistogram( const TH1* ) const;
        const std::vector< double >& binBorders() const{ return _binBorders; }

    private:
        std::vector< int > _binsToMerge;
        std::vector< double > _binBorders;
        int _originalNumberOfBins;
        int _newNumberOfBins;
};



#endif