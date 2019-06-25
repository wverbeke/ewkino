#ifndef MuonSelector_H
#define MuonSelector_H

//include c++ library classes 
#include <utility>

//include other parts of code 
#include "LeptonSelector.h"

class Muon;

class MuonSelector : public LeptonSelector {
    
    public:
        MuonSelector( const Muon* const mPtr ) : muonPtr( mPtr ) {} 
        
        virtual bool isLoose() const override;
        virtual bool isFO() const override;
        virtual bool isGood() const override;
        virtual bool isTight() const override;

    private:
        const Muon* const muonPtr;

        virtual LeptonSelector* clone() const & override{ return new MuonSelector(*this); }
        virtual LeptonSelector* clone() && override{ return new MuonSelector( std::move( *this ) ); }
};
#endif 
