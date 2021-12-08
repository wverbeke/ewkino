#include "../interface/SlidingCut.h"


SlidingCut::SlidingCut( const double minX, const double maxX, const double leftCut, const double rightCut ):
    _minX( minX ), _maxX( maxX ), _leftCut( leftCut ), _rightCut( rightCut )
{}


double SlidingCut::cut( const double x ) const{
    if( x <= _minX ){
        return _leftCut;
    } else if( x >= _maxX ){
        return _rightCut;
    } else {
        return _leftCut - ( _leftCut - _rightCut ) * ( x - _minX ) / ( _maxX - _minX );
    }
}


std::string SlidingCut::name( const std::string& xName, const std::string& cutName ) const{
    std::string cutRangeName = cutName + std::to_string( _leftCut )+ "To" + std::to_string( _rightCut );
    std::string xRangeName = xName + std::to_string( _minX ) + "To" + std::to_string( _maxX );
    return "slidingCut_" + cutRangeName + "_for_" + xRangeName;
}


SlidingCutCollection::SlidingCutCollection( const double minX, const double maxX, const double minCut, const double maxCut, const double granularity ){

    double leftCut = minCut;
    while( leftCut <= maxCut ){
        double rightCut = minCut;
        while( rightCut <= maxCut ){
            _collection.emplace_back( SlidingCut( minX, maxX, leftCut, rightCut ) );
            rightCut += granularity;
        }
        leftCut += granularity;
    }
}
