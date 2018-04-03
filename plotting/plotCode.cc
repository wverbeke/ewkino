//include c++ library classesQ
#include <algorithm>
#include <math.h>
#include <iterator>
#include <utility>
#include <iostream>
//include Root classes
#include "TCanvas.h"
#include "TLine.h"
#include "TGraphAsymmErrors.h"
//Include other parts of the code
#include "plotCode.h"
#include "drawLumi.h"

extern const double xPad;
extern const Color_t colors[];

//set color of histogram to be plotted separately
void histcol(TH1D* h, const Color_t color){
    h->SetLineColor(color);
    h->SetMarkerColor(color);
    h->SetLineWidth(2);
}
//set color of histogram to be plotted in a stack
void StackCol(TH1D* h, const Color_t color){
    histcol(h,color);
    h->SetFillColor(color);
    h->SetLineWidth(1);
    h->SetLineColor(kBlack); //black line between the stack elements
}
//set histogram axis and label sizes simulataneously
void HistLabelSizes(TH1D* h, const double xLabel, const double xTitle, const double yLabel, const double yTitle){
    h->GetXaxis()->SetLabelSize(xLabel);
    h->GetXaxis()->SetTitleSize(xTitle);
    h->GetYaxis()->SetLabelSize(yLabel);
    h->GetYaxis()->SetTitleSize(yTitle);
}
//Order an array of histograms by yield
void yieldOrder(TH1D** hists, const unsigned nHist, const bool* isSMSignal){
    if(isSMSignal == nullptr){  //No SM signal that has to be stacked on top
        std::vector<TH1D*> temp(hists, hists + nHist);
        std::sort(temp.begin(), temp.end(), [](const TH1D* h1, const TH1D* h2){ return h1->GetSumOfWeights() > h2->GetSumOfWeights(); } );
        for(unsigned h = 0; h < nHist; ++h){
            hists[h] = temp[h];
        }
    } else{
        std::vector<std::pair < TH1D*, bool > > sigMap(nHist);
        for(unsigned h = 0; h < nHist; ++h){
            sigMap[h] = {hists[h], isSMSignal[h]};
        }
        std::sort(sigMap.begin(), sigMap.end(), [](const std::pair<TH1D*, bool>& p1, const std::pair<TH1D*, bool>& p2){ return (p1.first)->GetSumOfWeights() > (p2.first)->GetSumOfWeights(); } );
        std::sort(sigMap.begin(), sigMap.end(), [](const std::pair<TH1D*, bool>& p1, const std::pair<TH1D*, bool>& p2){ return (p1.second) > (p2.second); } );
        for(unsigned h = 0; h < nHist; ++h){
            hists[h] = sigMap[h].first;
        }
    }
}

Color_t bkgColorEWK(const std::string& bkgName){
    if(bkgName == "non-prompt") return kAzure + 1;
    else if(bkgName == "WZ") return kOrange;
    else if(bkgName == "ZZ/H") return  kGreen + 1;
    else if(bkgName == "TT/T + X") return  kViolet-3;
    else if(bkgName == "triboson") return kRed + 1;
    else if(bkgName == "X + #gamma") return kOrange + 7;
    else if(bkgName == "T + X") return kCyan + 1;
    else return kBlack;
}

Color_t bkgColorEWKDilept(const std::string bkgName){
    if(bkgName == "DY") return kAzure + 1;
    else if(bkgName == "TT + Jets") return kRed - 7;
    else if(bkgName == "TT dilep.") return kRed - 7;
    else if(bkgName == "TT semilep.") return kBlue - 7;
    else if(bkgName == "VV") return kCyan + 1;
    else if(bkgName == "WJets") return kOrange;
    else if(bkgName == "TT + X") return kGreen + 1;
    else if(bkgName == "T + X") return kMagenta -7;
    else return kBlack;
}

