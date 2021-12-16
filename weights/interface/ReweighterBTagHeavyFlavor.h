#ifndef ReweighterBTagHeavyFlavor_H
#define ReweighterBTagHeavyFlavor_H

#include "ReweighterBTag.h"

//include ROOT classes
#include "TH2.h"


class ReweighterBTagHeavyFlavor : public ReweighterBTag {

    public:
        ReweighterBTagHeavyFlavor(  const std::string& weightDirectory, 
				    const std::string& sfFilePath, 
				    const std::string& workingPoint, 
				    const std::shared_ptr< TH2 >& efficiencyC, 
				    const std::shared_ptr< TH2 >& efficiencyB );

    private:
        std::shared_ptr< TH2 > bTagEfficiencyC;
        std::shared_ptr< TH2 > bTagEfficiencyB; 
        virtual double efficiencyMC( const Jet& ) const override;
};

#endif 
