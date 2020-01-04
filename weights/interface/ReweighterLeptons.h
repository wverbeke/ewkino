#ifndef ReweighterLeptons_H
#define ReweighterLeptons_H

//include c++ library classes
#include <memory>

//include other parts of framework
#include "Reweighter.h"
//#include "ConcreteLeptonReweighter.h"
#include "../../Event/interface/Event.h"


template < typename LeptonType, typename CollectionType, typename ReweighterType > class ReweighterLeptons : public Reweighter{

    public:
        ReweighterLeptons( const ReweighterType& r ) : leptonReweighter( new ReweighterType( r ) ) {}
        ReweighterLeptons( const std::shared_ptr< ReweighterType >& r ) : leptonReweighter( r ) {}

        virtual double weight( const Event& event ) const override;
        virtual double weightDown( const Event& event ) const override;
        virtual double weightUp( const Event& event ) const override;

    private:
        double weight( const Event& event, double (ReweighterType::*weightFunction)( const LeptonType& ) const ) const;
        virtual CollectionType leptonCollection( const Event& event ) const = 0;
        std::shared_ptr< ReweighterType > leptonReweighter;
};



template< typename LeptonType, typename CollectionType, typename ReweighterType >
    double ReweighterLeptons< LeptonType, CollectionType, ReweighterType >::weight( const Event& event, double (ReweighterType::*weightFunction)( const LeptonType& ) const ) const
{
    double ret = 1.;
    for( const auto& leptonPtr : leptonCollection( event ) ){
        ret *= ( *leptonReweighter.*weightFunction )( *leptonPtr );
    }
    return ret;
}


template< typename LeptonType, typename CollectionType, typename ReweighterType >
    double ReweighterLeptons< LeptonType, CollectionType, ReweighterType >::weight( const Event& event ) const
{
    return weight( event, &ReweighterType::weight );
}


template< typename LeptonType, typename CollectionType, typename ReweighterType >
    double ReweighterLeptons< LeptonType, CollectionType, ReweighterType >::weightDown( const Event& event ) const
{
    return weight( event, &ReweighterType::weightDown );
}


template< typename LeptonType, typename CollectionType, typename ReweighterType >
    double ReweighterLeptons< LeptonType, CollectionType, ReweighterType >::weightUp( const Event& event ) const
{
    return weight( event, &ReweighterType::weightUp );
}

#endif
