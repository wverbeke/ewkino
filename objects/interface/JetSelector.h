#ifndef JetSelector_H
#define JetSelector_H

class Jet;

class JetSelector{

    public:
        JetSelector( Jet* jPtr ) : jetPtr( jPtr ){}
        
        bool isGood() const;
        bool isBTaggedLoose() const;
        bool isBTaggedMedium() const;
        bool isBTaggedTight() const;

    private:
        Jet* jetPtr;
};

#endif
