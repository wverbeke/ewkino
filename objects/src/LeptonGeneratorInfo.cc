#include "../interface/LeptonGeneratorInfo.h"

LeptonGeneratorInfo::LeptonGeneratorInfo( const TreeReader& treeReader, const unsigned leptonIndex ):
    _isPrompt( treeReader._lIsPrompt[leptonIndex] ),
    _matchPdgId( treeReader._lMatchPdgId[leptonIndex] ),
    _matchCharge( treeReader._lMatchCharge[leptonIndex] ),
    _momPdgId( treeReader._lMomPdgId[leptonIndex] ),
    _provenance( treeReader._lProvenance[leptonIndex] ), 
    _provenanceCompressed( treeReader._lProvenanceCompressed[leptonIndex] ),
    _provenanceConversion( treeReader._lProvenanceConversion[leptonIndex] )
{
    if( treeReader.hasBranchWithName( "_lMomPt" ) ){
	_momPt = treeReader._lMomPt[leptonIndex];
    }
}
