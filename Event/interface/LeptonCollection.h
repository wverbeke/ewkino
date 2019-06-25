#ifndef LeptonCollection_H
#define LeptonCollection_H

//include other parts of code 
#include "../../objects/interface/Lepton.h"
#include "../../TreeReader/interface/TreeReader.h"
#include "PhysicsObjectCollection.h"
#include "MuonCollection.h"
#include "ElectronCollection.h"
#include "TauCollection.h"



class LeptonCollection : public PhysicsObjectCollection< Lepton > {
    
    public:
        LeptonCollection( const TreeReader& );

        MuonCollection muonCollection() const;
        ElectronCollection electronCollection() const;
        TauCollection tauCollection() const;

        //select leptons
        void selectLooseLeptons();

    private:
        //clean electrons and taus (to be implemented )
};



#endif
