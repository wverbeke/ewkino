#include "../interface/QuantileBinner.h"

//include c++ library classes
#include <numeric>
#include <stdexcept>
#include <string>
#include <algorithm>


QuantileBinner::QuantileBinner( const TH1* originalHist, const std::vector< double >& contentFractions ){

    //check if content fractions approximately sum to one
    double fractionSum = std::accumulate( contentFractions.cbegin(), contentFractions.cend(), 0. );
    if( std::abs( ( fractionSum - 1. ) / fractionSum  ) > 1e-6 ){
        throw std::invalid_argument( "Sum of given content fractions is " + std::to_string( fractionSum ) + ", while it should be 1." );
    }

    double totalContent = originalHist->GetSumOfWeights();

    double currentSum = 0.;
    size_t quantileCounter = 0;
    for( int bin = originalHist->GetNbinsX(); bin != 0; --bin ){
    
        if( quantileCounter == ( contentFractions.size() - 1 ) ){
            break;  
        }

        currentSum += originalHist->GetBinContent( bin );

        if( ( currentSum / totalContent ) > contentFractions[ contentFractions.size() - quantileCounter - 1 ] ){
            _binsToMerge.push_back( bin );
            currentSum = 0.;
            ++quantileCounter;
        }
    }

    for( int bin : _binsToMerge ){
        _binBorders.push_back( originalHist->GetBinLowEdge( bin ) );
    }
    std::reverse( _binBorders.begin(), _binBorders.end() );

    _originalNumberOfBins = originalHist->GetNbinsX();
    _newNumberOfBins = contentFractions.size();
}


std::shared_ptr< TH1D > QuantileBinner::rebinnedHistogram( const TH1* histogramPtr ) const{

    if( histogramPtr->GetNbinsX() != _originalNumberOfBins ){
        throw std::invalid_argument( "Given histogram has " + std::to_string( histogramPtr->GetNbinsX() ) + " bins, while the QuantileBinner has been initialized using a histogram with " + std::to_string( _originalNumberOfBins ) + " bins." );
    }

    std::string xTitle = histogramPtr->GetXaxis()->GetTitle();
    std::string yTitle = "Events";
    std::string histogramName = histogramPtr->GetName();

    std::shared_ptr< TH1D > ret = std::make_shared< TH1D >( ( histogramName + "_transFormed" ).c_str(), ( histogramName + ";" + xTitle + ";" + yTitle ).c_str(), _newNumberOfBins, 0., 1. );

    for( int bin = 1; bin < _newNumberOfBins + 1; ++bin ){

        //determine range of old bins corresponding to this new bin
        int lowBinIndex; //inclusive
        int highBinIndex; //exclusive

        if( bin == 1 ){
            lowBinIndex = 1;
            highBinIndex = _binsToMerge.back();
        } else if( bin == _newNumberOfBins ){
            lowBinIndex = _binsToMerge.front();
            highBinIndex = _originalNumberOfBins + 1;
        } else {
            lowBinIndex = _binsToMerge[ _binsToMerge.size() - bin + 1 ];
            highBinIndex = _binsToMerge[ _binsToMerge.size() - bin ];
        }

        //set the new bin content and error
        double binContent = 0.;
        double binErrorSquared = 0.;
        for( int oldBin = lowBinIndex; oldBin < highBinIndex; ++oldBin ){
            binContent += histogramPtr->GetBinContent( oldBin );
            double oldBinError = histogramPtr->GetBinError( oldBin );
            binErrorSquared += ( oldBinError * oldBinError );
        }
        ret->SetBinContent( bin, binContent );
        ret->SetBinError( bin, std::sqrt( binErrorSquared ) );
    }
    return ret;
}
