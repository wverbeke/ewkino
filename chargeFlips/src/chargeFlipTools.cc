#include "../interface/chargeFlipTools.h"

//include other parts of framework
#include "../../Tools/interface/histogramTools.h"


double chargeFlips::chargeFlipWeight( const Event& event, const std::shared_ptr< TH2 >& chargeFlipMap ){

    // P( A + B ) = P( A ) + P( B ) - P( A & B )
    double summedProbabilities = 0.;
    double multipliedProbabilities = 1.;
    for( const auto& electronPtr : event.electronCollection() ){

		double flipRate = histogram::contentAtValues( chargeFlipMap.get(), electronPtr->pt(), electronPtr->absEta() );
        summedProbabilities += flipRate / ( 1. - flipRate );
        multipliedProbabilities *= flipRate / ( 1. - flipRate );
    }
    return ( summedProbabilities - multipliedProbabilities );
}
