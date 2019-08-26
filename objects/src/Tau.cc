#include "../interface/Tau.h"

//include other parts of framework
#include "../interface/TauSelector.h"


Tau::Tau( const TreeReader& treeReader, const unsigned leptonIndex ) :
    Lepton( treeReader, leptonIndex, new TauSelector( this ) ),
    _decayMode( treeReader._tauDecayMode[ leptonIndex ] ),
    _passDecayModeFinding( treeReader._decayModeFinding[ leptonIndex ] ),
    _passDecayModeFindingNew( treeReader._decayModeFindingNew[ leptonIndex ] ),
    _passMuonVetoLoose( treeReader._tauMuonVetoLoose[ leptonIndex ] ),
    _passMuonVetoTight( treeReader._tauMuonVetoTight[ leptonIndex ] ),
    _passEleVetoVLoose( treeReader._tauEleVetoVLoose[ leptonIndex ] ),
    _passEleVetoLoose( treeReader._tauEleVetoLoose[ leptonIndex ] ),
    _passEleVetoMedium( treeReader._tauEleVetoMedium[ leptonIndex ] ),
    _passEleVetoTight( treeReader._tauEleVetoTight[ leptonIndex ] ),
    _passEleVetoVTight( treeReader._tauEleVetoVTight[ leptonIndex ] ),

    _passVLooseMVAOld2015( treeReader._tauPOGVLoose2015[ leptonIndex ] ),
    _passLooseMVAOld2015( treeReader._tauPOGLoose2015[ leptonIndex ] ),
    _passMediumMVAOld2015( treeReader._tauPOGMedium2015[ leptonIndex ] ),
    _passTightMVAOld2015( treeReader._tauPOGTight2015[ leptonIndex ] ),
    _passVTightMVAOld2015( treeReader._tauPOGVTight2015[ leptonIndex ] ),

    _passVLooseMVANew2015( treeReader._tauVLooseMvaNew2015[ leptonIndex ] ),
    _passLooseMVANew2015( treeReader._tauLooseMvaNew2015[ leptonIndex ] ), 
    _passMediumMVANew2015( treeReader._tauMediumMvaNew2015[ leptonIndex ] ),
    _passTightMVANew2015( treeReader._tauTightMvaNew2015[ leptonIndex ] ),
    _passVTightMVANew2015( treeReader._tauVTightMvaNew2015[ leptonIndex ] ),

    _passVVLooseMVAOld2017( treeReader._tauPOGVVLoose2017v2[ leptonIndex ] ),
    _passVLooseMVAOld2017( treeReader._lPOGVeto[ leptonIndex ] ),
    _passLooseMVAOld2017( treeReader._lPOGLoose[ leptonIndex ] ),
    _passMediumMVAOld2017( treeReader._lPOGMedium[ leptonIndex ] ),
    _passTightMVAOld2017( treeReader._lPOGTight[ leptonIndex ] ),
    _passVTightMVAOld2017( treeReader._tauPOGVTight2017v2[ leptonIndex ] ),
    _passVVTightMVAOld2017( treeReader._tauPOGVVTight2017v2[ leptonIndex ] ),

    _passVLooseMVANew2017( treeReader._tauVLooseMvaNew2017v2[ leptonIndex ] ),
    _passLooseMVANew2017( treeReader._tauLooseMvaNew2017v2[ leptonIndex ] ),
    _passMediumMVANew2017( treeReader._tauMediumMvaNew2017v2[ leptonIndex ] ),
    _passTightMVANew2017( treeReader._tauTightMvaNew2017v2[ leptonIndex ] ),
    _passVTightMVANew2017( treeReader._tauVTightMvaNew2017v2[ leptonIndex ] )
        
    {} 


