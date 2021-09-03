#ifndef Muon_H
#define Muon_H

//include other parts of code 
#include "PhysicsObject.h"
#include "LightLepton.h"


template< typename objectType > class PhysicsObjectCollection;

class Muon : public LightLepton {
    
    friend class PhysicsObjectCollection<Muon>;

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

        double segmentCompatibility() const{ return _segmentCompatibility; }
        double trackPt() const{ return _trackPt; }
        double trackPtError() const{ return _trackPtError; }
        double relIso0p4DeltaBeta() const{ return _relIso0p4DeltaBeta; }

        virtual std::ostream& print( std::ostream& os = std::cout ) const override;

    private:
        double _segmentCompatibility = 0;
        double _trackPt = 0;
        double _trackPtError = 0;
        double _relIso0p4DeltaBeta = 0;

        //cut based POG ID working points ( do not include isolation )
        bool _isLoosePOGMuon = false;
        bool _isMediumPOGMuon = false;
        bool _isTightPOGMuon = false;

        virtual Muon* clone() const & override{ return new Muon( *this ); }
        virtual Muon* clone() && override{ return new Muon( std::move(*this) ); }
};

#endif 
