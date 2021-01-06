#ifndef ReweighterPileupJetId_H
#define ReweighterPileupJetId_H

#include "Reweighter.h"

// NOTE: NOT YET TESTED, NOT GUARANTEED TO WORK AS EXPECTED

class ReweighterPileupJetId : public Reweighter {

    public:

	// constructor 
	ReweighterPileupJetId( std::string& efficiencyFile, 
				std::string& scaleFactorFile,
				std::string& workingPoint,
				std::string& year );

	// central, up and down weights per event
	double weight( const Event& ) const;
	double weightDown( const Event& ) const;
	double weightUp( const Event& ) const;


    private:

	// attributes
	std::string _workingPoint = "";
	std::string _year = "";
	// (copy of given workingPoint argument)
	std::map< std::string, std::shared_ptr< TH2 >> _histograms;
	// (map of required histograms indexed by a suitable name or tag)

	// central, up and down weights per jet
	double calcJetWeight( const Jet&, double, double ) const;
	double weight( const Jet& ) const;
	double weightDown( const Jet& ) const;
	double weightUp( const Jet& ) const;

	// retrieve the scale factor and efficiency for a given jet
	double getHistogramValueForJet( const Jet&, const std::string& ) const;
	double getEfficiency( const Jet& ) const;
	double getScaleFactor( const Jet& ) const;
	double getScaleFactorUnc( const Jet& ) const;
};

#endif
