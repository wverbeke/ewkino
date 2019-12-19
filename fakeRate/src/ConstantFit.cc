#include "../interface/ConstantFit.h" 


//include c++ library classes 
#include <limits>
#include <stdexcept>

//include ROOT classes
#include "TF1.h"

bool valueIsBad( const double val ){
	return ( std::isnan( val ) || std::isinf( val ) );
}


double getXMin( TH1* histogram ){
	return histogram->GetBinLowEdge( 1 );
}


double getXMax( TH1* histogram ){
	return ( histogram->GetBinLowEdge( histogram->GetNbinsX() ) + histogram->GetBinWidth( histogram->GetNbinsX() ) );
}


ConstantFit::ConstantFit( TH1* histogram, const double min, const double max ){

	if( ( min < getXMin( histogram ) ) || ( max > getXMax( histogram ) ) ){
		throw std::invalid_argument( "Given range for ConstantFit falls outside of the given histogram's range." );
	} 

    TF1 constFunc( "constFunc", "[0]", min, max );

    //fit the function twice (hack to improve the result in root ), in quiet mode ('Q')
    histogram->Fit( "constFunc", "Q" );
    histogram->Fit( "constFunc", "Q" );

    _value = constFunc.GetParameter( 0 ); 
    _uncertainty = constFunc.GetParError( 0 ); 
    _normalizedChi2 = constFunc.GetChisquare() / constFunc.GetNDF();

    //set high sentinel values for fit value and chi2 in case of empty data and/or failed fit
    bool emptyData = ( histogram->GetSumOfWeights() < 1e-6 );
    bool fitFailed = ( valueIsBad( _value ) || valueIsBad( _uncertainty ) || valueIsBad( _normalizedChi2 ) );
    if( emptyData || fitFailed ){
        _value = std::numeric_limits< double >::max();
        _uncertainty = std::numeric_limits< double >::max();
        _normalizedChi2 = std::numeric_limits< double >::max();
    }
}


ConstantFit::ConstantFit( TH1* histogram ):
	ConstantFit( histogram, getXMin( histogram ), getXMax( histogram ) ) {}


ConstantFit::ConstantFit( const std::shared_ptr< TH1 >& histogram, const double min, const double max ):
	ConstantFit( histogram.get(), min, max ) {}


ConstantFit::ConstantFit( const std::shared_ptr< TH1 >& histogram ):
    ConstantFit( histogram.get() ) {}


/*
ConstantFit::ConstantFit( TH1* histogram ) :
	ConstantFit( 

   	double minBin = histogram->GetBinLowEdge( 1 );
    double maxBin = histogram->GetBinLowEdge( histogram->GetNbinsX() ) + histogram->GetBinWidth( histogram->GetNbinsX() );
    TF1 constFunc( "constFunc", "[0]", minBin, maxBin );

	//fit the function twice (hack to improve the result in root ), in quiet mode ('Q')
	histogram->Fit( "constFunc", "Q" );
	histogram->Fit( "constFunc", "Q" );

	_value = constFunc.GetParameter( 0 );
	_uncertainty = constFunc.GetParError( 0 );
	_normalizedChi2 = constFunc.GetChisquare() / constFunc.GetNDF();

	//set high sentinel values for fit value and chi2 in case of empty data and/or failed fit
    bool emptyData = ( histogram->GetSumOfWeights() < 1e-6 );
	bool fitFailed = ( valueIsBad( _value ) || valueIsBad( _uncertainty ) || valueIsBad( _normalizedChi2 ) );
    if( emptyData || fitFailed ){
        _value = std::numeric_limits< double >::max();
        _uncertainty = std::numeric_limits< double >::max();
		_normalizedChi2 = std::numeric_limits< double >::max();
    }
}


*/
