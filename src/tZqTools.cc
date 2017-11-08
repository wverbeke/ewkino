#include "../interface/tZqTools.h"
#include "../interface/trilepTools.h"

unsigned tzq::cat(unsigned nJets, unsigned nBJets){
    if(nBJets == 0){
        if(nJets < 2) return 0;
        else return 1;
    } else if(nBJets == 1){
        if(nJets < 2) return 2;
        else if(nJets < 4) return 3;
        else return 4;
    } else{
        return 5;
    }
}

double tzq::findMTop(const TLorentzVector& wLep, const TLorentzVector& met, std::vector<unsigned>& taggedJetI, const std::vector<unsigned>& jetI, const std::vector<unsigned>& bJetI, const TLorentzVector* jetV){
    static const double mTop = 173.1;
    std::pair<double, double> pzSol = trilep::neutrinoPZ(wLep, met);
    TLorentzVector neutrinoPlus(met.Px(), met.Py(), pzSol.first, met.Px()*met.Px() + met.Py()*met.Py() + pzSol.first*pzSol.first);
    TLorentzVector neutrinoMin(met.Px(), met.Py(), pzSol.second, met.Px()*met.Px() + met.Py()*met.Py() + pzSol.second*pzSol.second);
    bool plus;  //true if + solution is chosen, otherwise false
    if(bJetI.size() == 0){
        taggedJetI[0] = jetI[0];
    } else if(bJetI.size() == 1){
        taggedJetI[1] = bJetI[0];
    } else{
        double minDiff = 999999.;
        for(unsigned b = 0; b < bJetI.size(); ++b){
            double diffPlus = fabs( (neutrinoPlus + wLep + jetV[bJetI[b]] ).M() - mTop);
            double diffMin = fabs( (neutrinoMin + wLep + jetV[bJetI[b]] ).M() - mTop);
            if(diffPlus < minDiff && diffPlus < diffMin){
                plus = true;
                taggedJetI[0] = bJetI[b];
            } else if(diffMin < minDiff){
                plus = false;
                taggedJetI[0] = bJetI[b];
            }
        }
    }
    if(bJetI.size() < 2){
        if(fabs( (neutrinoPlus + wLep + jetV[taggedJetI[0]] ).M() - mTop)  < fabs( (neutrinoMin + wLep + jetV[taggedJetI[0]] ).M() - mTop) ){
            plus = true;
        } else{
            plus = false;
        }
    }
    //find recoiling jet ( := leading jet not considered the b-jet from top)
    for(unsigned j = 0; j < jetI.size(); ++j){
        if(jetI[j] != taggedJetI[0]){
            taggedJetI[1] = jetI[j];
            break;                              //jet collection is assumed oredered
        }
    }
    if(plus) return ( neutrinoPlus + wLep + jetV[taggedJetI[0]] ).M();
    else     return ( neutrinoMin  + wLep + jetV[taggedJetI[0]] ).M();
}
