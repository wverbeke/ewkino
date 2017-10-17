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
//Color order for plots and the background stack
const Color_t colors[8] = {kAzure + 1, kGreen - 7, kMagenta -7, kRed - 7, kBlue -3, kOrange + 6, kCyan + 1, kMagenta +3};//kGreen - 7
//Set histogram colors and lines
void histcol(TH1D *, const Color_t);
//Set Stack colors
void StackCol(TH1D *h, const Color_t);
//Set Histogram labelsizes
void HistLabelSizes(TH1D *h, const double xlabel = 0.045, const double xtitle = 0.05, const double ylabel = 0.045, const double ytitle = 0.045);
//Order histograms in terms of yields (biggest first)
void yieldOrder(TH1D**, unsigned*, const unsigned);
//Plot background stack and compare to observed yields
void plotDataVSMC(TH1D* data, TH1D** bkg, const std::string* names, const unsigned nBkg, const std::string& file, const std::string& analysis = "", const bool ylog = false, TH1D** bkgSyst = nullptr);
//return color corresponding to bkg
Color_t bkgColor(const std::string&); 
#endif
