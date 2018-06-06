#ifndef tZqTools_H
#define tZqTools_H

//include c++ library headers

//include ROOT headers
#include "TLorentzVector.h"

namespace tzq{
    unsigned cat(unsigned nJets, unsigned nBJets);
    TLorentzVector findBestNeutrinoAndTop(const TLorentzVector& wLep, const TLorentzVector& met, std::vector<unsigned>& taggedJetI, const std::vector<unsigned>& jetI, const std::vector<unsigned>& bJetI, const TLorentzVector* jetV);
    unsigned controlRegion(const unsigned lCount, const double mll, const double m3l);
    inline bool isWZControlRegion(const unsigned cr) { return ( cr == 0 ); };
    inline bool isXgammaControlRegion(const unsigned cr) { return ( cr == 1 ); }; 
    inline bool isZZControlRegion(const unsigned cr) { return ( cr == 2 ); };
    inline bool isControlRegion(const unsigned cr) { return (cr < 3 ); } 
}
#endif
