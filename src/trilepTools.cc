#include "../interface/trilepTools.h"

std::pair<unsigned, unsigned> trilep::bestZ(const TLorentzVector* lepV, const std::vector<unsigned>& ind, const unsigned* flavor, const int* charge, const unsigned lCount){
    //currently only works for events with an OSSF pair! dangerous otherwise
    static const double mZ = 91.1876;
    std::pair<unsigned, unsigned> bestZInd = {0, 1};
    for(unsigned l = 1; l < lCount - 1; ++l){
        for(unsigned k = l + 1; k < lCount; ++k){
            if(flavor[ind[k]] == flavor[ind[l]] && charge[ind[k]] != charge[ind[l]]){
                if( fabs( (lepV[l] + lepV[k]).M() - mZ) < fabs((lepV[bestZInd.first] + lepV[bestZInd.second]).M() - mZ) ){
                    bestZInd = {l, k};
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
    double mSquared = 0.5*mW*mW + wLep.Pt()*met.Pt()*cos(wLep.Phi() - met.Phi() );
    double preFac = mSquared/(wLep.Pt() * wLep.Pt());
    double term2 = wLep.P()*sqrt( std::max(0., 1 - met.Pt()*met.Pt()*wLep.Pt()*wLep.Pt()/(mSquared*mSquared) ) );
    return {preFac*(wLep.Pz() + term2), preFac*(wLep.Pz() - term2)};
}
