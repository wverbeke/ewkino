#include "../interface/Event.h"


//include other pars of framework
#include "../../TreeReader/interface/TreeReader.h"
#include "../../constants/particleMasses.h"


Event::Event( const TreeReader& treeReader, 
		const bool readIndividualTriggers , const bool readIndividualMetFilters,
		const bool readAllJECVariations, const bool readGroupedJECVariations,
		const bool readDCandidates ):
    // make collections of physics objects
    _leptonCollectionPtr( new LeptonCollection( treeReader ) ),
    _jetCollectionPtr( new JetCollection( treeReader,
			readAllJECVariations, readGroupedJECVariations ) ),
    _dmesonCollectionPtr( readDCandidates ? new DMesonCollection( treeReader ) : nullptr ),
    _metPtr( new Met( treeReader,
			readAllJECVariations, readGroupedJECVariations ) ),
    // make additional information structures
    _triggerInfoPtr( new TriggerInfo( treeReader, 
			readIndividualTriggers, readIndividualMetFilters ) ),
    _jetInfoPtr( new JetInfo( treeReader, 
			readAllJECVariations, readGroupedJECVariations ) ),
    _eventTagsPtr( new EventTags( treeReader ) ),
    _generatorInfoPtr( treeReader.isMC() ? new GeneratorInfo( treeReader ) : nullptr ),
    _htowdGenInfoPtr( treeReader.isMC() ? new HToWDGenInfo( treeReader ) : nullptr ),
    _susyMassInfoPtr( treeReader.isSusy() ? new SusyMassInfo( treeReader ) : nullptr ),
    _numberOfVertices( treeReader._nVertex ),

    //WARNING : use treeReader::_scaledWeight instead of treeReader::_weight since the former already includes cross-section and lumiosity scaling
    _weight( treeReader._scaledWeight ),
    _samplePtr( treeReader.currentSamplePtr() )
    {}


Event::~Event(){
    delete _leptonCollectionPtr;
    delete _jetCollectionPtr;
    if( hasDMesonCollection() ) delete _dmesonCollectionPtr;
    delete _metPtr;
    delete _triggerInfoPtr;
    delete _jetInfoPtr;
    delete _eventTagsPtr;
    if( hasGeneratorInfo() ){
        delete _generatorInfoPtr;
	delete _htowdGenInfoPtr;
    }
    if( hasSusyMassInfo() ){
        delete _susyMassInfoPtr;
    }
}


Event::Event( const Event& rhs ) :
    _leptonCollectionPtr( new LeptonCollection( *rhs._leptonCollectionPtr ) ),
    _jetCollectionPtr( new JetCollection( *rhs._jetCollectionPtr ) ),
    _dmesonCollectionPtr( rhs.hasDMesonCollection() ? 
			  new DMesonCollection( *rhs._dmesonCollectionPtr ) : 
			  nullptr ),
    _metPtr( new Met( *rhs._metPtr ) ),
    _triggerInfoPtr( new TriggerInfo( *rhs._triggerInfoPtr ) ),
    _jetInfoPtr( new JetInfo( *rhs._jetInfoPtr ) ),
    _eventTagsPtr( new EventTags( *rhs._eventTagsPtr ) ),
    _generatorInfoPtr( rhs.hasGeneratorInfo() ? new GeneratorInfo( *rhs._generatorInfoPtr ) : nullptr ),
    _htowdGenInfoPtr( rhs.hasGeneratorInfo() ? new HToWDGenInfo( *rhs._htowdGenInfoPtr ) : nullptr ),
    _susyMassInfoPtr( rhs.hasSusyMassInfo() ? new SusyMassInfo( *rhs._susyMassInfoPtr ) : nullptr ),
    _numberOfVertices( rhs._numberOfVertices ),
    _weight( rhs._weight ),
    _samplePtr( rhs._samplePtr )
    {}


Event::Event( Event&& rhs ) noexcept :
    _leptonCollectionPtr( rhs._leptonCollectionPtr ),
    _jetCollectionPtr( rhs._jetCollectionPtr ),
    _dmesonCollectionPtr( rhs._dmesonCollectionPtr ),
    _metPtr( rhs._metPtr ),
    _triggerInfoPtr( rhs._triggerInfoPtr ),
    _jetInfoPtr( rhs._jetInfoPtr ),
    _eventTagsPtr( rhs._eventTagsPtr ),
    _generatorInfoPtr( rhs._generatorInfoPtr ),
    _htowdGenInfoPtr( rhs._htowdGenInfoPtr ),
    _susyMassInfoPtr( rhs._susyMassInfoPtr ),
    _numberOfVertices( rhs._numberOfVertices ),
    _weight( rhs._weight ),
    _samplePtr( rhs._samplePtr )
{
    rhs._leptonCollectionPtr = nullptr;
    rhs._jetCollectionPtr = nullptr;
    rhs._dmesonCollectionPtr = nullptr;
    rhs._metPtr = nullptr;
    rhs._triggerInfoPtr = nullptr;
    rhs._jetInfoPtr = nullptr;
    rhs._eventTagsPtr = nullptr;
    rhs._generatorInfoPtr = nullptr;
    rhs._htowdGenInfoPtr = nullptr;
    rhs._susyMassInfoPtr = nullptr;
    rhs._samplePtr = nullptr;
}
    

