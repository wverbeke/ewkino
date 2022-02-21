#ifndef ReweighterPileup_H
#define ReweighterPileup_H

#include "Reweighter.h"

//include c++ library classes
#include <map>
#include <memory>

//include ROOT classes
#include "TH1.h"

class ReweighterPileup : public Reweighter {

    public:
	// pre-UL constructor
        ReweighterPileup( const std::vector< Sample >& sampleList, 
			  const std::string& weightDirectory );
	// UL constructor
	ReweighterPileup( const std::string& pileupWeightPath );

        virtual double weight( const Event& ) const override;
        virtual double weightDown( const Event& ) const override;
        virtual double weightUp( const Event& ) const override;

    private:
	bool isUL = false; // set to true if UL constructor is used
	// for pre-UL:
        std::map< std::string, std::shared_ptr< TH1 > > puWeightsCentral;
        std::map< std::string, std::shared_ptr< TH1 > > puWeightsDown;
        std::map< std::string, std::shared_ptr< TH1 > > puWeightsUp;
	double weight( const Event&, const std::map< std::string, std::shared_ptr< TH1 > >& ) const;
	// for UL:
	std::shared_ptr< TH1 > puWeightsCentralUL;
        std::shared_ptr< TH1 > puWeightsDownUL;
        std::shared_ptr< TH1 > puWeightsUpUL;
        double weight( const Event&, const std::shared_ptr< TH1 >& ) const;
};


#endif
