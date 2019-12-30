#ifndef ConcreteSelection_H
#define ConcreteSelection_H

#include "LeptonSelectionHelper.h"



class LooseSelector : public LeptonSelectionHelper{
    
    public:
        virtual bool passSelection( const Lepton& lepton ) const override{ return lepton.isLoose(); }
};



class FOSelector : public LeptonSelectionHelper{
    
    public: 
        virtual bool passSelection( const Lepton& lepton ) const override{ return lepton.isFO(); }
};



class TightSelector : public LeptonSelectionHelper{

    public:
        virtual bool passSelection( const Lepton& lepton ) const override{ return lepton.isTight(); }
};



class LooseNotTightSelector : public LeptonSelectionHelper{
    
    public:
        virtual bool passSelection( const Lepton& lepton ) const override{ return ( lepton.isLoose() && ( ! lepton.isTight() ) ); }
};



class FONotTightSelector : public LeptonSelectionHelper{

    public:
        virtual bool passSelection( const Lepton& lepton ) const override{ return ( lepton.isFO() && ( ! lepton.isTight() ) ); }
};


class LooseNotFOSelector : public LeptonSelectionHelper{

    public:
        virtual bool passSelection( const Lepton& lepton ) const override{ return ( lepton.isLoose() && ( ! lepton.isFO() ) ); }

};



#endif 
