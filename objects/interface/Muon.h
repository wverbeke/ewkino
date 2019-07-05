#ifndef Muon_H
#define Muon_H

//include other parts of code 
#include "LightLepton.h"

class Muon : public LightLepton {
    
    public:
        Muon( const TreeReader&, const unsigned );
        Muon( const Muon& );
        Muon( Muon&& ) noexcept;

        Muon& operator=( const Muon& ) = default;
        Muon& operator=( Muon&& ) = default;

        virtual bool isMuon() const override{ return true; }
        virtual bool isElectron() const override{ return false; }

        bool isLoosePOGMuon() const{ return _isLoosePOGMuon; }
        bool isMediumPOGMuon() const{ return _isMediumPOGMuon; }
        bool isTightPOGMuon() const{ return _isTightPOGMuon; }

    private:
        double _segmentCompatibility = 0;
        double _trackPt = 0;
        double _trackPtError = 0;

        //cut based POG ID working points ( do not include isolation )
        bool _isLoosePOGMuon = false;
        bool _isMediumPOGMuon = false;
        bool _isTightPOGMuon = false;

        virtual Muon* clone() const & override{ return new Muon( *this ); }
        virtual Muon* clone() && override{ return new Muon( std::move(*this) ); }
};

#endif 
