#ifndef Lepton_H
#define Lepton_H

//include c++ library classes 
#include <utility>
#include <memory>

//include other parts of code 
#include "PhysicsObject.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "LeptonGeneratorInfo.h"
#include "LeptonSelector.h"

//class LeptonSelector;
template< typename ObjectType > class PhysicsObjectCollection;

class Lepton : public PhysicsObject {
    
    //The leptonCollection class represents a collection of leptons. Its base (PhysicsObjectCollection<Lepton>) should be a friend to be able to use clone().
    friend class PhysicsObjectCollection<Lepton>;
    
    public: 
        Lepton( const TreeReader&, const unsigned, LeptonSelector* ); 
        Lepton( const Lepton& );
        Lepton( Lepton&& ) noexcept;

        virtual ~Lepton();
        
        Lepton& operator=( const Lepton& );
        Lepton& operator=( Lepton&& );

        //get reconstructed lepton attributes 
        double dxy() const{ return _dxy; }
        double dz() const{ return _dz; }
        double sip3d() const{ return _sip3d; }
        int charge() const{ return _charge; }

        //access generator-level attributes
        bool isPrompt() const;
        int matchPdgId() const;
        int momPdgId() const;
        unsigned provenance() const;
        unsigned provenanceCompressed() const;
        unsigned provenanceConversion() const;

        //lepton id decisions 
        bool isLoose() const{ return selector->isLoose(); }
        bool isFO() const{ return selector->isFO(); }
        bool isGood() const{ return selector->isGood(); }
        bool isTight() const{ return selector->isTight(); }

        //check what type of lepton the object is
        virtual bool isMuon() const = 0;
        virtual bool isElectron() const = 0;
        virtual bool isTau() const = 0;
        virtual bool isLightLepton() const = 0;


    private:

        int _charge = 0;
    
        //lepton impact parameter variables 
        double _dxy = 0;
        double _dz = 0;
        double _sip3d = 0;

        //lepton generator information
        LeptonGeneratorInfo* generatorInfo = nullptr;

        //lepton selector object 
        LeptonSelector* selector;

        //check whether generator-level info was initialized 
        bool hasGeneratorInfo() const{ return  generatorInfo != nullptr; }
        bool checkGeneratorInfo() const;

        //copy non-pointer attributes from other leptons, to be used in copy operations
        void copyNonPointerAttributes( const Lepton& );

        //functions to facilitate dynamically typed dynamic memory allocation 
        virtual Lepton* clone() const & = 0;
        virtual Lepton* clone() && = 0;
};
#endif
