#include "../interface/LightLepton.h"

//include c++ library classes 
#include <cmath>


LightLepton::LightLepton( const TreeReader& treeReader, const unsigned leptonIndex, LeptonSelector* leptonSelector ) :
    Lepton( treeReader, leptonIndex, leptonSelector ),
    _relIso0p3( treeReader._relIso[leptonIndex] ),
    _relIso0p4( treeReader._relIso0p4[leptonIndex] ), 
    _miniIso( treeReader._miniIso[leptonIndex] ),
    _miniIsoCharged( treeReader._miniIsoCharged[leptonIndex] ),
    _ptRatio( treeReader._ptRatio[leptonIndex] ),
    _ptRel( treeReader._ptRel[leptonIndex] ), 
    _closestJetDeepCSV( treeReader._closestJetDeepCsv_b[leptonIndex] + treeReader._closestJetDeepCsv_bb[leptonIndex] ), 
    _closestJetTrackMultiplicity( treeReader._selectedTrackMult[leptonIndex] ),

    //a way has to be found to handle this correctly for all separate datasets!
    _leptonMVA( treeReader._leptonMvatZqTTV16[leptonIndex] )
{

    //catch nan deep CSV values 
    if( std::isnan( _closestJetDeepCSV ) ){
        _closestJetDeepCSV = 0.;

    //catch default values in deep CSV
    } else if( _closestJetDeepCSV < 0. ){
        _closestJetDeepCSV = 0.;
    }
}


LightLepton::LightLepton( const LightLepton& rhs, LeptonSelector* leptonSelector ):
    Lepton( rhs, leptonSelector ),
    _relIso0p3( rhs._relIso0p3 ),
    _relIso0p4( rhs._relIso0p4 ),
    _miniIso( rhs._miniIso ),
    _miniIsoCharged( rhs._miniIsoCharged ),
    _ptRatio( rhs._ptRatio ),
    _ptRel( rhs._ptRel ),
    _closestJetDeepCSV( rhs._closestJetDeepCSV ),
    _closestJetTrackMultiplicity( rhs._closestJetTrackMultiplicity ),
    _leptonMVA( rhs._leptonMVA )
    {}


LightLepton::LightLepton( LightLepton&& rhs, LeptonSelector* leptonSelector ):
    Lepton( std::move( rhs ), leptonSelector ),
    _relIso0p3( rhs._relIso0p3 ),
    _relIso0p4( rhs._relIso0p4 ),
    _miniIso( rhs._miniIso ),
    _miniIsoCharged( rhs._miniIsoCharged ),
    _ptRatio( rhs._ptRatio ),
    _ptRel( rhs._ptRel ),
    _closestJetDeepCSV( rhs._closestJetDeepCSV ),
    _closestJetTrackMultiplicity( rhs._closestJetTrackMultiplicity ),
    _leptonMVA( rhs._leptonMVA )
    {}
