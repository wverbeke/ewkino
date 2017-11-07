#include "../interface/tZqTools.h"

unsigned tzq::cat(unsigned nJets, unsigned nBJets){
    if(nBJets == 0){
        if(nJets < 2) return 0;
        else return 1;
    } else if(nBJets == 1){
        if(nJets < 1) return 2;
        else if(nJets < 2) return 3;
        else return 4;
    } else{
        return 5;
    }
}
