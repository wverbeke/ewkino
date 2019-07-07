#ifndef Jet_H
#define Jet_H

//include other parts of code 
#include "PhysicsObject.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "JetSelector.h"


template< typename objectType > class PhysicsObjectCollection;

class Jet : public PhysicsObject{
    
    friend class PhysicsObjectCollection< Jet >;

    public:
        Jet( const TreeReader&, const unsigned);

        Jet( const Jet& );
        Jet( Jet&& ) noexcept;

        ~Jet();

        Jet& operator=( const Jet& );
        Jet& operator=( Jet&& ) noexcept;

        double deepCSV() const{ return _deepCSV; }
        unsigned hadronFlavor() const{ return _hadronFlavor; }
        bool isLoose() const{ return _isLoose; }
        bool isTight() const{ return _isTight; }
        bool isTightLeptonVeto() const{ return _isTightLeptonVeto; }

        //analysis-specific jet selection
        virtual bool isGood() const override{ return selector->isGood(); }
        bool isBTaggedLoose() const { return selector->isBTaggedLoose(); }
        bool isBTaggedMedium() const { return selector->isBTaggedMedium(); }
        bool isBTaggedTight() const { return selector->isBTaggedTight(); }

        //create new Jet with JEC varied within uncertainties
        Jet JetJECDown() const;
        Jet JetJECUp() const;

    private:
        double _deepCSV = 0;
        bool _isLoose = false;
        bool _isTight = false;
        bool _isTightLeptonVeto = false;
        unsigned _hadronFlavor = 0;
        
        //JEC uncertainties 
        double _pt_JECDown = 0;
        double _pt_JECUp = 0;

        //jet selector 
        JetSelector* selector;

        Jet variedJet(const double) const;

        Jet* clone() const & { return new Jet(*this); }
        Jet* clone() && { return new Jet( std::move( *this ) ); }

        void copyNonPointerAttributes( const Jet& );
};

#endif
