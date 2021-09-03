#ifndef Tau_H
#define Tau_H

//include other parts of code 
#include "PhysicsObject.h"
#include "Lepton.h"

template< typename objectType > class PhysicsObjectCollection;

class Tau : public Lepton{

    friend class PhysicsObjectCollection< Tau >;

    public:
        Tau( const TreeReader&, const unsigned );
        Tau( const Tau& );
        Tau( Tau&& ) noexcept;

        Tau& operator=( const Tau& ) = default;
        Tau& operator=( Tau&& ) noexcept = default;

        virtual bool isLightLepton() const override{ return false; }
        virtual bool isMuon() const override{ return false; }
        virtual bool isElectron() const override{ return false; }
        virtual bool isTau() const override{ return true; }

        unsigned decayMode() const{ return _decayMode; }
        bool passDecayModeFinding() const{ return _passDecayModeFinding; }
        bool passDecayModeFindingNew() const{ return _passDecayModeFindingNew; }

        bool passMuonVetoLoose() const{ return _passMuonVetoLoose; }
        bool passMuonVetoTight() const{ return _passMuonVetoTight; }
        bool passEleVetoVLoose() const{ return _passEleVetoVLoose; }
        bool passEleVetoLoose() const{ return _passEleVetoLoose; }
        bool passEleVetoMedium() const{ return _passEleVetoMedium; }
        bool passEleVetoTight() const{ return _passEleVetoTight; }
        bool passEleVetoVTight() const{ return _passEleVetoVTight; } 

        bool passVLooseMVAOld2015() const{ return _passVLooseMVAOld2015; }
        bool passLooseMVAOld2015() const{ return _passLooseMVAOld2015; }
        bool passMediumMVAOld2015() const{ return _passMediumMVAOld2015; }
        bool passTightMVAOld2015() const{ return _passTightMVAOld2015; }
        bool passVTightMVAOld2015() const{ return _passVTightMVAOld2015; }

        bool passVLooseMVANew2015() const{ return _passVLooseMVANew2015; }
        bool passLooseMVANew2015() const{ return _passLooseMVANew2015; }
        bool passMediumMVANew2015() const{ return _passMediumMVANew2015; }
        bool passTightMVANew2015() const{ return _passTightMVANew2015; }
        bool passVTightMVANew2015() const{ return _passVTightMVANew2015; }

        bool passVVLooseMVAOld2017() const{ return _passVVLooseMVAOld2017; }
        bool passVLooseMVAOld2017() const{ return _passVLooseMVAOld2017; }
        bool passLooseMVAOld2017() const{ return _passLooseMVAOld2017; }
        bool passMediumMVAOld2017() const{ return _passMediumMVAOld2017; }
        bool passTightMVAOld2017() const{ return _passTightMVAOld2017; }
        bool passVTightMVAOld2017() const{ return _passVTightMVAOld2017; }
        bool passVVTightMVAOld2017() const{ return _passVVTightMVAOld2017; }

        bool passVLooseMVANew2017() const{ return _passVLooseMVANew2017; }
        bool passLooseMVANew2017() const{ return _passLooseMVANew2017; }
        bool passMediumMVANew2017() const{ return _passMediumMVANew2017; }
        bool passTightMVANew2017() const{ return _passTightMVANew2017; }
        bool passVTightMVANew2017() const{ return _passVTightMVANew2017; }

        virtual std::ostream& print( std::ostream& os = std::cout ) const override;

    private:
        unsigned _decayMode;
        bool _passDecayModeFinding;
        bool _passDecayModeFindingNew;

        bool _passMuonVetoLoose;
        bool _passMuonVetoTight;
        bool _passEleVetoVLoose;
        bool _passEleVetoLoose;
        bool _passEleVetoMedium;
        bool _passEleVetoTight;
        bool _passEleVetoVTight;

        bool _passVLooseMVAOld2015;
        bool _passLooseMVAOld2015;
        bool _passMediumMVAOld2015;
        bool _passTightMVAOld2015;
        bool _passVTightMVAOld2015;

        bool _passVLooseMVANew2015;
        bool _passLooseMVANew2015;
        bool _passMediumMVANew2015;
        bool _passTightMVANew2015;
        bool _passVTightMVANew2015;

        bool _passVVLooseMVAOld2017;
        bool _passVLooseMVAOld2017;
        bool _passLooseMVAOld2017;
        bool _passMediumMVAOld2017;
        bool _passTightMVAOld2017;
        bool _passVTightMVAOld2017;
        bool _passVVTightMVAOld2017;

        bool _passVLooseMVANew2017;
        bool _passLooseMVANew2017;
        bool _passMediumMVANew2017;
        bool _passTightMVANew2017;
        bool _passVTightMVANew2017;
        
        virtual Tau* clone() const & override{ return new Tau( *this ); }
        virtual Tau* clone() && override{ return new Tau( std::move( *this ) ); }   
};

#endif
