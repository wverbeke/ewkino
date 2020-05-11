#include "../interface/ReweighterBTagHeavyFlavor.h"


//include c++ library classes


ReweighterBTagHeavyFlavor::ReweighterBTagHeavyFlavor(const std::string& weightDirectory, const std::string& sfFilePath, const std::string& workingPoint, const std::shared_ptr< TH2 >& efficiencyC, const std::shared_ptr< TH2 >& efficiencyB ):
    ReweighterBTag( weightDirectory, sfFilePath, workingPoint, true ),
    bTagEfficiencyC( efficiencyC ),
    bTagEfficiencyB( efficiencyB )
{}


double ReweighterBTagHeavyFlavor::efficiencyMC( const Jet& jet ) const{
    if( jet.hadronFlavor() == 4 ){
        return histogram::contentAtValues( bTagEfficiencyC.get(), jet.pt(), jet.absEta() );
    } else if( jet.hadronFlavor() == 5 ){
        return histogram::contentAtValues( bTagEfficiencyB.get(), jet.pt(), jet.absEta() );
    } else {
        throw std::invalid_argument( "hadronFlavor of jet is " + std::to_string( jet.hadronFlavor() ) + " while it should be 4 or 5." );
    }
}
