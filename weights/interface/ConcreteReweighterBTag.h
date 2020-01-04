#ifndef ConcreteReweighterBTag_H
#define ConcreteReweighterBTag_H

class ReweighterBTagDeepCSV : public ReweighterBTag {

    using ReweighterBTag::ReweighterBTag;

    private:
        virtual double CSVValue( const Jet& jet ) const override{ return jet.deepCSV(); }
};


class ReweighterBTagDeepFlavor : public ReweighterBTag {

    using ReweighterBTag::ReweighterBTag;
    
    private:
        virtual double CSVValue( const Jet& jet ) const override{ return jet.deepFlavor(); }
};

#endif
