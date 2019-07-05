#include "../interface/Lepton.h"

//include c++ library classes
#include <utility>


Lepton::Lepton( const TreeReader& treeReader, const unsigned leptonIndex, LeptonSelector* leptonSelector ) :
    PhysicsObject( treeReader._lPt[leptonIndex], treeReader._lEta[leptonIndex], treeReader._lPhi[leptonIndex], treeReader._lE[leptonIndex] ),
    _charge( treeReader._lCharge[leptonIndex] ),
    _dxy( treeReader._dxy[leptonIndex] ),
    _dz( treeReader._dz[leptonIndex] ), 
    _sip3d( treeReader._3dIPSig[leptonIndex] ),
    generatorInfo( new LeptonGeneratorInfo( treeReader, leptonIndex ) ),
    selector( leptonSelector )
    {}


//Lepton has value-like behavior 
Lepton::Lepton( const Lepton& rhs, LeptonSelector* leptonSelector ) : 
    PhysicsObject( rhs ),
    _charge( rhs._charge ),
    _dxy( rhs._dxy ),
    _dz( rhs._dz ),
    _sip3d( rhs._sip3d ),
    generatorInfo( new LeptonGeneratorInfo( *(rhs.generatorInfo) ) ),

    //WARNING: selector remains uninitialized, and has to be dynamically allocated in derived classes. Final derived copy constructur MUST CREATE A NEW SELECTOR
    //selector( nullptr )
    selector( leptonSelector )
    {} 


Lepton::Lepton( Lepton&& rhs, LeptonSelector* leptonSelector ) noexcept :
    PhysicsObject( std::move( rhs ) ),
    _charge( rhs._charge ),
    _dxy( rhs._dxy ),
    _dz( rhs._dz ),
    _sip3d( rhs._sip3d ),
    generatorInfo( rhs.generatorInfo ),

    //WARNING: selector remains uninitialized, and has to be dynamically allocated in derived classes. Final derived copy constructur MUST CREATE A NEW SELECTOR
    //selector( nullptr )
    selector( leptonSelector )
{
    rhs.generatorInfo = nullptr;
}


Lepton::~Lepton(){
    delete selector;
    if( hasGeneratorInfo() ){
        delete generatorInfo;
    }
}


void Lepton::copyNonPointerAttributes( const Lepton& rhs ){
    _charge = rhs._charge;
    _dxy = rhs._dxy;
    _dz = rhs._dz;
    _sip3d = rhs._sip3d;
}


Lepton& Lepton::operator=( const Lepton& rhs ){

    //copy the PhysicsObject part of the Lepton
    PhysicsObject::operator=(rhs); 

    //copy non pointer members
    copyNonPointerAttributes( rhs );

    //selector can keep pointing to the current lepton and does not need to be copied!
    
    //need to create new LeptonGeneratorInfo object to ensure self assignment works 
    if( hasGeneratorInfo() ){
        LeptonGeneratorInfo* oldInfo = generatorInfo;
        generatorInfo = new LeptonGeneratorInfo( *rhs.generatorInfo );
        delete oldInfo;
    }
    return *this;
}


Lepton& Lepton::operator=( Lepton&& rhs ) noexcept{

    //move the PhysicsObject part of the lepton
    PhysicsObject::operator=( std::move(rhs) );

    //in case of self assignment the move assignment should do no work
    if( this != &rhs ){

        //copy non pointer members
        copyNonPointerAttributes( rhs );

        //selector can keep pointing to the current lepton and does not need to be moved!
        //move the lepton selector
        //selector = rhs.selector;
        //rhs.selector = nullptr;

        //move pointer to generator information
        generatorInfo = rhs.generatorInfo;
        rhs.generatorInfo = nullptr;
        
    }
    return *this;
}


bool Lepton::checkGeneratorInfo() const{
    if( !hasGeneratorInfo() ){
        std::cerr << "Error: trying to access generator information for a lepton that has no generator info!" << std::endl;
        return false;
    }
    return true;
}


bool Lepton::isPrompt() const{
    if( checkGeneratorInfo() ){
        return generatorInfo->isPrompt();
    } else {
        return true;
    }
}


int Lepton::momPdgId() const{
    if( checkGeneratorInfo() ){
        return generatorInfo->momPdgId(); 
    } else {
        return 0;
    }
}


int Lepton::matchPdgId() const{
    if( checkGeneratorInfo() ){
        return generatorInfo->matchPdgId();
    } else {
        return 0;
    }
}


unsigned Lepton::provenance() const{
    if( checkGeneratorInfo() ){
        return generatorInfo->provenance();
    } else {
        return 0;
    }
}


unsigned Lepton::provenanceCompressed() const{
    if( checkGeneratorInfo() ){
        return generatorInfo->provenanceCompressed();
    } else {
        return 0;
    }
}


unsigned Lepton::provenanceConversion() const{
    if( checkGeneratorInfo() ){
        return generatorInfo->provenanceConversion();
    } else {
        return 0;
    }
}
