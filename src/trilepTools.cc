#include "../interface/trilepTools.h"

std::pair<unsigned, unsigned> trilep::bestZ(const TLorentzVector* lepV, const unsigned lCount){
    static const double mZ = 91.1876;
    std::pair<unsigned, unsigned> bestZInd = {0, 1};
    for(unsigned l = 1; l < lCount - 1; ++l){
        for(unsigned k = l + 1; k < lCount; ++k){
            if( fabs( (lepV[l] + lepV[k]).M() - mZ) < (lepV[bestZInd.first] + lepV[bestZInd.second]).M() ){
                bestZInd = {l, k};
            }
        }
    }
    return bestZInd;
}

unsigned trilep::flavorChargeComb(const std::vector<unsigned>& ind, const unsigned* flavor, const int* charge, const unsigned lCount){
    bool OSOF = false;
    for(unsigned l = 0; l < lCount - 1; ++l){
        for(unsigned k = l + 1; k < lCount; ++k){
            if(charge[ind[l]] != charge[ind[k]]){
                if(flavor[ind[l]] == flavor[ind[k]]){
                    return 0;
                }
                else{
                    OSOF = true;
                }
            }
        }
    }
    return (1 + !OSOF);     //0 = OSSF, 1 = OSOF, 2 = SSS
}