Tau::Tau( const Tau& rhs ) :
    Lepton( rhs, new TauSelector( this) ),
   	_decayMode( rhs._decayMode ),
    _passDecayModeFinding( rhs._passDecayModeFinding ),
    _passDecayModeFindingNew( rhs._passDecayModeFindingNew ),
    _passMuonVetoLoose( rhs._passMuonVetoLoose ),
    _passMuonVetoTight( rhs._passMuonVetoTight ),
    _passEleVetoVLoose( rhs._passEleVetoVLoose ),
    _passEleVetoLoose( rhs._passEleVetoLoose ),
    _passEleVetoMedium( rhs._passEleVetoMedium ),
    _passEleVetoTight( rhs._passEleVetoTight ),
    _passEleVetoVTight( rhs._passEleVetoVTight ),

    _passVLooseMVAOld2015( rhs._passVLooseMVAOld2015 ),
    _passLooseMVAOld2015( rhs._passLooseMVAOld2015 ),
    _passMediumMVAOld2015( rhs._passMediumMVAOld2015 ),
    _passTightMVAOld2015( rhs._passTightMVAOld2015 ),
    _passVTightMVAOld2015( rhs._passVTightMVAOld2015 ),

    _passVLooseMVANew2015( rhs._passVLooseMVANew2015 ),
    _passLooseMVANew2015( rhs._passLooseMVANew2015 ),
    _passMediumMVANew2015( rhs._passMediumMVANew2015 ),
    _passTightMVANew2015( rhs._passTightMVANew2015 ),
    _passVTightMVANew2015( rhs._passVTightMVANew2015 ),

    _passVVLooseMVAOld2017( rhs._passVVLooseMVAOld2017 ),
    _passVLooseMVAOld2017( rhs._passVLooseMVAOld2017 ),
    _passLooseMVAOld2017( rhs._passLooseMVAOld2017 ),
    _passMediumMVAOld2017( rhs._passMediumMVAOld2017 ),
    _passTightMVAOld2017( rhs._passTightMVAOld2017 ),
    _passVTightMVAOld2017( rhs._passVTightMVAOld2017 ),
    _passVVTightMVAOld2017( rhs._passVVTightMVAOld2017 ),

    _passVLooseMVANew2017( rhs._passVLooseMVANew2017 ),
    _passLooseMVANew2017( rhs._passLooseMVANew2017 ),
    _passMediumMVANew2017( rhs._passMediumMVANew2017 ),
    _passTightMVANew2017( rhs._passTightMVANew2017 ),
    _passVTightMVANew2017( rhs._passVTightMVANew2017 )
	{}


Tau::Tau( Tau&& rhs ) noexcept :
    Lepton( std::move( rhs ), new TauSelector( this ) ),
	_decayMode( rhs._decayMode ),
    _passDecayModeFinding( rhs._passDecayModeFinding ),
    _passDecayModeFindingNew( rhs._passDecayModeFindingNew ),
    _passMuonVetoLoose( rhs._passMuonVetoLoose ),
    _passMuonVetoTight( rhs._passMuonVetoTight ),
    _passEleVetoVLoose( rhs._passEleVetoVLoose ),
    _passEleVetoLoose( rhs._passEleVetoLoose ),
    _passEleVetoMedium( rhs._passEleVetoMedium ),
    _passEleVetoTight( rhs._passEleVetoTight ),
    _passEleVetoVTight( rhs._passEleVetoVTight ),

    _passVLooseMVAOld2015( rhs._passVLooseMVAOld2015 ),
    _passLooseMVAOld2015( rhs._passLooseMVAOld2015 ),
    _passMediumMVAOld2015( rhs._passMediumMVAOld2015 ),
    _passTightMVAOld2015( rhs._passTightMVAOld2015 ),
    _passVTightMVAOld2015( rhs._passVTightMVAOld2015 ),

    _passVLooseMVANew2015( rhs._passVLooseMVANew2015 ),
    _passLooseMVANew2015( rhs._passLooseMVANew2015 ),
    _passMediumMVANew2015( rhs._passMediumMVANew2015 ),
    _passTightMVANew2015( rhs._passTightMVANew2015 ),
    _passVTightMVANew2015( rhs._passVTightMVANew2015 ),

    _passVVLooseMVAOld2017( rhs._passVVLooseMVAOld2017 ),
    _passVLooseMVAOld2017( rhs._passVLooseMVAOld2017 ),
    _passLooseMVAOld2017( rhs._passLooseMVAOld2017 ),
    _passMediumMVAOld2017( rhs._passMediumMVAOld2017 ),
    _passTightMVAOld2017( rhs._passTightMVAOld2017 ),
    _passVTightMVAOld2017( rhs._passVTightMVAOld2017 ),
    _passVVTightMVAOld2017( rhs._passVVTightMVAOld2017 ),

    _passVLooseMVANew2017( rhs._passVLooseMVANew2017 ),
    _passLooseMVANew2017( rhs._passLooseMVANew2017 ),
    _passMediumMVANew2017( rhs._passMediumMVANew2017 ),
    _passTightMVANew2017( rhs._passTightMVANew2017 ),
    _passVTightMVANew2017( rhs._passVTightMVANew2017 )
	{}


