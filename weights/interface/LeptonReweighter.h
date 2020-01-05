#ifndef LeptonReweighter_H
#define LeptonReweighter_H

//include c++ library classes
#include <memory>

//include root classes 
#include "TH2.h"

//include other parts of framework
#include "LeptonSelectionHelper.h"
#include "../../Tools/interface/histogramTools.h"


template < typename LeptonType > class LeptonReweighter{

    public:
        LeptonReweighter( const std::shared_ptr< TH2 >&, LeptonSelectionHelper* );

        double weight( const LeptonType& ) const;
        double weightDown( const LeptonType& ) const;
        double weightUp( const LeptonType& ) const;

        virtual double ptVariable( const LeptonType& lepton ) const{ return lepton.uncorrectedPt(); }
        virtual double etaVariable( const LeptonType& lepton ) const{ return lepton.absEta(); }

    private:
        std::shared_ptr< TH2 > weightMap;
        std::shared_ptr< LeptonSelectionHelper > selector;
        bool ptOnXAxis;

        double weight( const LeptonType& lepton, double (&retrieveValue)( TH2*, const double, const double ) ) const;
};


template < typename LeptonType > LeptonReweighter< LeptonType >::LeptonReweighter( const std::shared_ptr< TH2 >& scaleFactorMap, LeptonSelectionHelper* selectionHelper ) :
    weightMap( scaleFactorMap ),
    selector( selectionHelper )
{

    //use the range of the weight histogram to determine whether the X or Y axis represents the pT 
    if( histogram::maxXValue( scaleFactorMap.get() ) > 3. ){
        ptOnXAxis = true;
    } else {
        ptOnXAxis = false;
    }
}


template < typename LeptonType > double LeptonReweighter< LeptonType >::weight( const LeptonType& lepton, double (&retrieveValue)( TH2*, const double, const double ) ) const{
    if( selector->passSelection( lepton ) ){
        if( ptOnXAxis ){
            return retrieveValue( weightMap.get(), ptVariable( lepton ), etaVariable( lepton ) );
        } else {
            return retrieveValue( weightMap.get(), etaVariable( lepton ), ptVariable( lepton ) );
        }
    } else {
        return 1.;
    }
}


template < typename LeptonType > double LeptonReweighter< LeptonType >::weight( const LeptonType& lepton ) const{
    return weight( lepton, histogram::contentAtValues );
}


template < typename LeptonType > double LeptonReweighter< LeptonType >::weightDown( const LeptonType& lepton ) const{
    return weight( lepton, histogram::contentDownAtValues );
}


template< typename LeptonType > double LeptonReweighter< LeptonType >::weightUp( const LeptonType& lepton ) const{
    return weight( lepton, histogram::contentUpAtValues );
}
#endif 
