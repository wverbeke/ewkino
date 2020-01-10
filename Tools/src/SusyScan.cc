#include "../interface/SusyScan.h"

//include other parts of code 
#include "../interface/stringTools.h"
#include "../interface/analysisTools.h"

//include c++ library classes 
#include <cmath>

//import ROOT classes 
#include "TH2D.h"


SusyScan::SusyScan( const Sample& sample ){
    if( ! analysisTools::sampleIsSusy( sample.fileName() ) ){
        throw std::invalid_argument( "Given Sample " + sample.uniqueName() + " does not correspond to a SUSY scan, so a SusyScan object can not be instantiated." );
    }
    readMassPoints_Fast(sample );
}


template < typename T > unsigned floatToUnsigned( T f ){
    return static_cast< unsigned >( std::floor( f + 0.5 ) );
}


void SusyScan::readMassPoints_Fast( const Sample& sample ){

    std::shared_ptr< TFile > file( sample.filePtr() );

    //read histogram which contains the amount of events for each susy signal
    std::shared_ptr< TH2 > susyCounter( dynamic_cast< TH2* >( file->Get( "blackJackAndHookers/hCounterSUSY" ) ) );

    //WARNING: the following code makes some assumptions about the binning (make sure this is in sync with : https://github.com/GhentAnalysis/heavyNeutrino/blob/master/multilep/src/SUSYMassAnalyzer.cc)
    //The assumption in what follows is that bin centers are integer values, if this is not the case, the code will not work!

    //internal index for keeping track of mass points 
    size_t pointIndex = 0;

    //loop over all bins, extract the mass point for each and check if there are events
    for( int xBin = 1; xBin < susyCounter->GetNbinsX() + 1; ++xBin ){
        double xCenter = susyCounter->GetXaxis()->GetBinCenter( xBin );
        unsigned massNLSP = floatToUnsigned( xCenter );
        for( int yBin = 1; yBin < susyCounter->GetNbinsY() + 1; ++yBin ){
            double yCenter = susyCounter->GetYaxis()->GetBinCenter( yBin );
            unsigned massLSP = floatToUnsigned( yCenter );

            double sumOfWeights = susyCounter->GetBinContent( xBin, yBin );

            //if there are any events for this mass point, add it to the collection
            if( sumOfWeights > 0 ){
                std::pair< unsigned, unsigned > massPair( massNLSP, massLSP );
                massesToIndices[ massPair ] = pointIndex;
                indicesToMasses[ pointIndex ] = massPair;
                indicesToSumOfWeights[ pointIndex ] = sumOfWeights;

                //next point
                ++pointIndex;
            }
        }
    }
}


std::pair< unsigned, unsigned > SusyScan::massesAtIndex( const size_t pointIndex ) const{
    auto it = indicesToMasses.find( pointIndex );
    if( it == indicesToMasses.cend() ){
        throw std::invalid_argument( "Index " + std::to_string( pointIndex ) + " does not correspond to any masses." );
    } else {
        return it->second;
    }
}


std::pair< double, double > SusyScan::masses( const size_t pointIndex ) const{
    return static_cast< std::pair< double, double > >( massesAtIndex( pointIndex ) );
}


std::string SusyScan::massesString( const size_t pointIndex ) const{
    auto masses = massesAtIndex( pointIndex );
    std::string name = "mChi2_";
    name += std::to_string( masses.first );
    name += "_mChi1_";
    name += std::to_string( masses.second );
    return name;
}


size_t SusyScan::numberOfPoints() const{
    return massesToIndices.size();
}


size_t SusyScan::index( const double mChi2, const double mChi1 ) const{
    auto it = massesToIndices.find( { floatToUnsigned( mChi2 ), floatToUnsigned( mChi1 ) } );
    if( it == massesToIndices.cend() ){
        throw std::invalid_argument( "Mass point " + std::to_string( mChi2 ) + "/" + std::to_string( mChi2 ) + " is unknown and does not correspond to an index." );
    } else {
        return it->second;
    }
}


double SusyScan::sumOfWeights( const double mChi2, const double mChi1 ) const{
    size_t i = index( mChi2, mChi1 );
    return sumOfWeights( i );
}


double SusyScan::sumOfWeights( const size_t index ) const{
    auto it = indicesToSumOfWeights.find( index );
    if( it== indicesToSumOfWeights.cend() ){
        throw std::invalid_argument( "Index " + std::to_string( index ) + " does not correspond to any known mass point." );
    } else {
        return it->second;
    }
}