std::ostream& Tau::print( std::ostream& os ) const{
    os << "Tau : ";
    Lepton::print( os );

    //decay mocde finding 
    if( _passDecayModeFinding ){
        os << " / passes old decay mode finding";
    } 
    if( _passDecayModeFindingNew ){
        os << " / passes new decay mode finding";
    } 

    //muon veto
    if( _passMuonVetoTight ){
        os << " / pass tight muon veto";
    } else if( _passMuonVetoLoose ){
        os << " / pass loose muon veto";
    } else{
        os << " / fail muon veto";
    }

    //electron veto
    if( _passEleVetoVTight ){
        os << " / pass very tight electron veto";
    } else if( _passEleVetoTight ){
        os << " / pass tight electron veto";
    } else if( _passEleVetoMedium ){
        os << " / pass medium electron veto";
    } else if( _passEleVetoLoose ){
        os << " / pass loose electron veto";
    } else if( _passEleVetoVLoose ){
        os << " / pass very loose electron veto";
    } else {
        os << " / fail electron veto";
    }

    //old MVA 2015
    if( _passVTightMVAOld2015 ){
        os << " / pass very tight old MVA 2015";
    } else if( _passTightMVAOld2015 ){
        os << " / pass tight old MVA 2015";
    } else if( _passMediumMVAOld2015 ){
        os << " / pass medium old MVA 2015";
    } else if( _passLooseMVAOld2015 ){
        os << " / pass loose old MVA 2015";
    } else if( _passVLooseMVAOld2015 ){
        os << " / pass very loose old MVA 2015";
    } else {
        os << " / fail old MVA 2015";
    }

    //new MVA 2015
    if( _passVTightMVANew2015 ){
        os << " / pass very tight new MVA 2015";
    } else if( _passTightMVANew2015 ){
        os << " / pass tight new MVA 2015";
    } else if( _passMediumMVANew2015 ){
        os << " / pass medium new MVA 2015";
    } else if( _passLooseMVANew2015 ){
        os << " / pass loose new MVA 2015";
    } else if( _passVLooseMVANew2015 ){
        os << " / pass very loose new MVA 2015";
    } else {
        os << " / fail new MVA 2015";
    }

    //old MVA 2017
    if( _passVVTightMVAOld2017 ){
        os << " / pass very very tight old MVA 2017";
    } else if( _passVTightMVAOld2017 ){
        os << " / pass very tight old MVA 2017";
    } else if( _passTightMVAOld2017 ){
        os << " / pass tight old MVA 2017";
    } else if( _passMediumMVAOld2017 ){
        os << " / pass medium old MVA 2017";
    } else if( _passLooseMVAOld2017 ){
        os << " / pass loose old MVA 2017";
    } else if( _passVLooseMVAOld2017 ){
        os << " / pass very loose old MVA 2017";
    } else if( _passVVLooseMVAOld2017 ){
        os << " / pass very very loose old MVA 2017";
    } else {
        os << " / fail old MVA 2017";
    }

    //new MVA 2017
    if( _passVTightMVANew2017 ){
        os << " / pass very tight new MVA 2017";
    } else if( _passTightMVANew2017 ){
        os << " / pass tight new MVA 2017";
    } else if( _passMediumMVANew2017 ){
        os << " / pass medium new MVA 2017";
    } else if( _passLooseMVANew2017 ){
        os << " / pass loose new MVA 2017";
    } else if( _passVLooseMVANew2017 ){
        os << " / pass very loose new MVA 2017";
    } else {
        os << " / fail new MVA 2017";
    }

    return os;
}
