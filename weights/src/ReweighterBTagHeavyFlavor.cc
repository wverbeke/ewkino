/*
=== b-tag reweighter for heavy flavor === 
Extension of reweighterBTag with b-tagging efficiency histograms for heavy-flavor jets
and a method to extract the efficiency for a given jet.

Note: this class still contains a virtual method (CSVValue) 
so should not be directly instantiated!
See ConcreteReweighterBTag.h! 
*/


#include "../interface/ReweighterBTagHeavyFlavor.h"


ReweighterBTagHeavyFlavor::ReweighterBTagHeavyFlavor(
	    const std::string& weightDirectory, 
	    const std::string& sfFilePath,
	    const std::string& workingPoint, 
	    const std::shared_ptr< TH2 >& efficiencyC, 
	    const std::shared_ptr< TH2 >& efficiencyB ):
    // constructor
    // input arguments:
    // - weightDirectory: path to the ewkino/weights folder
    // - sfFilePath: path to the scale factor csv file relative from ewkino/weights
    // - workingPoint: should be either "loose", "medium", "tight" or "reshaping"
    // - efficiencyC: 2D root histogram with b-tag efficiencies for c-jets
    // - efficiencyB: 2D root histogram with b-tag efficiencies for b-jets
    ReweighterBTag( weightDirectory, sfFilePath, workingPoint, true ),
    bTagEfficiencyC( efficiencyC ),
    bTagEfficiencyB( efficiencyB )
{}


double ReweighterBTagHeavyFlavor::efficiencyMC( const Jet& jet ) const{
    // get the b-tagging efficiency for a given jet.
    // the efficiency is read from the histograms, based on the pt and eta of the jet.
    if( jet.hadronFlavor() == 4 ){
        return histogram::contentAtValues( bTagEfficiencyC.get(), jet.pt(), jet.absEta() );
    } else if( jet.hadronFlavor() == 5 ){
        return histogram::contentAtValues( bTagEfficiencyB.get(), jet.pt(), jet.absEta() );
    } else {
        throw std::invalid_argument( "hadronFlavor of jet is " 
		    + std::to_string( jet.hadronFlavor() ) 
		    + " while it should be 4 or 5." );
    }
}
