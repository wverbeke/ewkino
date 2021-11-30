#ifndef JetSelector_H
#define JetSelector_H


//include other parts of framework
#include "Jet.h"

class JetSelector{

    public:
        JetSelector( Jet* jPtr ) : jetPtr( jPtr ){}
        
        bool isGood() const{
            if( !isGoodBase() ) return false;
            
            if( jetPtr->is2016PreVFP() ){
                return isGood2016PreVFP();
	    } else if( jetPtr->is2016PostVFP() ){
		return isGood2016PostVFP();
            } else if( jetPtr->is2017() ){
                return isGood2017();
            } else {
                return isGood2018();
            }
        }


        bool isBTaggedLoose() const{
            if( !inBTagAcceptance() ) return false;

            if( jetPtr->is2016PreVFP() ){
                return isBTaggedLoose2016PreVFP();
	    } else if( jetPtr->is2016PostVFP() ){
		return isBTaggedLoose2016PostVFP();
            } else if( jetPtr->is2017() ){
                return isBTaggedLoose2017();
            } else {
                return isBTaggedLoose2018();
            }
        }


        bool isBTaggedMedium() const{
            if( !inBTagAcceptance() ) return false;

            if( jetPtr->is2016PreVFP() ){
                return isBTaggedMedium2016PreVFP();
	    } else if( jetPtr->is2016PostVFP() ){
		return isBTaggedMedium2016PostVFP();
            } else if( jetPtr->is2017() ){
                return isBTaggedMedium2017();
            } else {
                return isBTaggedMedium2018();
            }
        }


        bool isBTaggedTight() const{
            if( !inBTagAcceptance() ) return false;

            if( jetPtr->is2016PreVFP() ){
                return isBTaggedTight2016PreVFP();
	    } else if( jetPtr->is2016PostVFP() ){
		return isBTaggedTight2016PostVFP();
            } else if( jetPtr->is2017() ){
                return isBTaggedTight2017();
            } else {
                return isBTaggedTight2018();
            }
        }

        bool inBTagAcceptance() const;

    private:
        Jet* jetPtr;

        bool isGoodBase() const;
        bool isGood2016PreVFP() const;
	bool isGood2016PostVFP() const;
        bool isGood2017() const;
        bool isGood2018() const;

        bool isBTaggedLoose2016PreVFP() const;
	bool isBTaggedLoose2016PostVFP() const;
        bool isBTaggedLoose2017() const;
        bool isBTaggedLoose2018() const;

        bool isBTaggedMedium2016PreVFP() const;
	bool isBTaggedMedium2016PostVFP() const;
        bool isBTaggedMedium2017() const;
        bool isBTaggedMedium2018() const;

        bool isBTaggedTight2016PreVFP() const;
	bool isBTaggedTight2016PostVFP() const;
        bool isBTaggedTight2017() const;
        bool isBTaggedTight2018() const;
};

#endif
