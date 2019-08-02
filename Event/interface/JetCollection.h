#ifndef JetCollection_H
#define JetCollection_H

//include c++ library classes 
#include <vector>
#include <memory>
#include <algorithm>

//include other parts of framework
#include "../../objects/interface/Jet.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "PhysicsObjectCollection.h"
#include "../../objects/interface/Lepton.h"
//#include "LeptonCollection.h"


class LeptonCollection;

class JetCollection : public PhysicsObjectCollection< Jet > {

    public:
        JetCollection( const TreeReader& );

        //make jet collection with b-tagged jets 
        JetCollection looseBTagCollection() const;
        JetCollection mediumBTagCollection() const;
        JetCollection tightBTagCollection() const;

        //select jets
        void selectGoodJets();

        JetCollection goodJetCollection() const;

        //clean jets 
        void cleanJetsFromLooseLeptons( const LeptonCollection&, const double coneSize = 0.4 );
        void cleanJetsFromFOLeptons( const LeptonCollection&, const double coneSize = 0.4 );
        void cleanJetsFromTightLeptons( const LeptonCollection&, const double coneSize = 0.4 );

    
    private:
        
        //clean jets 
        void cleanJetsFromLeptons(const LeptonCollection&, bool (Lepton::*passSelection)() const, const double coneSize );

        //build JetCollection of jets satisfying a certain requirement
        JetCollection buildSubCollection( bool (Jet::*passSelection)() const ) const;
        JetCollection( const std::vector< std::shared_ptr< Jet > >& jetVector ) : PhysicsObjectCollection< Jet >( jetVector ) {}

};

#endif 
