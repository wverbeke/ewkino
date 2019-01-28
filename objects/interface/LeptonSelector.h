#ifndef LeptonSelector_H
#define LeptonSelector_H

class LeptonSelector {

    public:
        virtual bool isLoose() const = 0;
        virtual bool isGood() const = 0;
        virtual bool isTight() const = 0;

};
#endif
