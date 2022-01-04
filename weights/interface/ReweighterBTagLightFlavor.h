#ifndef ReweighterBTagLightFlavor_H
#define ReweighterBTagLightFlavor_H


class ReweighterBTagLightFlavor : public ReweighterBTag {

    public:
        ReweighterBTagLightFlavor(  const std::string& weightDirectory, 
				    const std::string& sfFilePath, 
				    const std::string& workingPoint, 
				    const std::shared_ptr< TH2 >& efficiencyUDSG );


    private:
        std::shared_ptr< TH2 > bTagEfficiencyUDSG;
        virtual double efficiencyMC( const Jet& ) const override;
};

#endif
