#ifndef Tau_H
#define Tau_H

//include other parts of code 
#include "Lepton.h"
#include "TauSelector.h"

class Tau : public Lepton{

    public:
        Tau( const TreeReader&, const unsigned );
        Tau( const Tau& );
        Tau( Tau&& ) noexcept;

        Tau& operator=( const Tau& ) = default;
        Tau& operator=( Tau&& ) noexcept = default;

        virtual bool isLightLepton() const override{ return false; }
        virtual bool isMuon() const override{ return false; }
        virtual bool isElectron() const override{ return false; }
        virtual bool isTau() const override{ return false; }

        bool passMuonVeto() const{ return _passMuonVeto; }
        bool passElectronVeto() const{ return _passElectronVeto; }
        bool passDecayModeFindingNew() const{ return _passDecayModeFindingNew; }

    private:
        bool _passMuonVeto = false;
        bool _passElectronVeto = false;
        bool _passDecayModeFindingNew = false;

        //More variables should be added in the future!

        virtual Tau* clone() const & override{ return new Tau( *this ); }
        virtual Tau* clone() && override{ return new Tau( std::move( *this ) ); }   
};

#endif
