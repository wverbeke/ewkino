#include "../interface/trilepTools.h"

std::pair<unsigned, unsigned> trilep::bestZ(const TLorentzVector* lepV, const std::vector<unsigned>& ind, const unsigned* flavor, const int* charge, const unsigned lCount){
    //currently only works for events with an OSSF pair! dangerous otherwise
    static const double mZ = 91.1876;
    std::pair<unsigned, unsigned> bestZInd;
    bool first = true;
    for(unsigned l = 0; l < lCount - 1; ++l){
        for(unsigned k = l + 1; k < lCount; ++k){
            if(flavor[ind[k]] == flavor[ind[l]] && charge[ind[k]] != charge[ind[l]]){
                if( first || fabs( (lepV[l] + lepV[k]).M() - mZ) < fabs((lepV[bestZInd.first] + lepV[bestZInd.second]).M() - mZ) ){
                    bestZInd = {l, k};
                    first = false;
                }
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

std::pair<double, double> trilep::neutrinoPZ(const TLorentzVector& wLep, const TLorentzVector& met){
    static const double mW = 80.385;
    double mSquared = 0.5*mW*mW + wLep.Px()*met.Px() + wLep.Py()*met.Py();
    double preFac = mSquared/(wLep.Pt() * wLep.Pt());
    double term2 = wLep.P()*sqrt( std::max(0., 1 - met.Pt()*met.Pt()*wLep.Pt()*wLep.Pt()/(mSquared*mSquared) ) );
    return {preFac*(wLep.Pz() + term2), preFac*(wLep.Pz() - term2)};
}

enum leptonFlavorTrilep
{
    eee, //0
    eem, //1
    emm, //2
    mmm, //3
    eet, //4
    emt, //5
    mmt, //6
    ett, //7
    mtt, //8
    ttt  //9
};

unsigned trilep::flavorComposition(const std::vector<unsigned>& ind, const unsigned* flavor, const unsigned lCount){    
    unsigned flavCount[3] = {0,0,0};
    for(unsigned l = 0; l < lCount; ++l) ++flavCount[flavor[ind[l]]];
    if(flavCount[2] == 0){
        if(flavCount[1] == 0){
            return eee;
        } else if(flavCount[1] == 1){
            return eem;
        } else if(flavCount[1] == 2){
            return emm;
        } else{
            return mmm;
        }
    } else if(flavCount[2] == 1){
        if(flavCount[1] == 0){
            return eet;
        } else if(flavCount[1] == 1){
            return emt;
        } else{
            return mmt;
        }
    } else if(flavCount[2] == 2){
        if(flavCount[1] == 0){
            return ett;
        } else{
            return mtt;
        }
    } else{
        return ttt;
    }
}

enum leptonFlavorFourlep
{
    eeee, //0
    eeem, //1
    eemm, //2
    emmm, //3
    mmmm, //4
    eeet, //5
    eemt, //6
    emmt, //7
    mmmt, //8
    eett, //9
    emtt, //10
    mmtt, //11
    ettt, //12
    mttt, //13
    tttt //14
};

unsigned trilep::flavorCompositionFourlepton(const std::vector<unsigned>& ind, const unsigned* flavor, const unsigned lCount){
    unsigned flavCount[3] = {0, 0, 0};
    for(unsigned l = 0; l < lCount; ++l) ++flavCount[flavor[ind[l]]];
    if(flavCount[2] == 0){
        if(flavCount[1] == 0){
            return eeee;
        } else if(flavCount[1] == 1){
            return eeem;
        } else if(flavCount[1] == 2){
            return eemm;
        } else if(flavCount[1] == 3){
            return emmm;
        } else{
            return mmmm;
        }
    } else if(flavCount[2] == 1){
        if(flavCount[1] == 0){
            return eeet;
        } else if(flavCount[1] == 1){
            return eemt;
        } else if(flavCount[1] == 2){
            return emmt;
        } else{
            return mmmt;
        }
    } else if(flavCount[2] == 2){
        if(flavCount[1] == 0){
            return eett;
        } else if(flavCount[1] == 1){
            return emtt;
        } else {
            return mmtt;
        }
    } else if(flavCount[2] == 3){
        if(flavCount[1] == 0){
            return ettt;
        } else {
            return mttt;
        }
    } else{
        return tttt;
    }
}
