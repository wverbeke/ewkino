#ifndef LeptonSelector_H
#define LeptonSelector_H

class Lepton;

class LeptonSelector {
    friend class Lepton;

    public:
        virtual bool isLoose() const = 0;
        virtual bool isFO() const = 0;
        virtual bool isGood() const = 0;
        virtual bool isTight() const = 0;

        virtual ~LeptonSelector(){}

    private:
        virtual LeptonSelector* clone() const & = 0;
        virtual LeptonSelector* clone() && = 0;
};
#endif
