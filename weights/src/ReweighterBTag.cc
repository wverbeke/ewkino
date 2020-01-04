#include "../interface/ReweighterBTag.h"

//include c++ library classes
#include <stdexcept>

//include other parts of framework
#include "../../Tools/interface/histogramTools.h"


ReweighterBTag::ReweighterBTag( const std::string& sfFilePath, const std::string& workingPoint, const std::shared_ptr< TH2 >& efficiencyUDSG, const std::shared_ptr< TH2 >& efficiencyC, const std::shared_ptr< TH2 >& efficiencyB ) :
    bTagEfficiencyUDSG( efficiencyUDSG ),
    bTagEfficiencyC( efficiencyC ),
    bTagEfficiencyB( efficiencyB )
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
    std::string methodHeavy = ( workingPoint == "reshaping" ) ? "iterativefit" : "comb";
    std::string methodLight = ( workingPoint == "reshaping" ) ? "iterativefit" : "incl";

    //calibrate the reader
    bTagSFCalibration = std::shared_ptr< BTagCalibration >( new BTagCalibration( "", sfFilePath ) );
    bTagSFReader->load( *bTagSFCalibration, BTagEntry::FLAV_B, methodHeavy );
    bTagSFReader->load( *bTagSFCalibration, BTagEntry::FLAV_C, methodHeavy );
    bTagSFReader->load( *bTagSFCalibration, BTagEntry::FLAV_UDSG, methodLight );
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


double ReweighterBTag::efficiencyMC( const Jet& jet ) const{
    BTagEntry::JetFlavor jetFlavor = jetFlavorEntry( jet );
    if( jetFlavor == BTagEntry::FLAV_UDSG ){
        return histogram::contentAtValues( bTagEfficiencyUDSG.get(), jet.pt(), jet.eta() );
    } else if( jetFlavor == BTagEntry::FLAV_C ){
        return histogram::contentAtValues( bTagEfficiencyC.get(), jet.pt(), jet.eta() );
    } else {
        return histogram::contentAtValues( bTagEfficiencyB.get(), jet.pt(), jet.eta() );
    }
}


double ReweighterBTag::weight( const Jet& jet, const std::string& uncertainty ) const{

    //make sure jet passes b-tag selection
    if( ! jet.inBTagAcceptance() ){
        return 1.;
    }

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
