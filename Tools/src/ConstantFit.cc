#include "../interface/ConstantFit.h" 


//include c++ library classes 
#include <limits>
#include <stdexcept>

//include ROOT classes
#include "TF1.h"

//include other parts of framework
#include "../../Tools/interface/histogramTools.h"


bool valueIsBad( const double val ){
	return ( std::isnan( val ) || std::isinf( val ) );
}


ConstantFit::ConstantFit( TH1* histPtr, const double min, const double max ){

    if( ( min < histogram::minXValue( histPtr ) ) || ( max > histogram::maxXValue( histPtr ) ) ){
	throw std::invalid_argument( "Given range for ConstantFit falls outside of the given histogram's range." );
    } 

    TF1 constFunc( "constFunc", "[0]", min, max );

    //fit the function twice (hack to improve the result in root ), in quiet mode ('Q')
    histPtr->Fit( "constFunc", "Q", "", min, max );
    histPtr->Fit( "constFunc", "Q", "", min, max );

    _value = constFunc.GetParameter( 0 ); 
    _uncertainty = constFunc.GetParError( 0 ); 
    _normalizedChi2 = constFunc.GetChisquare() / constFunc.GetNDF();

    //set high sentinel values for fit value and chi2 in case of empty data and/or failed fit
    bool emptyData = ( histPtr->GetSumOfWeights() < 1e-15 );
    bool fitFailed = ( valueIsBad( _value ) || valueIsBad( _uncertainty ) || valueIsBad( _normalizedChi2 ) );
   
    if( emptyData || fitFailed ){
        _value = std::numeric_limits< double >::max();
        _uncertainty = std::numeric_limits< double >::max();
        _normalizedChi2 = std::numeric_limits< double >::max();
    }
}


ConstantFit::ConstantFit( TH1* histPtr ):
	ConstantFit( histPtr, histogram::minXValue( histPtr ), histogram::maxXValue( histPtr ) ) {}


ConstantFit::ConstantFit( const std::shared_ptr< TH1 >& histPtr, const double min, const double max ):
	ConstantFit( histPtr.get(), min, max ) {}


ConstantFit::ConstantFit( const std::shared_ptr< TH1 >& histPtr ):
    ConstantFit( histPtr.get() ) {}
