#ifndef Reweighter_H
#define Reweighter_H
//Class storing scale-factor weights to be used in events
class Reweighter{
    public:
        Reweighter();
        ~Reweighter();
        double puWeight(const double nTrueInt, const unsigned period = 0, const unsigned unc = 0); //period: 0 = 2016, 1 = 2017  unc: 0 = central, 1 = up, 2 = down
        double bTagWeight(const unsigned jetFlavor, const double jetPt, const double jetEta, const double jetCSV, const unsigned unc = 0);
        double bTagEff(const unsigned jetFlavor, const double jetPt, const double jetEta);
        double muonRecoWeight(const double eta);
        double electronRecoWeight(const double superClusterEta, const double pt);
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
        TH1D* muonMediumSF;
        TH1D* muonMiniIsoSF;
        TH1D* muonIPSF;
        TH1D* leptonMvaSF;
        //return jet flavor index 1-> 0, 4 -> 1, 5->2
        unsigned flavorInd(const unsigned jetFlavor){ return 0 + (jetFlavor == 4) + 2*(jetFlavor == 5);}
};
#endif
