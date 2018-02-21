#ifndef Reweighter_H
#define Reweighter_H

//include c++ library classes

//include ROOT classes
#include "TH1D.h"
#include "TH2D.h"
#include "TGraph.h"

//include other parts of code 
#include "../bTagSFCode/BTagCalibrationStandalone.h"

//Class storing scale-factor weights to be used in events
class Reweighter{
    public:
        Reweighter();
        ~Reweighter();

        //pileup weight
        double puWeight(const double nTrueInt, const unsigned period = 0, const unsigned unc = 0) const; //period: 0 = 2016, 1 = 2017  unc: 0 = central, 1 = up, 2 = down

        //b-tag weight
        double bTagWeight(const unsigned jetFlavor, const double jetPt, const double jetEta, const double jetCSV, const unsigned unc = 0) const;

        //b-tagging efficiency
        double bTagEff(const unsigned jetFlavor, const double jetPt, const double jetEta) const;

        //lepton id + reconstruction weight
        double muonWeight(const double pt, const double eta) const{ 
            return muonRecoWeight(eta)*muonIdWeight(pt,eta);
        }

        double electronWeight(const double pt, const double eta, const double superClusterEta) const{ 
            return electronRecoWeight(superClusterEta, pt)*electronIdWeight(pt,eta);
        }
    
        //fakerates 
        double muonFakeRate(const double pt, const double eta, const unsigned unc = 0) const;
        double electronFakeRate(const double pt, const double eta, const unsigned unc = 0) const;

    private:
        //pu scale factors
        TH1D* puWeights[3];

        //btag scale factors and efficiencies
        BTagCalibration* bTagCalib;
        BTagCalibrationReader* bTagCalibReader;
        TH1D* bTagEffHist[3];

        //reconstruction scale factors
        TGraph* muonRecoSF;
        TH2D* electronRecoSF;

        //muon id scale factors
        TH2D* muonMediumSF;
        TH2D* muonMiniIsoSF;
        TH2D* muonIPSF;
        TH2D* muonSIP3DSF;
        TH2D* muonLeptonMvaSF;

        //electron id scale factors
        //TH2D* electronIdSF;
        TH2D* electronEmuIPMvaLooseSF;
        TH2D* electronMiniIsoSF;
        TH2D* electronConvVetoMissingHitsSF;

        //return jet flavor index 1-> 0, 4 -> 1, 5->2
        unsigned flavorInd(const unsigned jetFlavor) const{ 
            return 0 + (jetFlavor == 4) + 2*(jetFlavor == 5);
        }

        //fake rate maps
        TH2D* frMapEle[3];
        TH2D* frMapMu[3];
        
        //tracking + reconstruction weights
        double muonRecoWeight(const double eta) const;
        double electronRecoWeight(const double superClusterEta, const double pt) const;

        //id weights
        double muonIdWeight(const double pt, const double eta) const;
        double electronIdWeight(const double pt, const double eta) const;
};
#endif
