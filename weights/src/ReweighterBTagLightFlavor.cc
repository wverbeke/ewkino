/*
=== b-tag reweighter for light flavor === 
Extension of reweighterBTag with b-tagging efficiency histograms for light-flavor jets
and a method to extract the efficiency for a given jet.

Note: this class still contains a virtual method (CSVValue) 
so should not be directly instantiated!
See ConcreteReweighterBTag.h! 
*/


#include "../interface/ReweighterBTagLightFlavor.h"


ReweighterBTagLightFlavor::ReweighterBTagLightFlavor(
	const std::string& weightDirectory, 
	const std::string& sfFilePath, 
	const std::string& workingPoint, 
	const std::shared_ptr< TH2 >& efficiencyUDSG):
    // constructor
    // input arguments:
    // - weightDirectory: path to the ewkino/weights folder
    // - sfFilePath: path to the scale factor csv file relative from ewkino/weights
    // - workingPoint: should be either "loose", "medium", "tight" or "reshaping"
    // - efficiencyUDSG: 2D root histogram with b-tag efficiencies for light jets
    ReweighterBTag( weightDirectory, sfFilePath, workingPoint, false ),
    bTagEfficiencyUDSG( efficiencyUDSG )
{}


double ReweighterBTagLightFlavor::efficiencyMC( const Jet& jet ) const{
    // get the b-tagging efficiency for a given jet.
    // the efficiency is read from the histograms, based on the pt and eta of the jet.
    if( jet.hadronFlavor() == 0 ){
        return histogram::contentAtValues( bTagEfficiencyUDSG.get(), jet.pt(), jet.absEta() );
    } else {
        throw std::invalid_argument( "hadronFlavor of jet is " 
		    + std::to_string( jet.hadronFlavor() ) 
		    + " while it should be 0." );
    }
}
