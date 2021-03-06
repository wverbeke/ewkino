#include "../interface/SusyScan.h"

//include other parts of code 
#include "../interface/stringTools.h"
#include "../interface/analysisTools.h"
#include "../interface/numericTools.h"

//include c++ library classes 
#include <cmath>

//import ROOT classes 
#include "TH2D.h"


SusyScan::SusyScan( const double massSplitting ) :
    _massSplitting( numeric::floatToUnsigned( massSplitting ) )
{}


SusyScan::SusyScan( const double minimumMassSplitting, const double maximumMassSplitting ) :
    _minimumMassSplitting( numeric::floatToUnsigned( minimumMassSplitting ) ),
    _maximumMassSplitting( numeric::floatToUnsigned( maximumMassSplitting ) )
{}


SusyScan::SusyScan( const Sample& sample, const double massSplitting ) :
    _massSplitting( numeric::floatToUnsigned( massSplitting ) )
{
    addMassPoints_Fast( sample );
}


SusyScan::SusyScan( const Sample& sample, const double minimumMassSplitting, const double maximumMassSplitting ) :
    _minimumMassSplitting( numeric::floatToUnsigned( minimumMassSplitting ) ),
    _maximumMassSplitting( numeric::floatToUnsigned( maximumMassSplitting ) )
{
    addMassPoints_Fast( sample );
}


SusyScan::SusyScan( const Sample& sample ) : SusyScan( sample, 0 ) {}


void SusyScan::addMassPoints_Fast( const Sample& sample ){

    if( ! analysisTools::sampleIsSusy( sample.fileName() ) ){
        throw std::invalid_argument( "Given Sample " + sample.uniqueName() + " does not correspond to a SUSY scan, so a SusyScan object can not be instantiated." );
    }

    std::shared_ptr< TFile > file( sample.filePtr() );

    //read histogram which contains the amount of events for each susy signal
    std::shared_ptr< TH2 > susyCounter( dynamic_cast< TH2* >( file->Get( "blackJackAndHookers/hCounterSUSY" ) ) );

    //WARNING: the following code makes some assumptions about the binning (make sure this is in sync with : https://github.com/GhentAnalysis/heavyNeutrino/blob/master/multilep/src/SUSYMassAnalyzer.cc)
    //The assumption in what follows is that bin centers are integer values, if this is not the case, the code will not work!

    //internal index for keeping track of mass points 
    //start the index at the current amount of mass points so several scans can be combined
    size_t pointIndex = numberOfPoints();

    //loop over all bins, extract the mass point for each and check if there are events
    for( int xBin = 1; xBin < susyCounter->GetNbinsX() + 1; ++xBin ){
        double xCenter = susyCounter->GetXaxis()->GetBinCenter( xBin );
        unsigned massNLSP = numeric::floatToUnsigned( xCenter );
        for( int yBin = 1; yBin < susyCounter->GetNbinsY() + 1; ++yBin ){
            double yCenter = susyCounter->GetYaxis()->GetBinCenter( yBin );
            unsigned massLSP = numeric::floatToUnsigned( yCenter );

            //consider 1 GeV LSP as massless to avoid having separate mass splittings that are 1 GeV apart
            if( massLSP == 1 ){
                massLSP = 0;
            }

            //LSP mass can not be higher than NLSP mass
            if( massLSP > massNLSP ) continue;

            //if a particular mass-splitting was required, only allow points at this splitting
            unsigned deltaM = ( massNLSP - massLSP );
            if( _massSplitting != 0 && deltaM != _massSplitting ) continue;
            if( _minimumMassSplitting != 0 || _maximumMassSplitting != 0 ){
                if( deltaM < _minimumMassSplitting || deltaM > _maximumMassSplitting ) continue;
            }

            double sumOfWeights = susyCounter->GetBinContent( xBin, yBin );

            //if there are any events for this mass point, add it to the collection
            if( sumOfWeights > 0 ){

                //check if the mass-point is present
                std::pair< unsigned, unsigned > massPair( massNLSP, massLSP );
        
                //not yet present
                if( massesToIndices.find( massPair ) == massesToIndices.cend() ){
                    massesToIndices[ massPair ] = pointIndex;
                    indicesToMasses[ pointIndex ] = massPair;
                    indicesToSumOfWeights[ pointIndex ] = sumOfWeights;

                    //next point
                    ++pointIndex;

                //already present
                } else {

                    //maps don't have to be modified in this case, only the sum of weights
                    //warning, you can't use pointIndex as the index here, that would be a bug
                    indicesToSumOfWeights[ massesToIndices[ massPair ] ] += sumOfWeights;
                }
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
    auto it = massesToIndices.find( { numeric::floatToUnsigned( mChi2 ), numeric::floatToUnsigned( mChi1 ) } );
    if( it == massesToIndices.cend() ){
        throw std::invalid_argument( "Mass point " + std::to_string( mChi2 ) + "/" + std::to_string( mChi1 ) + " is unknown and does not correspond to an index." );
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


std::vector< unsigned > SusyScan::massSplittings() const{
    std::set< unsigned > ret;
    for( size_t p = 0; p < numberOfPoints(); ++p ){
        auto massPair = massesAtIndex( p );
        ret.insert( massPair.first - massPair.second );
    }
    return std::vector< unsigned >( ret.cbegin(), ret.cend() );
}


bool SusyScan::containsMassSplitting( const double massSplitting ) const{
    unsigned dM = numeric::floatToUnsigned( massSplitting );
    return ( dM >= _minimumMassSplitting && dM <= _maximumMassSplitting );
}
