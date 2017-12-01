#ifndef Reweighter_H
#define Reweighter_H
//Class storing scale-factor weights to be used in events
class Reweighter{
    public:
        Reweighter();
        ~Reweighter();
        inline double puWeight(const double nTrueInt, const unsigned period = 0, const unsigned unc = 0); //period: 0 = 2016, 1 = 2017  unc: 0 = central, 1 = up, 2 = down
        inline double bTagWeight(const unsigned jetFlavor, const double jetPt, const double jetEta, const double jetCSV, const unsigned unc = 0);
        inline double bTagEff(const unsigned jetFlavor, const double jetPt, const double jetEta);
        inline double muonRecoWeight(const double eta);
        inline double electronRecoWeight(const double superClusterEta, const double pt);
        inline double muonIdWeight(const double pt, const double eta);
        inline double electronIdWeight(const double pt, const double eta);
        double muonWeight(const double pt, const double eta){ return muonRecoWeight(eta)*muonIdWeight(pt,eta);}
        double electronWeight(const double pt, const double eta, const double superClusterEta){ return electronRecoWeight(superClusterEta, pt)*electronIdWeight(pt,eta);}
    private:
        //pu scale factors
        TH1D* puWeights[3];
        //btag scale factors and efficiencies
        BTagCalibration* bTagCalib;
        BTagCalibrationReader* bTahCalibReader;
        TH1D* bTagEff[3];
        //reconstruction scale factors
        TGraph* muonRecoSF;
        TH2D* electronRecoSF;
        //muon id scale factors
        TH2D* muonMediumSF;
        TH2D* muonMiniIsoSF;
        TH2D* muonIPSF;
        TH2D* muonSIP3DSF
        TH2D* muonLeptonMvaSF;
        //electron id scale factors
        TH2D* electronIdSF;
        //return jet flavor index 1-> 0, 4 -> 1, 5->2
        unsigned flavorInd(const unsigned jetFlavor){ return 0 + (jetFlavor == 4) + 2*(jetFlavor == 5);}
};
#endif
