#include "../interface/ReweighterBTagLightFlavor.h"


ReweighterBTagLightFlavor::ReweighterBTagLightFlavor(const std::string& weightDirectory, const std::string& sfFilePath, const std::string& workingPoint, const std::shared_ptr< TH2 >& efficiencyUDSG):
    ReweighterBTag( weightDirectory, sfFilePath, workingPoint, false ),
    bTagEfficiencyUDSG( efficiencyUDSG )
{}


double ReweighterBTagLightFlavor::efficiencyMC( const Jet& jet ) const{
    if( jet.hadronFlavor() == 0 ){
        return histogram::contentAtValues( bTagEfficiencyUDSG.get(), jet.pt(), jet.absEta() );
    } else {
        throw std::invalid_argument( "hadronFlavor of jet is " + std::to_string( jet.hadronFlavor() ) + " while it should be 0." );
    }
}
