#ifndef LeptonCollection_H
#define LeptonCollection_H

//include c++ library classes

//include other parts of code 
#include "../../objects/interface/Lepton.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "PhysicsObjectCollection.h"
#include "MuonCollection.h"
#include "ElectronCollection.h"
#include "TauCollection.h"
#include "LightLeptonCollection.h"



class LeptonCollection : public PhysicsObjectCollection< Lepton > {
    
    public:
        LeptonCollection( const TreeReader& );

        MuonCollection muonCollection() const;
        ElectronCollection electronCollection() const;
        TauCollection tauCollection() const;
        LightLeptonCollection lightLeptonCollection() const;

        //select leptons
        void selectLooseLeptons();
        void selectFOLeptons();
        void selectTightLeptons();

        //build collection of leptons passing certain selection
        LeptonCollection looseLeptonCollection() const;
        LeptonCollection FOLeptonCollection() const;
        LeptonCollection tightLeptonCollection() const;

        //clean electrons and taus 
        void cleanElectronsFromLooseMuons( const double coneSize = 0.05 );
        void cleanElectronsFromFOMuons( const double coneSize = 0.05 );
        void cleanTausFromLooseLightLeptons( const double coneSize = 0.4 );
        void cleanTausFromFOLightLeptons( const double coneSize = 0.4 );

        //apply cone correction for fake-rate prediction
        void applyConeCorrection() const;

        //number of leptons of each flavor 
        size_type numberOfMuons() const;
        size_type numberOfElectrons() const;
        size_type numberOfTaus() const;
        size_type numberOfLightLeptons() const;

        //number of leptons passing selection
        size_type numberOfLooseLeptons() const;
        size_type numberOfFOLeptons() const;
        size_type numberOfTightLeptons() const;

        bool hasOSSFPair() const;
        bool hasLightOSSFPair() const;
        bool hasOSPair() const;
        bool isSameSign() const;

        //number of unique OSSF pairs 
        size_type numberOfUniqueOSSFPairs() const;

        //number of unique OS pairs 
        size_type numberOfUniqueOSPairs() const;

        double bestZBosonCandidateMass() const;
        std::pair< size_type, size_type > bestZBosonCandidateIndices() const;
        std::pair< std::pair< size_type, size_type >, double > bestZBosonCandidateIndicesAndMass() const;


    private:

        //clean electrons and taus
        void cleanElectronsFromMuons( bool (Lepton::*passSelection)() const, const double );
        void cleanTausFromLightLeptons( bool (Lepton::*passSelection)() const, const double );
        void clean( bool (Lepton::*isFlavorToClean)() const, bool (Lepton::*isFlavorToCleanFrom)() const, bool (Lepton::*passSelection)() const, const double );

        //build collection of objects passing given selection
        LeptonCollection selectedCollection( void (LeptonCollection::*applySelection)() ) const;

        //determine the flavor + charge combination of the leptons
        enum FlavorCharge : unsigned int;
        FlavorCharge  flavorChargeCombination() const;

        //number of unique lepton pairs satisfying given condition
        template< typename function_type > size_type numberOfUniquePairs( const function_type&  ) const;
};



#endif
