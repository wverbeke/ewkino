/* 
=== b-tag reweighter ===
This reweighter accounts for a systematic difference between data and simulation 
with respect to b-tagging score.

The method relies both on b-tagging efficiencies and data-to-simulation scale factors.
-  The efficiencies should be measured by the user, by running the script
   weights/computeBTagSFCode/computeBTagEfficiencies.cc.
-  The scale factors are provided centrally. The scale factors are given in .csv format 
   and are situated in weights/weightFiles/bTagSF/ and the code to read them is provided 
   in weights/bTagSFCode/BTagCalibrationStandalone.cc

For more information: see https://twiki.cern.ch/twiki/bin/view/CMS/BTagSFMethods
The method implemented here corresponds to method "1a".
Note that this is in principle not enough if the b-tag shape information is used,
e.g. when using b-tagging scores in an MVA. 
In those cases, the reweighterBTagShape class should be used instead.

This class contains virtual methods (efficiencyMC and CSVValue) 
so should not be directly instantiated!
The classes reweighterBTagHeavyFlavor and reweighterBTagLightFlavor are derived from this one.
*/


#include "../interface/ReweighterBTag.h"


ReweighterBTag::ReweighterBTag( const std::string& weightDirectory, 
				const std::string& sfFilePath, 
				const std::string& workingPoint, 
				const bool heavyFlavor ):
    // constructor
    // input arguments:
    // - weightDirectory: path to the ewkino/weights folder
    // - sfFilePath: path to the scale factor csv file relative from ewkino/weights
    // - workingPoint: should be either "loose", "medium", "tight" or "reshaping"
    // - heavyFlavor: boolean whether this is for heavy flavor
    //                (used to select correct scale factors as the csv files contain many)
    _heavyFlavor( heavyFlavor )
{

    //determine what working point to use, and the corresponding selection function
    BTagEntry::OperatingPoint wp;
    if( workingPoint == "loose" ){
        wp = BTagEntry::OP_LOOSE;
        passBTag= &Jet::isBTaggedLoose;
    } else if( workingPoint == "medium" ){
        wp = BTagEntry::OP_MEDIUM;
        passBTag = &Jet::isBTaggedMedium;
    } else if( workingPoint == "tight" ){
        wp = BTagEntry::OP_TIGHT;
        passBTag = &Jet::isBTaggedTight;
    } else if( workingPoint == "reshaping" ){
        wp = BTagEntry::OP_RESHAPING;
        passBTag = nullptr;
    } else{
        throw std::invalid_argument( "Unknown b-tagging working point string given." );
    }

    //make the scale factor reader
    bTagSFReader.reset( new BTagCalibrationReader( wp, "central", {"up", "down"}) );

    //method for extracting scale factors
    std::string fitMethod;
    if( workingPoint == "reshaping" ){
        fitMethod = "iterativefit";
    } else if( _heavyFlavor ){
        fitMethod = "comb";
    } else {
        fitMethod = "incl";
    }

    //calibrate the reader
    bTagSFCalibration = std::shared_ptr< BTagCalibration >( 
	new BTagCalibration( "", stringTools::formatDirectoryName(weightDirectory)+sfFilePath ) );
    if( heavyFlavor ){
        bTagSFReader->load( *bTagSFCalibration, BTagEntry::FLAV_B, fitMethod );
        bTagSFReader->load( *bTagSFCalibration, BTagEntry::FLAV_C, fitMethod );
    } else {
        bTagSFReader->load( *bTagSFCalibration, BTagEntry::FLAV_UDSG, fitMethod );
    }
}


BTagEntry::JetFlavor jetFlavorEntry( const Jet& jet ){
    // return the flavor of a jet (at simulation truth level)
    if( jet.hadronFlavor() == 0 ){
        return BTagEntry::FLAV_UDSG;
    } else if( jet.hadronFlavor() == 4 ){   
        return BTagEntry::FLAV_C;
    } else if( jet.hadronFlavor() == 5 ){
        return BTagEntry::FLAV_B;
    } else {
        throw std::invalid_argument( "Jet has hadron flavor " 
		+ std::to_string( jet.hadronFlavor() ) 
		+ ", while expected numbers are 0, 4 or 5." );
    }
}


double ReweighterBTag::weight( const Jet& jet, const std::string& uncertainty ) const{
    // calculate the weight (either central, up or down) for a single jet
    // the weight is calculated as follows:
    // - if the jet is a light jet and this reweighter is for heavy flavour, the weight is 1.
    // - if the jet is a b- or a c-jet and this reweighter is for light flavour, the weight is 1.
    // - if the jet is not in b-tag acceptance, the weight is 1.
    // - the correct scalefactor is read from the csv file.
    // - if the jet passes the given working point, weight = scalefactor.
    // - if not, weight = (1 - efficiency*scalefactor) / (1 - efficiency)

    // check flavour
    if( _heavyFlavor ){
	if( !( jet.hadronFlavor() == 4 || jet.hadronFlavor() == 5 ) ) return 1.;
    } else{
	if( !( jet.hadronFlavor() == 0 ) ) return 1.;
    }

    // make sure jet passes b-tag selection
    if( ! jet.inBTagAcceptance() ) return 1.;

    // read the scale factor
    double scaleFactor = bTagSFReader->eval_auto_bounds( uncertainty, 
			    jetFlavorEntry( jet ), jet.eta(), jet.pt(), CSVValue( jet ) );
    
    // check if jet passes chosen b-tag working point
    // in case of reweighting of the full shape, no selection is required
    if( ( passBTag == nullptr ) || ( jet.* passBTag )() ){
        return scaleFactor;
    } else {
        double efficiency = efficiencyMC( jet );
        return ( 1. - efficiency * scaleFactor ) / ( 1. - efficiency );
    }
}


double ReweighterBTag::weight( const Jet& jet ) const{
    return weight( jet, "central" );
}


double ReweighterBTag::weightDown( const Jet& jet ) const{
    return weight( jet, "down" );
}


double ReweighterBTag::weightUp( const Jet& jet ) const{
    return weight( jet, "up" );
}


double ReweighterBTag::weight( const Event& event, 
			       double ( ReweighterBTag::*jetWeight )( const Jet& ) const ) const{
    // calculate the weight (central, up or down) for an event,
    // by multiplying those for individual jets in the event. 
    double weight = 1.;
    for( const auto& jetPtr : event.jetCollection() ){
        weight *= (this->*jetWeight)( *jetPtr );
    }
    return weight;
}


double ReweighterBTag::weight( const Event& event ) const{
    return weight( event, &ReweighterBTag::weight );
}


double ReweighterBTag::weightDown( const Event& event ) const{
    return weight( event, &ReweighterBTag::weightDown );
}


double ReweighterBTag::weightUp( const Event& event ) const{
    return weight( event, &ReweighterBTag::weightUp );
}
