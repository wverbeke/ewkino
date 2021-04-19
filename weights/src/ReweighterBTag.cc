#include "../interface/ReweighterBTag.h"

//include c++ library classes
#include <stdexcept>

//include other parts of framework
#include "../../Tools/interface/histogramTools.h"
#include "../../Tools/interface/stringTools.h"


ReweighterBTag::ReweighterBTag( const std::string& weightDirectory, const std::string& sfFilePath, const std::string& workingPoint, const bool heavyFlavor ):
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
    bTagSFCalibration = std::shared_ptr< BTagCalibration >( new BTagCalibration( "", stringTools::formatDirectoryName( weightDirectory ) + sfFilePath ) );
    if( heavyFlavor ){
        bTagSFReader->load( *bTagSFCalibration, BTagEntry::FLAV_B, fitMethod );
        bTagSFReader->load( *bTagSFCalibration, BTagEntry::FLAV_C, fitMethod );
    } else {
        bTagSFReader->load( *bTagSFCalibration, BTagEntry::FLAV_UDSG, fitMethod );
    }
}


BTagEntry::JetFlavor jetFlavorEntry( const Jet& jet ){
    if( jet.hadronFlavor() == 0 ){
        return BTagEntry::FLAV_UDSG;
    } else if( jet.hadronFlavor() == 4 ){   
        return BTagEntry::FLAV_C;
    } else if( jet.hadronFlavor() == 5 ){
        return BTagEntry::FLAV_B;
    } else {
        throw std::invalid_argument( "Jet has hadron flavor " + std::to_string( jet.hadronFlavor() ) + ", while expected numbers are 0, 4 or 5." );
    }
}


double ReweighterBTag::weight( const Jet& jet, const std::string& uncertainty ) const{

    if( _heavyFlavor ){
        if( !( jet.hadronFlavor() == 4 || jet.hadronFlavor() == 5 ) ) return 1.;
    } else {
        if( !( jet.hadronFlavor() == 0 ) ) return 1.;
    }

    //make sure jet passes b-tag selection
    if( ! jet.inBTagAcceptance() ){
        return 1.;
    }

    // note: https://twiki.cern.ch/twiki/bin/view/CMS/BTagCalibration#Using_b_tag_scale_factors_in_you
    // this page recommends to use absolute value of eta, but BTagCalibrationStandalone.cc
    // seems to handle negative values of eta more correctly (only taking abs when needed)
    double scaleFactor = bTagSFReader->eval_auto_bounds( uncertainty, jetFlavorEntry( jet ), jet.eta(), jet.pt(), CSVValue( jet ) );
    
    //check if jet passes chosen b-tag working point
    //in case of reweighting of the full shape, no selection is required
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


double ReweighterBTag::weight( const Event& event, double ( ReweighterBTag::*jetWeight )( const Jet& ) const ) const{
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
