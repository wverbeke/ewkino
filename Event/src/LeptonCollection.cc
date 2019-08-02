#include "../interface/LeptonCollection.h"

//include c++ library classes 

//include other parts of code 
#include "../../objects/interface/Muon.h"
#include "../../objects/interface/Electron.h"
#include "../../objects/interface/Tau.h"


LeptonCollection::LeptonCollection( const TreeReader& treeReader ){

    //add muons to lepton collection
    for( unsigned m = 0; m < treeReader._nMu; ++m){
        push_back( Muon( treeReader, m ) );
    }

    //add electrons to lepton collection
    for( unsigned e = treeReader._nMu; e < treeReader._nLight; ++ e){
        push_back( Electron( treeReader, e ) );
    } 

    //add taus to lepton collection
    for( unsigned t = treeReader._nLight; t < treeReader._nL; ++t){
        push_back( Tau( treeReader, t ) );
    }
}


MuonCollection LeptonCollection::muonCollection() const{
    std::vector< std::shared_ptr< Muon > > muonVector;
    for( const auto& leptonPtr : *this ){
        if( leptonPtr->isMuon() ){
            muonVector.push_back( std::static_pointer_cast< Muon >( leptonPtr ) );
        }
    }
    return MuonCollection( muonVector );
}


ElectronCollection LeptonCollection::electronCollection() const{
    std::vector< std::shared_ptr< Electron > > electronVector;
    for( const auto& leptonPtr : *this ){
        if( leptonPtr->isElectron() ){
            electronVector.push_back( std::static_pointer_cast< Electron >( leptonPtr ) );
        }
    }
    return ElectronCollection( electronVector );
}


TauCollection LeptonCollection::tauCollection() const{
    std::vector< std::shared_ptr< Tau > > tauVector;
    for( const auto& leptonPtr : *this  ){
        if( leptonPtr->isTau() ){
            tauVector.push_back( std::static_pointer_cast< Tau >( leptonPtr ) );
        }
    }
    return TauCollection( tauVector );
}


LightLeptonCollection LeptonCollection::lightLeptonCollection() const{
    std::vector< std::shared_ptr< LightLepton > > lightLeptonVector;
    for( const auto leptonPtr : *this ){
        if( leptonPtr->isLightLepton() ){
            lightLeptonVector.push_back( std::static_pointer_cast< LightLepton >( leptonPtr ) );
        }
    }
    return LightLeptonCollection( lightLeptonVector );
}


void LeptonCollection::selectLooseLeptons(){
    selectObjects( &Lepton::isLoose );
}


void LeptonCollection::selectFOLeptons(){
    selectObjects( &Lepton::isFO );
}


void LeptonCollection::selectTightLeptons(){
    selectObjects( &Lepton::isTight );
}


LeptonCollection LeptonCollection::selectedCollection( void (LeptonCollection::*applySelection)() ) const{
    LeptonCollection lepCol( *this );
    (lepCol.*applySelection)();
    return lepCol;
}


LeptonCollection LeptonCollection::looseLeptonCollection() const{
    return selectedCollection( &LeptonCollection::selectLooseLeptons );
}


LeptonCollection LeptonCollection::FOLeptonCollection() const{
    return selectedCollection( &LeptonCollection::selectFOLeptons );
}


LeptonCollection LeptonCollection::tightLeptonCollection() const{
    return selectedCollection( &LeptonCollection::selectTightLeptons );
}


void LeptonCollection::clean( bool (Lepton::*isFlavorToClean)() const, bool (Lepton::*isFlavorToCleanFrom)() const, bool (Lepton::*passSelection)() const, const double coneSize ){
    std::vector< const_iterator > objectsToDelete;
    for( const_iterator l1It = cbegin(); l1It != cend(); ++l1It ){
        Lepton& l1 = **l1It;
        if( (l1.*isFlavorToClean)() ){
            for( const_iterator l2It = cbegin(); l2It != cend(); ++l2It ){

                //prevent comparing same objects 
                if( l1It == l2It ) continue;

                Lepton& l2 = **l2It;

                //make sure l2 passes required selection for cleaning 
                if( ! ( (l2.*isFlavorToCleanFrom)() && (l2.*passSelection)() ) ) continue;

                //clean within given cone size
                if( deltaR( l1, l2 ) < coneSize ){
                    objectsToDelete.push_back( l1It );
                    break;
                }
            }
        }
    }
    erase( objectsToDelete );
}


void LeptonCollection::cleanElectronsFromMuons( bool (Lepton::*passSelection)() const, const double coneSize ){
    return clean( &Lepton::isElectron, &Lepton::isMuon, passSelection, coneSize );
}


void LeptonCollection::cleanTausFromLightLeptons( bool (Lepton::*passSelection)() const, const double coneSize ){
    return clean( &Lepton::isTau, &Lepton::isLightLepton, passSelection, coneSize );
}


void LeptonCollection::cleanElectronsFromLooseMuons( const double coneSize ){
    return cleanElectronsFromMuons( &Lepton::isLoose, coneSize );
}


