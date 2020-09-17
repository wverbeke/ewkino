#include "../interface/SampleCrossSections.h"

//include c++ library classes
#include <stdexcept>
#include <string>

//include ROOT classes
#include "TH1.h"
#include "TROOT.h"

SampleCrossSections::SampleCrossSections( const Sample& sample ){

    //open file
    std::shared_ptr< TFile > sampleFile = sample.filePtr();

    std::shared_ptr< TH1 > hCounter( dynamic_cast< TH1* >( sampleFile->Get( "blackJackAndHookers/hCounter" ) ) );
    hCounter->SetDirectory( gROOT );
    if( hCounter == nullptr ){
        throw std::invalid_argument( "hCounter is not present in file '" + sample.fileName() + "'." );
    } 

    std::shared_ptr< TH1 > lheCounter( dynamic_cast< TH1* >( sampleFile->Get( "blackJackAndHookers/lheCounter" ) ) );
    lheCounter->SetDirectory( gROOT );
    if( lheCounter == nullptr ){
        throw std::invalid_argument( "lheCounter is not present in file '" + sample.fileName() + "'." );
    } 

    std::shared_ptr< TH1 > psCounter( dynamic_cast< TH1* >( sampleFile->Get( "blackJackAndHookers/psCounter" ) ) );
    psCounter->SetDirectory( gROOT );
    if( psCounter == nullptr ){
        throw std::invalid_argument( "psCounter is not present in file '" + sample.fileName() + "'." );
    }

    double nominalSumOfWeights = hCounter->GetBinContent( 1 );

    //get the scale and pdf weight order from first event
    TreeReader treeReader;
    treeReader.initSample( sample );
    if(treeReader.numberOfEntries() > 0){
	Event event = treeReader.buildEvent(0);
	_firstScaleIndex = event.generatorInfo().firstScaleIndex(); // usually zero
	_numberOfScaleVariations = event.generatorInfo().numberOfScaleVariations(); // usually 9
	_firstPdfIndex = event.generatorInfo().firstPdfIndex(); // usually 9
	_numberOfPdfVariations = event.generatorInfo().numberOfPdfVariations(); // usually about 100
    }
 
    //store all lhe variations
    for( int bin = 1; bin < lheCounter->GetNbinsX() + 1; ++bin ){
        double lheVariedSumOfWeights = lheCounter->GetBinContent( bin );

        //0 entries indicate that a sample didn't have the respective weights
        if( lheVariedSumOfWeights < 1e-6 ) break;
        
        lheCrossSectionRatios.push_back( lheVariedSumOfWeights / nominalSumOfWeights );
    }

    //store all parton shower variations
    for( int bin = 1; bin < lheCounter->GetNbinsX() + 1; ++bin ){
        double psVariedSumOfWeights = psCounter->GetBinContent( bin );

        //0 entries indicate that a sample didn't have the respective weights
        if( psVariedSumOfWeights < 1e-6 ) break;

        psCrossSectionRatios.push_back( psVariedSumOfWeights / nominalSumOfWeights );
    }
}


double SampleCrossSections::crossSectionRatio_lheVar( const size_type index ) const{
    if( index > lheCrossSectionRatios.size() ){
        throw std::out_of_range( "Requesting lhe cross section variation " + std::to_string( index ) + " while only " + std::to_string( lheCrossSectionRatios.size() ) + " are present." );
    }
    return lheCrossSectionRatios[ index ];
}


double SampleCrossSections::crossSectionRatio_pdfVar( const size_type index ) const{
    return crossSectionRatio_lheVar( _firstPdfIndex + index );
}


double SampleCrossSections::crossSectionRatio_scaleVar( const size_type index ) const{
    return crossSectionRatio_lheVar( _firstScaleIndex + index );
}


double SampleCrossSections::crossSectionRatio_psVar( const size_type index ) const{
    if( index > psCrossSectionRatios.size() ){
        throw std::out_of_range( "Requesting ps cross section variation " + std::to_string( index ) + " while only " + std::to_string( psCrossSectionRatios.size() ) + " are present." );
    }
    return psCrossSectionRatios[ index ];
}
