#ifndef PlotScript
#define PlotScript
 
//import ROOT classes
#include "TH1D.h"
#include "TFile.h"
#include "TString.h"
#include "TLegend.h"
#include "TCanvas.h"
#include "THStack.h"
#include "TH2D.h"
 
//const double xPad = 0.35;
const double xPad = 0.25;
//Set histogram colors and lines
void histcol(TH1D*, const Color_t);
//Set Stack colors
void StackCol(TH1D*, const Color_t);
//Set Histogram labelsizes
void HistLabelSizes(TH1D* h, const double xLabel = 0.045, const double xTitle = 0.05, const double yLabel = 0.045, const double yTitle = 0.045);
//Order histograms in terms of yields (biggest first)
void yieldOrder(TH1D**);
//return color corresponding to bkg
Color_t bkgColor(const std::string&, const std::string& analysis = ""); 
//Plot background stack and compare to observed yields
void plotDataVSMC(TH1D* data, TH1D** bkg, const std::string* names, const unsigned nBkg, const std::string& file, const std::string& analysis = "", const bool ylog = false, const bool normToData = false, const std::string& header = "", TH1D** bkgSyst = nullptr, const bool* isSMSignal = nullptr);
#endif
