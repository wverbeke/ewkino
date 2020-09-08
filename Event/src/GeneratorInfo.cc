#include "../interface/GeneratorInfo.h"

//include c++ library classes 
#include <stdexcept>

std::map< std::string,unsigned > getWeightIndices( const TreeReader& treeReader ){
    // dirty sample-by-sample hard-coded parameters...
    // see https://twiki.cern.ch/twiki/bin/viewauth/CMS/HowToPDF
    std::map< std::string,unsigned > resMap;
    // case 1: 2016 ZGToLLG sample
    if( stringTools::stringContains( treeReader.currentSamplePtr()->fileName(),
            std::string("ZGToLLG_01J_5f_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8") ) ){
	// the order of the pdf and scale variations is swapped. 
	// moreover, the scale variations give nonsensical results so ignore for now.
        resMap["firstScaleIndex"] = 104;
        resMap["numberOfScaleVariations"] = 0;
        resMap["firstPdfIndex"] = 1;
        resMap["numberOfPdfVariations"] = 103;
        // (to be checked by someone who knows more about this than I do...)
	return resMap;
    }
    // default case for most samples
    resMap["firstScaleIndex"] = 0;
    resMap["numberOfScaleVariations"] = std::min( treeReader._nLheWeights, unsigned(9) );
    resMap["firstPdfIndex"] = 9;
    resMap["numberOfPdfVariations"] = std::min( std::max( treeReader._nLheWeights, unsigned(9) ) - 9, unsigned(100));
    return resMap;
}

GeneratorInfo::GeneratorInfo( const TreeReader& treeReader ) :
    _numberOfLheWeights( treeReader._nLheWeights ),
    _numberOfPsWeights( treeReader._nPsWeights ),
    _prefireWeight( treeReader._prefireWeight ),
    _prefireWeightDown( treeReader._prefireWeightDown ),
    _prefireWeightUp( treeReader._prefireWeightUp ),
    _ttgEventType( treeReader._ttgEventType ),
    _zgEventType( treeReader._zgEventType ),
    _partonLevelHT( treeReader._lheHTIncoming ),
    _numberOfTrueInteractions( treeReader._nTrueInt ),
    _genMetPtr( new GenMet( treeReader ) )
{ 
    if( _numberOfLheWeights > 148 ){
        throw std::out_of_range( "_numberOfLheWeights is larger than 148, which is the maximum array size of _lheWeights." );
    }
    for( unsigned i = 0; i < _numberOfLheWeights; ++i  ){
        _lheWeights[i] = treeReader._lheWeight[i];
    }

    if( _numberOfPsWeights > maxNumberOfPsWeights ){
        throw std::out_of_range( "_numberOfPsWeights is larger than 14, which is the maximum array size of _psWeights." );
    }
    for( unsigned i = 0; i < _numberOfPsWeights; ++i ){
        _psWeights[i] = treeReader._psWeight[i];
    }

    //prefire weights are not defined for 2018 events, set them to unity
    if( treeReader.is2018() ){
        _prefireWeight = 1.;
        _prefireWeightDown = 1.;
        _prefireWeightUp = 1.;
    }

    // get addtional parameters to read the weights correctly
    // note: this is temporary in expectation of a good way to read the lhe weights correctly in the ntuplizer
    // instead of simply reading the first min(148,_nLheWeight)
    std::map< std::string,unsigned > paramMap = getWeightIndices(treeReader);
    //std::cout << "-- creating Event.GeneratorInfo, do printouts for testing --" << std::endl;
    //std::cout << "sample name: " << treeReader.currentSamplePtr()->fileName() << std::endl;
    //std::cout << "first scale index: " << paramMap["firstScaleIndex"] << std::endl;
    //std::cout << "number of scale variations: " << paramMap["numberOfScaleVariations"] << std::endl;
    //std::cout << "first pdf index: " << paramMap["firstPdfIndex"] << std::endl;
    //std::cout << "number of pdf variations: " << paramMap["numberOfPdfVariations"] << std::endl;
    //std::cout << "-- end printouts from Event/src/GeneratorInfo --" << std::endl;
    _firstScaleIndex = paramMap["firstScaleIndex"];
    _numberOfScaleVariations = paramMap["numberOfScaleVariations"];
    _firstPdfIndex = paramMap["firstPdfIndex"];
    _numberOfPdfVariations = paramMap["numberOfPdfVariations"]; 
}

double retrieveWeight( const double* array, const unsigned index, const unsigned startindex, 
			const unsigned nindices, const std::string& name ){
    if( index > nindices ){
        throw std::out_of_range( "Only " + std::to_string(nindices) + " " + name 
		+ " variations are available, and an index larger or equal than " + std::to_string(nindices) + " is requested." );
    }
    return array[ index + startindex ];
}


double GeneratorInfo::relativeWeightPdfVar( const unsigned pdfIndex ) const{
    return retrieveWeight( _lheWeights, pdfIndex, _firstPdfIndex, _numberOfPdfVariations, "pdf" );
}


double GeneratorInfo::relativeWeightScaleVar( const unsigned scaleIndex ) const{
    return retrieveWeight( _lheWeights, scaleIndex, _firstScaleIndex, _numberOfScaleVariations, "scale" );
}


double GeneratorInfo::relativeWeightPsVar( const unsigned psIndex ) const{
    return retrieveWeight( _psWeights, psIndex, 0, std::min( _numberOfPsWeights, unsigned(14) ), "parton shower" ); 
}
