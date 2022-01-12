#ifndef ReweighterBTag_H
#define ReweighterBTag_H

//include c++ library classes
#include <stdexcept>

//include ROOT classes
#include "TH2.h"

//include official b-tag weight reader
#include "../bTagSFCode/BTagCalibrationStandalone.h"

//include other parts of framework
#include "Reweighter.h"
#include "../../Tools/interface/histogramTools.h"
#include "../../Tools/interface/stringTools.h"


class ReweighterBTag : public Reweighter {

    public:
        ReweighterBTag( const std::string& weightDirectory, 
			const std::string& sfFilePath, 
			const std::string& workingPoint, 
			const bool heavyFlavor );

        virtual double weight( const Event& ) const override;
        virtual double weightDown( const Event& ) const override;
        virtual double weightUp( const Event& ) const override;

        double weight( const Jet& ) const;
        double weightDown( const Jet& ) const;
        double weightUp( const Jet& ) const;
    private:
        std::shared_ptr< BTagCalibration > bTagSFCalibration;
        std::shared_ptr< BTagCalibrationReader > bTagSFReader;
        bool _heavyFlavor;
        
        bool (Jet::*passBTag)() const = nullptr;

        virtual double CSVValue( const Jet& ) const = 0;
        virtual double efficiencyMC( const Jet& ) const = 0;
        double weight( const Jet&, const std::string& ) const; 
        double weight( const Event&, double (ReweighterBTag::*jetWeight)( const Jet& ) const ) const;

};

#endif
