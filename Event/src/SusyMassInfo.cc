#include "../interface/SusyMassInfo.h"


//include c++ library classes
#include <exception>


SusyMassInfo::SusyMassInfo( const TreeReader& treeReader ) :
    _massNLSP( treeReader._mChi2 ),
    _massLSP( treeReader._mChi1 )
{
    if( !treeReader.containsSusyMassInfo() ){
        throw std::runtime_error( "Can not instantiate SUSYMassInfo object since the TreeReader has no SUSY mass information." );
    }
    if( std::abs( _massLSP - 1. ) < 1e-8 ){
        _massLSP = 0.;
    }
}
