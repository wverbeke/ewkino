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
    _closestJetDeepFlavor( treeReader._closestJetDeepFlavor_b[leptonIndex] + treeReader._closestJetDeepFlavor_bb[leptonIndex] + treeReader._closestJetDeepFlavor_lepb[leptonIndex] ),
    _closestJetTrackMultiplicity( treeReader._selectedTrackMult[leptonIndex] ),
    _leptonMVAtZq( treeReader._leptonMvatZq[leptonIndex] ),
    _leptonMVAttH( treeReader._leptonMvaTTH[leptonIndex] ),
    _leptonMVATOP( treeReader._leptonMvaTOP[leptonIndex] ),
    _leptonMVATOPUL( treeReader._leptonMvaTOPUL[leptonIndex] ),
    _leptonMVATOPv2UL( treeReader._leptonMvaTOPv2UL[leptonIndex] )
{

    //catch nan deep CSV values 
    if( std::isnan( _closestJetDeepCSV ) ){
        _closestJetDeepCSV = 0.;

    //catch default values in deep CSV
    } else if( _closestJetDeepCSV < 0. ){
        _closestJetDeepCSV = 0.;
    }

    //catch nan deep Flavor values
    if( std::isnan( _closestJetDeepFlavor ) ){
        _closestJetDeepFlavor = 0.;

    //catch default values in deep Flavor
    } else if( _closestJetDeepFlavor < 0. ){
        _closestJetDeepFlavor = 0.;
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
    _closestJetDeepFlavor( rhs._closestJetDeepFlavor ),
    _closestJetTrackMultiplicity( rhs._closestJetTrackMultiplicity ),
    _leptonMVAtZq( rhs._leptonMVAtZq ),
    _leptonMVAttH( rhs._leptonMVAttH ),
    _leptonMVATOP( rhs._leptonMVATOP ),
    _leptonMVATOPUL( rhs._leptonMVATOPUL ),
    _leptonMVATOPv2UL( rhs._leptonMVATOPv2UL )
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
    _closestJetDeepFlavor( rhs._closestJetDeepFlavor ),
    _closestJetTrackMultiplicity( rhs._closestJetTrackMultiplicity ),
    _leptonMVAtZq( rhs._leptonMVAtZq ),
    _leptonMVAttH( rhs._leptonMVAttH ),
    _leptonMVATOP( rhs._leptonMVATOP ),
    _leptonMVATOPv2UL( rhs._leptonMVATOPv2UL )
    {}


std::ostream& LightLepton::print( std::ostream& os ) const{
    Lepton::print( os );
    os << " relIso 0.3 = " << _relIso0p3; 
    os << " / relIso 0.4 = " << _relIso0p4;
    os << " / miniIso = " << _miniIso;
    os << " / miniIsoCharged = " << _miniIsoCharged;
    os << " / ptRatio = " << _ptRatio;
    os << " / ptRel = " << _ptRel;
    os << " / closestJetDeepCSV = " << _closestJetDeepCSV;
    os << " / closestJetDeepFlavor = " << _closestJetDeepFlavor;
    os << " / closestJetTrackMultiplicity = " << _closestJetTrackMultiplicity;
    os << " / leptonMVAtZq = " << _leptonMVAtZq;
    os << " / leptonMVAttH = " << _leptonMVAttH;
    os << " / leptonMVATOP = " << _leptonMVATOP;
    os << " / leptonMVATOPUL = " << _leptonMVATOPUL;
    os << " / leptonMVATOPv2UL = " << _leptonMVATOPv2UL;
    return os;
}
