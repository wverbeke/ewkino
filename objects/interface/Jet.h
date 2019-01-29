#ifndef Jet_H
#define Jet_H

//include other parts of code 
#include "PhysicsObject.h"
#include "../../TreeReader/interface/TreeReader.h"

class Jet : public PhysicsObject{
    
    public:
        Jet( const TreeReader&, const unsigned);
        //Jet( const Jet& ) = default;
        //Jet( Jet&& ) = default;

        double deepCSV() const;
        unsigned hadronFlavor() const{ return _hadronFlavor; }
        bool isLoose() const{ return _isLoose; }
        bool isTight() const{ return _isTight; }
        bool isTightLeptonVeto() const{ return _isTightLeptonVeto; }

        //analysis-specific jet selection
        //virtual bool isGood() const override;

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

        Jet variedJet(const double) const;

};
#endif
