#include "../interface/ReweighterPileupJetId.h"


// constructor

ReweighterPileupJetId::ReweighterPileupJetId( std::string& efficiencyFile,
						std::string& scaleFactorFile,
						std::string& workingPoint,
						std::string& year ):
    _workingPoint( workingPoint ),
    _year( year )
{
    
    // check if workingPoint has known value
    if( ! (_workingPoint=="loose" || _workingPoint=="medium" || _workingPoint=="tight") ){
	throw std::invalid_argument( std::string("Error in ReweighterPileupJetId constructor: ")
				    + "argument workingPoint has unknown value: '"
				    + _workingPoint + "'..." );
    }
    // map workingPoint to one-letter notation (for use in histogram names)
    std::string wp = (_workingPoint=="loose")?"L":
			(_workingPoint=="medium")?"M":
			(_workingPoint=="tight")?"T":"";

    // check if year has known value
    if( ! (_year=="2016" || _year=="2017" || _year=="2018") ){
	throw std::invalid_argument( std::string("Error in ReweighterPileupJetId constructor:")
				    + "argument year has unknown value: '" + _year + "'..." );
    }

    std::string histName;
    // load the efficiency and mistag histograms
    TFile* effFile = TFile::Open( efficiencyFile.c_str() );
    // efficiency
    histName = "h2_eff_mc"+_year+"_"+wp;
    std::shared_ptr<TH2> effHist( dynamic_cast<TH2*> (
	    effFile->Get( histName.c_str() ) ) );
    effHist->SetDirectory( gROOT );
    _histograms["eff"] = effHist;
    // mistag
    histName = "h2_mistag_mc"+year+"_"+wp;
    std::shared_ptr<TH2> mtHist( dynamic_cast<TH2*> (
            effFile->Get( histName.c_str() ) ) );
    mtHist->SetDirectory( gROOT );
    _histograms["mistag"] = mtHist;
    effFile->Close();
    // load the scale factor histograms
    TFile* sfFile = TFile::Open( scaleFactorFile.c_str() );
    // scale factors for efficiency
    histName = "h2_eff_sf"+year+"_"+wp;
    std::shared_ptr<TH2> effSFHist( dynamic_cast<TH2*> (
            sfFile->Get( histName.c_str() ) ) );
    effSFHist->SetDirectory( gROOT );
    _histograms["effSF"] = effSFHist;
    // scale factor uncertainty for efficiency
    histName = "h2_eff_sf"+year+"_"+wp+"_Systuncty";
    std::shared_ptr<TH2> effSFUncHist( dynamic_cast<TH2*> (
            sfFile->Get( histName.c_str() ) ) );
    effSFUncHist->SetDirectory( gROOT );
    _histograms["effSFUnc"] = effSFUncHist;
    // scale factors for mistag
    histName = "h2_mistag_sf"+year+"_"+wp;
    std::shared_ptr<TH2> mtSFHist( dynamic_cast<TH2*> (
            sfFile->Get( histName.c_str() ) ) );
    mtSFHist->SetDirectory( gROOT );
    _histograms["mistagSF"] = mtSFHist;
    // scale factor uncertainty for efficiency
    histName = "h2_mistag_sf"+year+"_"+wp+"_Systuncty";
    std::shared_ptr<TH2> mtSFUncHist( dynamic_cast<TH2*> (
            sfFile->Get( histName.c_str() ) ) );
    mtSFUncHist->SetDirectory( gROOT );
    _histograms["mistagSFUnc"] = mtSFUncHist;
    sfFile->Close();
}

// central, up and down weights per event

double ReweighterPileupJetId::weight( const Event& event ) const{
    double w = 1.;
    for( std::shared_ptr<Jet> jetPtr : event.jetCollection() ){
	w *= weight( *jetPtr );
    }
    return w;
}

double ReweighterPileupJetId::weightDown( const Event& event ) const{
    double w = 1.;
    for( std::shared_ptr<Jet> jetPtr : event.jetCollection() ){
        w *= weightDown( *jetPtr );
    }
    return w;
}

double ReweighterPileupJetId::weightUp( const Event& event ) const{
    double w = 1.;
    for( std::shared_ptr<Jet> jetPtr : event.jetCollection() ){
        w *= weightUp( *jetPtr );
    }
    return w;
}

// central, up and down weights per jet

double ReweighterPileupJetId::calcJetWeight( const Jet& jet, double efficiency, 
					    double scalefactor ) const{
    bool passwp = (_workingPoint=="loose")?jet.passPileupIdLoose():
		    (_workingPoint=="medium")?jet.passPileupIdMedium():
		    (_workingPoint=="tight")?jet.passPileupIdTight():
		    throw std::invalid_argument(std::string("Error in ReweighterPileupJetId: ")
			    +"working point not recognized in method passPUId");
    if( passwp ) return scalefactor;
    else return (1-scalefactor*efficiency)/(1-efficiency);
}

double ReweighterPileupJetId::weight( const Jet& jet ) const{
    double sf = getScaleFactor( jet );
    double eff = getEfficiency( jet );
    return calcJetWeight( jet, eff, sf );
} 

double ReweighterPileupJetId::weightDown( const Jet& jet ) const{
    // still to check: histograms contain relative or nominal uncertainty???
    double sf = getScaleFactor( jet ) - getScaleFactorUnc(jet);
    double eff = getEfficiency( jet );
    return calcJetWeight( jet, eff, sf );
}

double ReweighterPileupJetId::weightUp( const Jet& jet ) const{
    // still to check: histograms contain relative or nominal uncertainty???
    double sf = getScaleFactor( jet ) + getScaleFactorUnc(jet);
    double eff = getEfficiency( jet );
    return calcJetWeight( jet, eff, sf );
}

// retrieve the scale factor and efficiency for a given jet

bool isPUJet( const Jet& jet ){
    // determine whether a jet is a PU jet or not (MC truth)
    // NOT YET SURE HOW TO DO...
    std::cout << jet.pt() << std::endl; // dummy statement to avoid unused argument
    throw std::logic_error("ERROR: PU jet ID reweighting not yet properly implemented");
}

double ReweighterPileupJetId::getHistogramValueForJet( const Jet& jet, 
						    const std::string& tag ) const{
    // get pt and eta
    double pt = jet.pt();
    double eta = jet.eta();
    // pileup ID is only defined up to pt 50, so return 1 above this threshold
    if( pt>50. ) return 1.;
    // get correct histogram
    std::shared_ptr<TH2> hist = _histograms.at(tag);
    // get correct value from histogram 
    return histogram::contentAtValues(hist.get(),pt,eta);
}

double ReweighterPileupJetId::getEfficiency( const Jet& jet ) const{
    std::string tag = (isPUJet(jet))?"mistag":"eff";
    return getHistogramValueForJet( jet, tag );
}

double ReweighterPileupJetId::getScaleFactor( const Jet& jet ) const{
    std::string tag = (isPUJet(jet))?"mistagSF":"effSF";
    return getHistogramValueForJet( jet, tag );   
}

double ReweighterPileupJetId::getScaleFactorUnc( const Jet& jet ) const{
    std::string tag = (isPUJet(jet))?"mistagSFUnc":"effSFUnc";
    return getHistogramValueForJet( jet, tag );
}