void LeptonCollection::cleanElectronsFromFOMuons( const double coneSize ){
    return cleanElectronsFromMuons( &Lepton::isFO, coneSize );
}


void LeptonCollection::cleanTausFromLooseLightLeptons( const double coneSize ){
    return cleanTausFromLightLeptons( &Lepton::isLoose, coneSize );
}


void LeptonCollection::cleanTausFromFOLightLeptons( const double coneSize ){
    return cleanTausFromLightLeptons( &Lepton::isFO, coneSize );
}


//count the number of leptons of a given flavor
LeptonCollection::size_type LeptonCollection::numberOfLeptonsOfFlavor( bool (Lepton::*isFlavorToCount)() const ) const{
    size_type flavorCounter = 0;
    for( const auto& leptonPtr : *this ){
        if( ((*leptonPtr).*isFlavorToCount)() ){
            ++flavorCounter;
        }
    }
    return flavorCounter;
}


LeptonCollection::size_type LeptonCollection::numberOfMuons() const{
    return numberOfLeptonsOfFlavor( &Lepton::isMuon );
}


LeptonCollection::size_type LeptonCollection::numberOfElectrons() const{
    return numberOfLeptonsOfFlavor( &Lepton::isElectron );
}


LeptonCollection::size_type LeptonCollection::numberOfTaus() const{
    return numberOfLeptonsOfFlavor( &Lepton::isTau );
}


LeptonCollection::size_type LeptonCollection::numberOfLightLeptons() const{
    return numberOfLeptonsOfFlavor( &Lepton::isLightLepton );
}


//numbers signifying lepton flavor-charge combinations
enum LeptonCollection::FlavorCharge : unsigned int { OSSF_light, OSSF_tau, OS, SS, ZeroOrOneLepton };


LeptonCollection::FlavorCharge LeptonCollection::flavorChargeCombination() const{
    if( size() <= 1 ){
        return ZeroOrOneLepton;
    } 
    
        
    FlavorCharge currentState = SS;

    for( const_iterator l1It = cbegin(); l1It != cend() - 1; ++l1It ){
        Lepton& l1 = **l1It;
        for( const_iterator l2It = l1It + 1; l2It != cend(); ++l2It ){
            Lepton& l2 = **l2It;
                
            if( l1.charge() != l2.charge() ){
                currentState = OS;
        
                if( sameFlavor( l1, l2 ) && l1.isLightLepton() ){

                    //OSSF_light can be returned because it has precedence over other combinations, as it is a requirement for Z boson reconstruction
                    return OSSF_light;
                } else if( sameFlavor( l1, l2 ) && l1.isTau() ){
                    currentState = OSSF_tau;
                }
            }
        }
    }
    return currentState;
}


bool LeptonCollection::hasOSSFPair() const{
    FlavorCharge flavorCharge = flavorChargeCombination();
    return ( ( flavorCharge == OSSF_light ) || ( flavorCharge == OSSF_tau ) );
}


bool LeptonCollection::hasLightOSSFPair() const{
    return ( flavorChargeCombination() == OSSF_light );
}


bool LeptonCollection::hasOSPair() const{
    FlavorCharge flavorCharge = flavorChargeCombination();
    return ( ( flavorCharge == OSSF_light ) || ( flavorCharge == OSSF_tau ) || ( flavorCharge == OS ) );
}


bool LeptonCollection::isSameSign() const{
    return ( flavorChargeCombination() == SS );
}


std::pair< std::pair< LeptonCollection::size_type, LeptonCollection::size_type >, double > LeptonCollection::bestZBosonCandidateIndicesAndMass() const{
    static const double mZ = 91.1876;

    //currently the code only works when an OSSF pair is present 
    if( !hasLightOSSFPair() ){
        throw std::domain_error( "Finding the best leptonic Z decay candidate is only defined when two light leptons of opposite sign and same flavor are present in the event." );
    }

    //minimize mass difference with mZ over OSSF lepton pairs, and determine best mass and indices of the leptons forming this mass 
    double minDiff = std::numeric_limits< double >::max();
    double bestMass = 0;
    std::pair< size_type, size_type > indicesZCandidate = {99, 99};

    for( const_iterator l1It = cbegin(); l1It != cend() - 1; ++l1It ){
        Lepton& l1 = **l1It;
        for( const_iterator l2It = l1It + 1; l2It != cend(); ++l2It ){
            Lepton& l2 = **l2It; 

            double mass = ( l1 + l2 ).mass();
            double massDifference = ( mass - mZ );
            if( massDifference < minDiff ){
                minDiff = massDifference;
                bestMass = mass;
                indicesZCandidate = { l1It - cbegin(), l2It - cbegin() };
            }
        }
    }
    
    return { indicesZCandidate, bestMass };
}


std::pair< LeptonCollection::size_type, LeptonCollection::size_type > LeptonCollection::bestZBosonCandidateIndices() const{
    return bestZBosonCandidateIndicesAndMass().first;
}


double LeptonCollection::bestZBosonCandidateMass() const{
    return bestZBosonCandidateIndicesAndMass().second;
}