Event& Event::operator=( const Event& rhs ){
    if( this != &rhs ){
        delete _leptonCollectionPtr;
        delete _jetCollectionPtr;
	if( hasDMesonCollection() ) delete _dmesonCollectionPtr;
        delete _metPtr;
        delete _triggerInfoPtr;
	delete _jetInfoPtr;
        delete _eventTagsPtr;
        if( hasGeneratorInfo() ){
            delete _generatorInfoPtr;
	    delete _htowdGenInfoPtr;
        }
        if( hasSusyMassInfo() ){
            delete _susyMassInfoPtr;
        }

        _leptonCollectionPtr = new LeptonCollection( *rhs._leptonCollectionPtr );
        _jetCollectionPtr = new JetCollection( *rhs._jetCollectionPtr );
	_dmesonCollectionPtr = rhs.hasDMesonCollection() ? 
			       new DMesonCollection( *rhs._dmesonCollectionPtr ) :
			       nullptr;
        _metPtr = new Met( *rhs._metPtr );
        _triggerInfoPtr = new TriggerInfo( *rhs._triggerInfoPtr );
	_jetInfoPtr = new JetInfo( *rhs._jetInfoPtr );
        _eventTagsPtr = new EventTags( *rhs._eventTagsPtr );
        _generatorInfoPtr = rhs.hasGeneratorInfo() ? new GeneratorInfo( *rhs._generatorInfoPtr ) : nullptr;
	_htowdGenInfoPtr = rhs.hasGeneratorInfo() ? new HToWDGenInfo( *rhs._htowdGenInfoPtr ) : nullptr;
        _susyMassInfoPtr = rhs.hasSusyMassInfo() ? new SusyMassInfo( *rhs._susyMassInfoPtr ) : nullptr;

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
	if( hasDMesonCollection() ) delete _dmesonCollectionPtr;
        delete _metPtr;
        delete _triggerInfoPtr;
	delete _jetInfoPtr;
        delete _eventTagsPtr;
        if( hasGeneratorInfo() ){
            delete _generatorInfoPtr;
	    delete _htowdGenInfoPtr;
        }
        if( hasSusyMassInfo() ){
            delete _susyMassInfoPtr;
        }

        _leptonCollectionPtr = rhs._leptonCollectionPtr;
        rhs._leptonCollectionPtr = nullptr;
        _jetCollectionPtr = rhs._jetCollectionPtr;
        rhs._jetCollectionPtr = nullptr;
	_dmesonCollectionPtr = rhs._dmesonCollectionPtr;
	rhs._dmesonCollectionPtr = nullptr;
        _metPtr = rhs._metPtr;
        rhs._metPtr = nullptr;
        _triggerInfoPtr = rhs._triggerInfoPtr;
        rhs._triggerInfoPtr = nullptr;
	_jetInfoPtr = rhs._jetInfoPtr;
	rhs._jetInfoPtr = nullptr;
        _eventTagsPtr = rhs._eventTagsPtr;
        rhs._eventTagsPtr = nullptr;
        _generatorInfoPtr = rhs._generatorInfoPtr;
        rhs._generatorInfoPtr = nullptr;
	_htowdGenInfoPtr = rhs._htowdGenInfoPtr;
	rhs._htowdGenInfoPtr = nullptr;
        _susyMassInfoPtr = rhs._susyMassInfoPtr;
        rhs._susyMassInfoPtr = nullptr;

        _numberOfVertices = rhs._numberOfVertices;
        _weight = rhs._weight;
        _samplePtr = rhs._samplePtr;
    }
    return *this;
}


void Event::checkGeneratorInfo() const{
    if( !hasGeneratorInfo() ){
        throw std::domain_error( "Trying to access generator information for a data event!" );
    }
}


GeneratorInfo& Event::generatorInfo() const{
    checkGeneratorInfo();
    return *_generatorInfoPtr;
}

HToWDGenInfo& Event::htowdGenInfo() const{
    checkGeneratorInfo();
    return *_htowdGenInfoPtr;
}

void Event::checkSusyMassInfo() const{
    if( !hasSusyMassInfo() ){
        throw std::domain_error( "Trying to access SUSY mass info for a non-SUSY event!" );
    }
}


SusyMassInfo& Event::susyMassInfo() const{
    checkSusyMassInfo();
    return *_susyMassInfoPtr;
}


void Event::checkDMesonCollection() const{
    if( !hasDMesonCollection() ){
	throw std::domain_error( "Trying to access D meson collection which is not present!" );
    }
}


DMesonCollection& Event::dmesonCollection() const{
    checkDMesonCollection();
    return *_dmesonCollectionPtr;
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
    return mt( WLepton(), met() );
}

// copy event with modified energy/momentum scales
// experimental stage! need to check what attributes are copied or modified in-place!

void Event::setLeptonCollection( const LeptonCollection& lepCollection ){
    _leptonCollectionPtr = new LeptonCollection( lepCollection);
}

Event Event::variedLeptonCollectionEvent(
		    LeptonCollection (LeptonCollection::*variedCollection)() const ) const{
    Event newevt = Event( *this );
    LeptonCollection lepCollection = (this->leptonCollection().*variedCollection)();
    //newevt._leptonCollectionPtr = new LeptonCollection( lepCollection );
    // (the above does not work since _leptonCollectionPtr is private,
    //  try to solve by defining a public function doing the same thing, but not optimal.)
    newevt.setLeptonCollection( lepCollection );  
    return newevt;
}

Event Event::electronScaleUpEvent() const{
    return variedLeptonCollectionEvent( &LeptonCollection::electronScaleUpCollection );
}

Event Event::electronScaleDownEvent() const{
    return variedLeptonCollectionEvent( &LeptonCollection::electronScaleDownCollection );
}

Event Event::electronResUpEvent() const{
    return variedLeptonCollectionEvent( &LeptonCollection::electronResUpCollection );
}

Event Event::electronResDownEvent() const{
    return variedLeptonCollectionEvent( &LeptonCollection::electronResDownCollection );
}

