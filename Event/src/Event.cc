#include "../interface/Event.h"


//include other pars of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../constants/particleMasses.h"


Event::Event( const TreeReader& treeReader, const bool readIndividualTriggers , const bool readIndividualMetFilters ) : 
    _leptonCollectionPtr( new LeptonCollection( treeReader ) ),
    _jetCollectionPtr( new JetCollection( treeReader ) ),
    _metPtr( new Met( treeReader ) ),
    _triggerInfoPtr( new TriggerInfo( treeReader, readIndividualTriggers, readIndividualMetFilters ) ),
    _eventTagsPtr( new EventTags( treeReader ) ),
    _generatorInfoPtr( treeReader.isMC() ? new GeneratorInfo( treeReader ) : nullptr ),
    _numberOfVertices( treeReader._nVertex ),

    //WARNING : use treeReader::_scaledWeight instead of treeReader::_weight since the former already includes
    _weight( treeReader._scaledWeight ),
    _samplePtr( treeReader.currentSamplePtr() )
    {}


Event::~Event(){
    delete _leptonCollectionPtr;
    delete _jetCollectionPtr;
    delete _metPtr;
    delete _triggerInfoPtr;
    delete _eventTagsPtr;
    if( hasGeneratorInfo() ){
        delete _generatorInfoPtr;
    }
}


Event::Event( const Event& rhs ) :
    _leptonCollectionPtr( new LeptonCollection( *rhs._leptonCollectionPtr ) ),
    _jetCollectionPtr( new JetCollection( *rhs._jetCollectionPtr ) ),
    _metPtr( new Met( *rhs._metPtr ) ),
    _triggerInfoPtr( new TriggerInfo( *rhs._triggerInfoPtr ) ),
    _eventTagsPtr( new EventTags( *rhs._eventTagsPtr ) ),
    _generatorInfoPtr( rhs.hasGeneratorInfo() ? new GeneratorInfo( *rhs._generatorInfoPtr ) : nullptr ),
    _numberOfVertices( rhs._numberOfVertices ),
    _weight( rhs._weight ),
    _samplePtr( rhs._samplePtr )
    {}


Event::Event( Event&& rhs ) noexcept :
    _leptonCollectionPtr( rhs._leptonCollectionPtr ),
    _jetCollectionPtr( rhs._jetCollectionPtr ),
    _metPtr( rhs._metPtr ),
    _triggerInfoPtr( rhs._triggerInfoPtr ),
    _eventTagsPtr( rhs._eventTagsPtr ),
    _generatorInfoPtr( rhs._generatorInfoPtr ),
    _numberOfVertices( rhs._numberOfVertices ),
    _weight( rhs._weight ),
    _samplePtr( rhs._samplePtr )
{
    rhs._leptonCollectionPtr = nullptr;
    rhs._jetCollectionPtr = nullptr;
    rhs._metPtr = nullptr;
    rhs._triggerInfoPtr = nullptr;
    rhs._eventTagsPtr = nullptr;
    rhs._generatorInfoPtr = nullptr;
    rhs._samplePtr = nullptr;
}
    

Event& Event::operator=( const Event& rhs ){
    if( this != &rhs ){
        delete _leptonCollectionPtr;
        delete _jetCollectionPtr;
        delete _metPtr;
        delete _triggerInfoPtr;
        delete _eventTagsPtr;
        if( hasGeneratorInfo() ){
            delete _generatorInfoPtr;
        }

        _leptonCollectionPtr = new LeptonCollection( *rhs._leptonCollectionPtr );
        _jetCollectionPtr = new JetCollection( *rhs._jetCollectionPtr );
        _metPtr = new Met( *rhs._metPtr );
        _triggerInfoPtr = new TriggerInfo( *rhs._triggerInfoPtr );
        _eventTagsPtr = new EventTags( *rhs._eventTagsPtr );
        _generatorInfoPtr = rhs.hasGeneratorInfo() ? new GeneratorInfo( *rhs._generatorInfoPtr ) : nullptr;

        _numberOfVertices = rhs._numberOfVertices;
        _weight = rhs._weight;
        _samplePtr = rhs._samplePtr;
    }
    return *this;
}