Color_t bkgColorHNL(const std::string& bkgName){
    if(bkgName == "non-prompt") return kAzure + 1;
    else if(bkgName == "nonprompt") return kAzure + 1;
    else if(bkgName == "WZ") return kRed - 7;
    else if(bkgName == "X + #gamma") return  kGreen + 1; //+ 1// -7 // -9
    else if(bkgName == "X#gamma^{(*)}") return  kGreen + 1; //+ 1// -7 // -9
    else if(bkgName == "TT/T + X" || bkgName == "t#bar{t}/t + X") return kMagenta -7;
    else if(bkgName == "ZZ/H") return kOrange + 6;
    else if(bkgName == "triboson") return kBlue - 7;
    //For MC fakes
    else if(bkgName == "Drell-Yan") return kAzure + 1;
    else if(bkgName == "TT") return kCyan + 1;
    else return kBlack;
}

Color_t bkgColortZq(const std::string& bkgName){
    if(bkgName == "tZq") return kRed - 7;
    if(bkgName == "TT" ||  bkgName == "TT + Jets") return kOrange - 2;
    if(bkgName == "nonprompt" ||  bkgName == "non-prompt") return kOrange - 2;
    if(bkgName == "TT/T + X" || bkgName == "TT + X") return kBlue - 10;
    if(bkgName == "X + #gamma") return kMagenta - 10;
    if(bkgName == "multiboson") return kYellow - 9;
    if(bkgName == "Multiboson") return kYellow - 9;
    if(bkgName == "Drell-Yan" || bkgName == "DY") return kViolet - 7;
    if(bkgName == "WJets") return kSpring + 2;
    if(bkgName == "TT + Z") return kBlue - 6; 
    if(bkgName == "ZZ/H") return kTeal - 4;
    if(bkgName == "WZ") return kCyan - 7;
    else return kBlack;    
}


//FIND WAY TO RESET THE COUNTER AFTER EVERY PLOT SO THAT COLOR ORDERING IS CONSISTENT!!
Color_t bkgColorGeneral(const bool reset = false){
    static unsigned counter = 0;
    static const Color_t colors[9] = {kBlue + 1, kAzure + 1, kGreen - 7, kMagenta -7, kRed - 7, kOrange + 6, kCyan + 1, kMagenta +3, kBlue -3};
    if(!reset){
        Color_t output = colors[counter];
        ++counter;
        if(counter == 8) counter = 0;
        return output;
    } else{
        counter = 0;
        return kBlack;
    }
}


Color_t bkgColor(const std::string& bkgName, const std::string& analysis){
    if(analysis == "ewkino" || analysis == "EWK" || analysis == "EWKino"){
        return bkgColorEWK(bkgName);
    } else if(analysis == "HNL"){
        return bkgColorHNL(bkgName);
    } else if(analysis == "ewkinoDilep"){
        return bkgColorEWKDilept(bkgName);
    } else if(analysis == "tZq" || analysis == "TZQ" || analysis == "tzq"){
        return bkgColortZq(bkgName);
    } else{
        return bkgColorGeneral();
    }
}

