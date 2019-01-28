#ifndef MuonSelector_H
#define MuonSelector_H

//include other parts of code 
#include "LeptonSelector.h"

class Muon;

class MuonSelector : public LeptonSelector {
    
    public:
        MuonSelector( const Muon& muon) : muonPtr( &muon ) {} 
        
        virtual bool isLoose() const override;
        virtual bool isGood() const override;
        virtual bool isTight() const override;

    private:
        const Muon* const muonPtr;

};
#endif 
