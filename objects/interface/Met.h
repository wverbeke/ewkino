#ifndef Met_H
#define Met_H

//include other parts of code 
#include "PhysicsObject.h"
#include "../../TreeReader/interface/TreeReader.h"

class Met : public PhysicsObject {

    public:
        Met() = default;
        Met( const TreeReader& );

        virtual bool isGood() const override{ return true; }

        //met objects varied by respective uncertainties
        Met MetJECDown() const;
        Met MetJECUp() const;
        Met MetUnclusteredDown() const;
        Met MetUnclusteredUp() const;

        //maximum variations of met pT
        double maxPtAnyVariation() const;
        double minPtAnyVariation() const;

        //print met information
        virtual std::ostream& print( std::ostream& ) const override;

    private:

        //JEC uncertainties
        double _pt_JECDown = 0;
        double _phi_JECDown = 0;
        double _pt_JECUp = 0;
        double _phi_JECUp = 0;

        //unclustered energy uncertainties
        double _pt_UnclDown = 0;
        double _phi_UnclDown = 0;
        double _pt_UnclUp = 0;
        double _phi_UnclUp = 0;

        Met variedMet( const double pt, const double phi ) const;

        std::vector< double > metPtVariations() const;        
};
#endif 
