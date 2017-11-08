#ifndef tZqTools_H
#define tZqTools_H

//include c++ library headers

//include ROOT headers
#include "TLorentzVector.h"

namespace tzq{
    unsigned cat(unsigned nJets, unsigned nBJets);
    double findMTop(const TLorentzVector& wLep, const TLorentzVector& met, std::vector<unsigned>& taggedJetI, const std::vector<unsigned>& jetI, const std::vector<unsigned>& bJetI, const TLorentzVector* jetV);
}
#endif
