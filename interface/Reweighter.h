#ifndef Reweighter_H
#define Reweighter_H
//Class storing scale-factor weights to be used in events
class Reweighter{
    public:
        Reweighter();
        ~Reweighter();
        double puWeight(const double nTrueInt, const unsigned period = 0, const unsigned unc = 0); //period: 0 = 2016, 1 = 2017  unc: 0 = central, 1 = up, 2 = down
        double bTagWeight(const unsigned jetFlavor, const double jetPt, const double jetEta, const double jetCSV, const unsigned unc = 0);
        double muonRecoWeight(const double eta);
        double electronRecoWeight(const double superClusterEta, const double pt);
    private:
        TH1D* puWeights[3];
        BTagCalibration* bTagCalib;
        BTagCalibrationReader* bTahCalibReader;
        TH1D* bTagEff[3];
        TGraph* muonRecoSF;
        TH2D* electronRecoSF;
};
#endif
