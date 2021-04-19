#ifndef ReweighterBTagShape_H
#define ReweighterBTagShape_H

#include "Reweighter.h"

// include C++ classes
#include <vector>

// include ROOT classes
#include "TH2.h"

// include official b-tag weight reader
#include "../bTagSFCode/BTagCalibrationStandalone.h"

class ReweighterBTagShape: public Reweighter{

    public:
	
	ReweighterBTagShape( const std::string&, const std::string&, const std::string&,
				const std::vector<Sample>&,
				const std::vector<std::string>&, const std::string& );
	//void initialize( const std::vector<Sample>& );
	bool hasVariation( const std::string& ) const;
	void setNormFactors( const Sample&, std::map<int,double>);
	double getNormFactor( const Event&, const std::string& jecVariation="nominal" ) const;
	double weight( const Jet& ) const;
	double weightUp( const Jet&, const std::string& ) const;
	double weightDown( const Jet&, const std::string& ) const;
	double weight( const Event& ) const;
	double weightJecVar( const Event&, const std::string& ) const;
	double weightNoNorm( const Event& ) const;
	double weightUp( const Event&, const std::string& ) const;
	double weightDown( const Event&, const std::string& ) const;
	std::vector<std::string> availableVariations() const{ return _variations; }
	std::vector<std::string> availableSystematics() const{ return _systematics; }
	// (note the difference between variations and systematics: 
	// systematics is a subcollection of variations, that should be applied separately,
	// while variations contains also jec variations only meant for propagation into jec unc.)
	// the following functions are needed for correct inheritance,
	// but do not use in practice! (use the ones with additional arguments)
	double weightUp( const Event& ) const{ return 0.; } 
        double weightDown( const Event& ) const{ return 0.; } 

    private:

	std::string _weightDirectory;
	std::shared_ptr<BTagCalibration> bTagSFCalibration;
	std::shared_ptr<BTagCalibrationReader> bTagSFReader;
	std::string _flavor;
	std::string _bTagAlgo;
	std::vector<std::string> _variations;
	std::vector<std::string> _systematics;
	std::map<std::string,std::map<int,double>> _normFactors;
	// (map of sample name to any normKey (e.g. nJets) to norm factor)

	double weight( const Jet&, const std::string& ) const;
	/*double writeSumOfWeights( const Sample&,
                            const std::string&,
                            long unsigned numberOfEntries=0,
			    bool doWrite=false ) const;
	double readSumOfWeights( const Sample&,
                                const std::string&,
                                long unsigned numberOfEntries=0 ) const;*/

};

#endif