void plotDataVSMC(TH1D* data, TH1D** bkg, const std::string* names, const unsigned nBkg, const std::string& file, const std::string& analysis, const bool ylog, const bool normToData, const std::string& header, TH1D** bkgSyst, const bool* isSMSignal, TH1D** signal, const std::string* sigNames, const unsigned nSig, const bool sigNorm){
    
    //do not make empty plots
    bool isEmpty = true;
    for(unsigned h = 0; h < nBkg; ++h){
        if(bkg[h]->GetSumOfWeights() > 0){
            isEmpty = false;
            break;
        }  
    }
    if(data->GetSumOfWeights() > 0.){
        isEmpty = false;
    } 
    if(isEmpty){
        std::cerr << "attempting to print empty plot, returning control" << std::endl;
        return;
    }

    //set background histogram colors
    for(unsigned h = 0; h < nBkg; ++h){
        StackCol(bkg[h], bkgColor(names[h + 1], analysis) ); //first name is data
    }    
    //color signal histgrams if they are to be plotted
    if(signal != nullptr){
        for(unsigned s = 0; s < nSig; ++s){
            signal[s]->SetLineColor( bkgColor("", "") );
            signal[s]->SetMarkerColor( bkgColor("", "") );
            signal[s]->SetLineWidth(3);
            signal[s]->SetFillStyle(0);
        }
        bkgColorGeneral(true); //reset colors so plots are consistent
    }

    //set Poisonian errors to data
    data->SetBinErrorOption(TH1::kPoisson);

    //Replace data by TGRaphAsymmErrors for plotting
    TGraphAsymmErrors* dataGraph = new TGraphAsymmErrors(data);
    for(int b = 1; b < data->GetNbinsX() + 1; ++b){
        dataGraph->SetPointError(b - 1, 0, 0, data->GetBinErrorLow(b), (data->GetBinContent(b) == 0 ) ? 0 : data->GetBinErrorUp(b) );
    }

    
    //Compute total background (needed later for uncertainty bands)
    TH1D* bkgTot = (TH1D*) bkg[0]->Clone();
    for(unsigned h = 1; h < nBkg; ++h){
        bkgTot->Add(bkg[h]);
    }

    //normalize background to data if option is chosen
    if(normToData){
        double SF = data->GetSumOfWeights()/bkgTot->GetSumOfWeights();
        for(unsigned h = 0; h < nBkg; ++h){
            bkg[h]->Scale(SF);
        }
        //Remcompute total background after scaling
        bkgTot = (TH1D*) bkg[0]->Clone();
        for(unsigned h = 1; h < nBkg; ++h){
            bkgTot->Add(bkg[h]);
        }
    }

    //normalize signal to data if option is chosen
    if(signal != nullptr && sigNorm){
        for(unsigned s = 0; s < nSig; ++s){
            double SF = data->GetSumOfWeights()/signal[s]->GetSumOfWeights();
            signal[s]->Scale(SF);
        }
    }

    //background histograms with full uncertainty
    TH1D* bkgE[nBkg]; //clone histograms so that the function does not affect its arguments!
    for(unsigned h = 0; h < nBkg; ++h){
        bkgE[h] = (TH1D*) bkg[h]->Clone(); //CHECK WHETHER THIS IS MEMORY SAFE
        if(bkgSyst != nullptr){
            for(int bin = 1; bin < bkgE[h]->GetNbinsX() + 1; ++bin){
                bkgE[h]->SetBinError(bin, sqrt(bkgE[h]->GetBinError(bin)*bkgE[h]->GetBinError(bin) + bkgSyst[h]->GetBinContent(bin)*bkgSyst[h]->GetBinContent(bin)) );
            }
        }
    }

    //compute total background
    TH1D* bkgTotE = (TH1D*) bkgE[0]->Clone(); //CHECK WHETHER THIS IS MEMORY SAFE
    for(unsigned h = 1; h < nBkg; ++h){
        bkgTotE->Add(bkgE[h]);
    }

    //make the total background uncertainty visible as a grey band
    bkgTotE->SetFillStyle(3244); //3005  3244
    bkgTotE->SetFillColor(kGray+2);
    bkgTotE->SetMarkerStyle(0); //1

    //make legend and add all histograms
    TLegend legend = TLegend(0.2,0.8,0.95,0.9,NULL,"brNDC");
    legend.SetNColumns(4);
    legend.SetFillStyle(0); //avoid legend box
    legend.AddEntry(dataGraph, (const TString&) names[0], "pe1"); //add data to legend
    for(unsigned h = 0; h < nBkg; ++h){
        legend.AddEntry(bkgE[h], (const TString&) names[h + 1], "f"); //add backgrounds to the legend
    }
    legend.AddEntry(bkgTotE, "Total bkg. unc.", "f"); //add total background uncertainty to legend

    //add signal to legend if plotting signal
    if(signal != nullptr){
        for(unsigned s = 0; s < nSig; ++s){
            legend.AddEntry(signal[s], (const TString&) sigNames[s], "l");
        }
    }
    
    //order background histograms by yield
    yieldOrder(bkgE, nBkg, isSMSignal);

    //add background histograms to stack
    THStack bkgStack = THStack("bkgStack", "bkgStack");
    for(unsigned h = 0; h < nBkg; ++h){
        bkgStack.Add(bkgE[nBkg - h - 1]); //Put highest yield on top -> good for log scale plots
    }
    
    //canvas dimenstions
    const double width = 600*(1 - xPad);
    const double height = 600;

    //make canvas to plot
    TCanvas* c = new TCanvas((const TString&) file,"",width,height);
    c->cd();

    //make upper pad to draw main plot and lower pad for ratios
    TPad* p1,* p2;

    //prepare first pad for plotting data and background yields
    p1 = new TPad((const TString&) file,"",0,xPad,1,1);
    p1->Draw();
    p1->cd();
    p1->SetBottomMargin(0.03);

    //make pad logarithmic if needed
    if(ylog) p1->SetLogy();
    
    /*
    From now on we will determine the range of the plot from the total background histogram which will always be drawn first
    in order to force the range of the canvas.
    */

    //set minimum to zero
    if(!ylog) bkgTotE->SetMinimum(0);
    
    //x-axis labels will only be drawn in the lower (ratio) canvas
    bkgTotE->GetXaxis()->SetLabelSize(0);

    //determine the maximum range of data and the backgrounds
    double totalMax = data->GetBinContent(data->GetMaximumBin()) + data->GetBinError(data->GetMaximumBin());
    totalMax = std::max(totalMax, bkgTotE->GetBinContent(bkgTotE->GetMaximumBin()) + bkgTotE->GetBinError(bkgTotE->GetMaximumBin()) );
    
    //take signal into account when determining plotting range
    if(signal != nullptr){
        double signalMax = 0;
        for(unsigned s = 0; s < nSig; ++s){
            signalMax = std::max( signalMax, signal[s]->GetBinContent(signal[s]->GetMaximumBin()) + signal[s]->GetBinError(signal[s]->GetMaximumBin()) );
        }
        totalMax = std::max(totalMax, signalMax);
    }

    //determine upper limit of plot
    if(!ylog){
        bkgTotE->SetMaximum(totalMax*1.3);
        //hack not to draw 0 observed event points
        for(int b = 1; b < data->GetNbinsX() + 1; ++b){
            if(dataGraph->GetY()[b - 1] == 0)  dataGraph->GetY()[b - 1] += totalMax*10;
        }
    } else{
        double minimum = totalMax; //find pad minimum when plotting on a log scale
        for(int b = 1; b < bkgTotE->GetNbinsX() + 1; ++b){
            if(bkgTotE->GetBinContent(b) != 0 && bkgTotE->GetBinContent(b) < minimum){
                minimum = bkgTotE->GetBinContent(b);
            }
        }
        double SF = 5*log10(std::max(10., totalMax/minimum));
        bkgTotE->SetMinimum(minimum/5.);
        bkgTotE->SetMaximum(totalMax*3*SF);
        //hack not to draw 0 observed event points
        for(int b = 1; b < data->GetNbinsX() + 1; ++b){
           if(data->GetBinContent(b) == 0)  dataGraph->GetY()[b - 1] += totalMax*30*SF;
        }
    }			

    //draw histograms and legends
    //first draw total background to fix plot range
    bkgTotE->Draw("e2");
    bkgStack.Draw("hist same");
    legend.Draw("same");
    bkgTotE->Draw("e2 same"); //Redraw data so it is overlaid on the background stack
    dataGraph->Draw("pe1 same");	

    //plot signal if this option is chosen
    if(signal != nullptr){
        for(unsigned s = 0; s < nSig; ++s){
            signal[s]->Draw("histsame");
        }
    }
    
    //redraw axis over histograms
    gPad->RedrawAxis();

    //draw CMS header
    if(header == "") drawLumi(p1);
    else drawLumi(p1, "Preliminary", (const TString&) header);

    //make ratio plot in second pad
    c->cd(); 
    p2 = new TPad((const TString&) file + "2","",0,0.0,1,xPad);
    p2->Draw();
    p2->cd();
    p2->SetTopMargin(0.01);     //small space between two pads
    p2->SetBottomMargin(0.4);

    //make separate histograms containing total and statistical background uncertainty which will be used to plot uncertainty bands
    const unsigned nBins = data->GetNbinsX();
    TH1D* bkgStatErrors = new TH1D((const TString&) "bkgStaterrors" + file, (const TString&) "bkgStaterrors" + file, nBins, data->GetBinLowEdge(1), data->GetBinLowEdge(data->GetNbinsX()) + data->GetBinWidth(data->GetNbinsX()));
    TH1D* bkgErrors = (TH1D*) bkgTotE->Clone();
    for(unsigned b = 1; b < nBins + 1; ++b){
        bkgStatErrors->SetBinContent(b, 1.);    //center bands around 0
        bkgErrors->SetBinContent(b, 1.);
        if(bkgTotE->GetBinContent(b) != 0){
            bkgStatErrors->SetBinError(b, bkgTot->GetBinError(b)/bkgTot->GetBinContent(b));
            bkgErrors->SetBinError(b, bkgTotE->GetBinError(b)/bkgTotE->GetBinContent(b));
        } else{
            bkgStatErrors->SetBinError(b, 0.);
            bkgErrors->SetBinError(b, 0.);
        }			
    }

    //set style of uncertainty bands 
    bkgStatErrors->SetFillStyle(1001);
    bkgErrors->SetFillStyle(1001);
    bkgStatErrors->SetFillColor(kCyan - 4);
    bkgErrors->SetFillColor(kOrange - 4);
    bkgStatErrors->SetMarkerStyle(1);
    bkgErrors->SetMarkerStyle(1);

    //make TGraph asymmErros to plot data with the correct uncertainties
    TGraphAsymmErrors* obsRatio = new TGraphAsymmErrors(data);
    for(int b = 1; b < data->GetNbinsX() + 1; ++b){
        obsRatio->GetY()[b - 1] *= 1./bkgTotE->GetBinContent(b);
        obsRatio->SetPointError(b - 1, 0, 0, data->GetBinErrorLow(b)/bkgTotE->GetBinContent(b), data->GetBinErrorUp(b)/bkgTotE->GetBinContent(b));
        if(data->GetBinContent(b) == 0) obsRatio->GetY()[b - 1] += 5;
    }

    //legend for uncertainties
    TLegend legend2 = TLegend(0.18,0.85,0.8,0.98,NULL,"brNDC");
    legend2.SetNColumns(3); 
    legend2.SetFillStyle(0); //avoid legend box 
    legend2.AddEntry(bkgStatErrors, "Stat. pred. unc.", "f");
    legend2.AddEntry(bkgErrors, "Total pred. unc.", "f");
    legend2.AddEntry(obsRatio, "Obs./Pred.", "pe12");

    /*
    We will set up the range and label sizes of the plot using bkgErros. As such this histogram always has to be 
    drawn first on the pad to fix the plotted labels.
    */
    bkgErrors->SetMarkerColor(1);
    bkgErrors->SetLineColor(1);
    bkgErrors->GetYaxis()->SetRangeUser(0.,1.999);
    bkgErrors->GetYaxis()->SetTitle("Obs./Pred.");
    bkgErrors->GetYaxis()->SetTitleOffset(1.25/((1.-xPad)/xPad));
    bkgErrors->GetYaxis()->SetTitleSize((1.-xPad)/xPad*0.06);
    bkgErrors->GetXaxis()->SetTitleSize((1.-xPad)/xPad*0.06);
    bkgErrors->GetYaxis()->SetLabelSize((1.-xPad)/xPad*0.05);
    bkgErrors->GetXaxis()->SetLabelSize((1.-xPad)/xPad*0.05);
    bkgErrors->GetXaxis()->SetLabelOffset((1-xPad)/xPad*0.009);
    bkgErrors->GetXaxis()->SetTitleOffset((1-xPad)/xPad*0.32);

    //draw objects on pad
    bkgErrors->Draw("e2");
    bkgErrors->Draw("e2 same");
    bkgStatErrors->Draw("e2 same");
    obsRatio->Draw("pe1 same");
    legend2.Draw("same");
    gPad->RedrawAxis();

    //draw line at 1 on ratio plot
    double xmax = data->GetBinCenter(data->GetNbinsX()) + data->GetBinWidth(data->GetNbinsX())/2;
    double xmin = data->GetBinCenter(0) + data->GetBinWidth(0)/2;
    TLine line = TLine(xmin, 1, xmax, 1);
    line.SetLineStyle(2);
    line.Draw("same");

    //save canvas to file
    c->SaveAs((const TString&) file + ".pdf");
    c->SaveAs((const TString&) file + ".png");
    
    //Clean up memory 
    delete dataGraph;
    delete obsRatio;
    delete bkgStatErrors;
    delete p1;
    delete p2;
    delete c;
}