Event& Event::operator=( Event&& rhs ) noexcept{
    if( this != &rhs ){
        delete _leptonCollectionPtr;
        delete _jetCollectionPtr;
        delete _metPtr;
        delete _triggerInfoPtr;
        delete _eventTagsPtr;
        if( hasGeneratorInfo() ){
            delete _generatorInfoPtr;
        }

        _leptonCollectionPtr = rhs._leptonCollectionPtr;
        rhs._leptonCollectionPtr = nullptr;
        _jetCollectionPtr = rhs._jetCollectionPtr;
        rhs._jetCollectionPtr = nullptr;
        _metPtr = rhs._metPtr;
        rhs._metPtr = nullptr;
        _triggerInfoPtr = rhs._triggerInfoPtr;
        rhs._triggerInfoPtr = nullptr;
        _eventTagsPtr = rhs._eventTagsPtr;
        rhs._eventTagsPtr = nullptr;
        _generatorInfoPtr = rhs._generatorInfoPtr;
        rhs._generatorInfoPtr = nullptr;

        _numberOfVertices = rhs._numberOfVertices;
        _weight = rhs._weight;
        _samplePtr = rhs._samplePtr;
    }
    return *this;
}


void Event::checkGeneratorInfo() const{
    if( !hasGeneratorInfo() ){
        std::domain_error( "Trying to access generator information for a data event!" );
    }
}


GeneratorInfo& Event::generatorInfo() const{
    checkGeneratorInfo();
    return *_generatorInfoPtr;
}


void Event::initializeZBosonCandidate(){
    if( !ZIsInitialized ){

        //check that there are at least two leptons is performed automatically in LeptonCollection
        
        //leading lepton not used in this pairing is considered to be from the W decay (in trilepton events )
        //BUT in order to have consistent indices, sort leptons by pT already here.
        sortLeptonsByPt();

        //reconstruct the best Z boson
        std::pair< std::pair< LeptonCollection::size_type, LeptonCollection::size_type >, double > ZBosonCandidateIndicesAndMass = _leptonCollectionPtr->bestZBosonCandidateIndicesAndMass();
        _bestZBosonCandidateIndices = ZBosonCandidateIndicesAndMass.first;
        _bestZBosonCandidateMass = ZBosonCandidateIndicesAndMass.second;

        //note that the third lepton can also be a tau in this case!
        if( numberOfLeptons() >= 3 ){
            for( LeptonCollection::size_type leptonIndex = 0; leptonIndex < numberOfLeptons(); ++leptonIndex ){
                if( !( leptonIndex == _bestZBosonCandidateIndices.first || leptonIndex == _bestZBosonCandidateIndices.second ) ){
                    _WLeptonIndex = leptonIndex;
                    break;
                }
            }
        }

        ZIsInitialized = true;
    }
}


std::pair< std::pair< LeptonCollection::size_type, LeptonCollection::size_type >, double > Event::bestZBosonCandidateIndicesAndMass(){
    initializeZBosonCandidate();    
    return { _bestZBosonCandidateIndices, _bestZBosonCandidateMass };
}


std::pair< LeptonCollection::size_type, LeptonCollection::size_type > Event::bestZBosonCandidateIndices(){
    initializeZBosonCandidate();
    return _bestZBosonCandidateIndices;
}


double Event::bestZBosonCandidateMass(){
    initializeZBosonCandidate();
    return _bestZBosonCandidateMass;
}


bool Event::hasZTollCandidate( const double oneSidedMassWindow ){
    initializeZBosonCandidate();
    return ( fabs( bestZBosonCandidateMass() - particle::mZ ) < oneSidedMassWindow );
}


LeptonCollection::size_type Event::WLeptonIndex(){
    initializeZBosonCandidate();
    return _WLeptonIndex;
}


double Event::mtW(){
    initializeZBosonCandidate();
    return mt( lepton( WLeptonIndex() ), met() );
}
