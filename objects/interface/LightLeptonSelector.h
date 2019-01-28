#ifndef LightLeptonSelector_H
#define LightLeptonSelector_H

class LightLepton;

class LightLeptonSelector : public LeptonSelector {

    public: 
        LightLepton( const LightLepton& ) : lightLeptonPtr( &LightLepton ) {}
        
        virtual bool isLoose() const override;
        virtual bool isGood() const override;
        virtual bool isTight() const override;

    private:
        LightLepton* lightLeptonPtr;

};


#endif 
