//include c++ library classesQ
#include <algorithm>
#include <math.h>
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
void histcol(TH1D *h, const Color_t color){
    h->SetLineColor(color);
    h->SetMarkerColor(color);
    h->SetLineWidth(2);
}
//set color of histogram to be plotted in a stack
void StackCol(TH1D *h, const Color_t color){
    histcol(h,color);
    h->SetFillColor(color);
    h->SetLineWidth(1);
    h->SetLineColor(kBlack); //black line between the stack elements
}
//set histogram axis and label sizes simulataneously
void HistLabelSizes(TH1D *h, const double xlabel, const double xtitle, const double ylabel, const double ytitle){
    h->GetXaxis()->SetLabelSize(xlabel);
    h->GetXaxis()->SetTitleSize(xtitle);
    h->GetYaxis()->SetLabelSize(ylabel);
    h->GetYaxis()->SetTitleSize(ytitle);
}
//Order an array of histograms by yield
void yieldOrder(TH1D** hists){
    std::order(std::begin(hists), std::end(hists), [](const TH1D* h1, const TH1D* h2){ return h1->GetSumOfWeights() > h2->GetSumOfWeights(); } );
}

Color_t bkgColorEWK(const std::string& bkgName){
    if(bkgName == "non-prompt") return kAzure + 1;
    else if(bkgName == "WZ") return kOrange;
    else if(bkgName == "ZZ/H") return  kGreen + 1;
    else if(bkgName == "TT/T + X") return  kViolet-3;
    else if(bkgName == "triboson") return kRed + 1;
    else if(bkgName == "X + #gamma") return kOrange + 7;
    else if(bkgName == "T + X") return kCyan + 1;
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

Color_t bkgColor_general(){
    static const Color_t colors[8] = {kAzure + 1, kGreen - 7, kMagenta -7, kRed - 7, kBlue -3, kOrange + 6, kCyan + 1, kMagenta +3};
    static unsigned counter = 0;
    Color_t output = colors[counter];
    ++counter;
    if(counter == 8) counter = 0;
    return output;
}


Color_t bkgColor(const std::string& bkgName, const std::string& analysis){
    if(analysis == "ewkino" || analysis == "EWK" || analysis == "EWKino"){
        return bkgColorEWK(bkgName);
    } else if(analysis == "HNL"){
        return bkgColorHNL(bkgName);
    } else{
        return bkgColorGeneral();
    }
}

void plotDataVSMC(TH1D* data, TH1D** bkg, const std::string* names, const unsigned nBkg, const std::string& file, const std::string& analysis, const bool ylog, TH1D** bkgSyst){
    //set background histogram colors
    for(unsigned h = 0; h < nBkg; ++h){
        StackCol(bkg[h], bkgColor(names[h + 1], analysis) ); //first name is data
    }    

    //Set Poisonian errors to data
    data->SetBinErrorOption(TH1::kPoisson);

    //Replace data by TGRaphAsymmErrors for plotting
    TGraphAsymmErrors dataGraph = TGraphAsymmErrors(data);
    for(unsigned b = 1; b < data->GetNbinsX() + 1; ++b){
        dataGraph.SetPointError(b - 1, 0, 0, data->GetBinErrorLow(b), (data->GetBinContent(b) == 0 ) ? 0 : data->GetBinErrorUp(b) );
    }

    //Background histograms with full uncertainty
    TH1D* bkgE[nHist]; //clone histograms so that the function does not affect its arguments!
    std::copy(std::begin(bkg), std::end(bkg), std::begin(bkgE) ); //CHECK WHETHER STD COPY DOES INDEED COPY THE OBJECTS AND NOT JUST THE POINTERS
    for(unsigned h = 0; h < nHist; ++h){
        for(unsigned bin = 1; bin < bkgE[b]->GetNbinsX() + 1; ++bin){
            bkgE[b]->SetBinError(bin, sqrt(bkgE[h]->GetBinError(bin)*bkgE[h]->GetBinError(bin) + bkgSyst[h]->GetBinContent(bin)*bkgSyst[h]->GetBinContent(bin)) );
        }
    }

    //Compute total background
    TH1D* bkgTotE = (TH1D*) bkgE[0]->Clone(); //REPLACE WITH MEMORY SAFE CODE
    for(unsigned h = 1; h < nHist; ++h){
        bkgTotE->Add(bkgE[h]);
    }

    //Make the total background uncertainty visible as a grey band
    bkgTotE->SetFillStyle(3244); //3005  3244
    bkgTotE->SetFillColor(kGray+2);
    bkgTotE->SetMarkerStyle(0); //1

    //make legend and add all histograms
    TLegend legend = TLegend(0.2,0.8,0.95,0.9,NULL,"brNDC");
    legend.SetNColumns(4);
    legend.SetFillStyle(0); //avoid legend box
    legend.AddEntry(dataGraph, names[0], "pe1"); //add data to legend
    for(unsigned h = 0; h < nBkg; ++h){
        legend.AddEntry(bkgE[h], names[h + 1], "f"); //add backgrounds to the legend
    }
    legend.AddEntry(bkgTotE, "total bkg. unc.", "f"); //add total background uncertainty to legend
    
    //order background histograms by yield
    yieldOrder(bkg);

    //add background histograms to stack
    THStack bkgStack = THStack("bkgStack", "bkgStack");
    for(unsigned h = 0; h < nHist; ++h){
        bkgStack.Add(bkgE[h]);
    }
    
    //canvas dimenstions
    const double width = 600*(1 - xPad);
    const double height = 600;

    //make canvas to plot
    TCanvas c = TCanvas(file,"",width,height);
    c.cd();

    //make upper pad to draw main plot and lower pad for ratios
    TPad p1, p2;

    //prepare first pad for plotting data and background yields
    p1 = TPad(file,"",0,xPad,1,1);
    p1.Draw();
    p1.cd();
    p1.SetBottomMargin(0.03);

    //make pad logarithmic if needed
    if(ylog) p1.SetLogy();
    
    /*
    From now on we will determine the range of the plot from the total background histogram which will always be drawn first
    in order to force the range of the canvas.
    */

    //set minimum to zero
    if(!ylog) bkgTotE->SetMinimum(0);
    
    //x-axis labels will only be drawn in the lower (ratio) canvas
    bkgTotE->GetXaxis()->SetLabelSize(0);

    //Determine the maximum range of the histogram, depending on the maximum range of the bkg or data
    double dataMax = data->GetBinContent(data->GetMaximumBin()) + data->GetBinError(data->GetMaximumBin());
    double bkgMax = bkgTot->GetBinContent(bkgTot->GetMaximumBin()) + bkgTotE->GetBinError(bkgTot->GetMaximumBin());
    double totalMax = std::max(dataMax, bkgMax);
    if(plotsig && !signorm){
        double sigMax = 0;
        for(unsigned sig = 0; sig < nSig; ++sig){
            if(signal[sig]->GetSumOfWeights() == 0) continue;
            double localMax = signal[sig]->GetBinContent(signal[sig]->GetMaximumBin()) + signal[sig]->GetBinError(signal[sig]->GetMaximumBin());
            if( localMax > sigMax) sigMax = localMax;
        }
        totalMax = std::max(totalMax, sigMax);
    }
    //if(!ylog) data->SetMaximum(totalMax*1.3);
    //if(!plotsig && !ylog) data->SetMaximum(totalMax*1.3);
    //else if(!ylog) data->SetMaximum(totalMax*1.6);

    //Hack not to draw 0 points
    for(unsigned b = 1; b < data->GetNbinsX() + 1; ++b){
        if(obs->GetY()[b - 1] == 0)  obs->GetY()[b - 1] += totalMax*10;
    }
    if(!plotsig && !ylog) bkgTotE->SetMaximum(totalMax*1.3);
    else if(!ylog) bkgTotE->SetMaximum(totalMax*1.6);
    else if (!plotsig) {
        /*
        //NEW TEST CODE
        double minimum = totalMax;
        for(unsigned back = 0; back < nHist; ++back){
        for(unsigned b = 1; b < bkg[back]->GetNbinsX() + 1; ++b){
        if(bkg[back]->GetBinContent(b) != 0 &&  bkg[back]->GetBinContent(b) < minimum){
        minimum = bkg[back]->GetBinContent(b);
        }
        }
        }
        if(0.5*bkgTot->GetBinContent(bkgTot->GetMinimumBin()) > minimum*30) data->SetMinimum(minimum*30);
        else if(0.5*bkgTot->GetBinContent(bkgTot->GetMinimumBin()) < minimum*30) data->SetMinimum(0.5*bkgTot->GetBinContent(bkgTot->GetMinimumBin()) );
         */
        double minimum = totalMax;
        for(unsigned b = 1; b < bkgTot->GetNbinsX() + 1; ++b){
            if(bkgTot->GetBinContent(b) != 0 && bkgTot->GetBinContent(b) < minimum){
                minimum = bkgTot->GetBinContent(b);
            }
        }
        //data->SetMinimum(minimum/5.); //10.
        double SF = log10(std::max(10., totalMax/minimum));
        //data->SetMaximum(totalMax*3*SF); //3
        bkgTotE->SetMinimum(minimum/5.);
        bkgTotE->SetMaximum(totalMax*3*SF);
        //Hack not to draw 0 points
        for(unsigned b = 1; b < data->GetNbinsX() + 1; ++b){
            if(data->GetBinContent(b) == 0)  obs->GetY()[b - 1] += totalMax*30*SF;
        }
    }			
    else{
        /*
           double minimum = totalMax;
           for(unsigned back = 0; back < nHist; ++back){
           for(unsigned b = 1; b < bkg[back]->GetNbinsX() + 1; ++b){
           if(bkg[back]->GetBinContent(b) != 0 &&  bkg[back]->GetBinContent(b) < minimum){
           minimum = bkg[back]->GetBinContent(b);
           }
           }
           }
           if(0.5*bkgTot->GetBinContent(bkgTot->GetMinimumBin()) > minimum*30) data->SetMinimum(minimum*30);
           else if(0.5*bkgTot->GetBinContent(bkgTot->GetMinimumBin()) < minimum*30) data->SetMinimum(0.5*bkgTot->GetBinContent(bkgTot->GetMinimumBin()) );
        //double SF = log10(std::max(10., totalMax/bkgTot->GetBinContent(bkgTot->GetMinimumBin()) ));
        double SF = log10(std::max(10., totalMax/minimum));
        data->SetMaximum(totalMax*6*SF);
         */
        double minimum = totalMax;
        for(unsigned b = 1; b < bkgTot->GetNbinsX() + 1; ++b){
            if(bkgTot->GetBinContent(b) != 0 && bkgTot->GetBinContent(b) < minimum){
                minimum = bkgTot->GetBinContent(b);
            }
        }
        //data->SetMinimum(minimum/5.); //10.
        double SF = log10(std::max(10., totalMax/minimum ));
        //data->SetMaximum(totalMax*6*SF);
        bkgTotE->SetMinimum(minimum/5.);
        bkgTotE->SetMaximum(totalMax*6*SF);
        //Hack not to draw 0 points
        for(unsigned b = 1; b < data->GetNbinsX() + 1; ++b){
            if(data->GetBinContent(b) == 0)  obs->GetY()[b - 1] += totalMax*30*SF;
        }
    }

    //Draw histograms and legends
    //First draw data to fix plot range
    //data->Draw("pe");
    bkgTotE->Draw("e2"); //e2same
    bkgStack->Draw("hist same");
    //Redraw data so it is overlaid on the background stack
    //data->Draw("pe same");
    legend->Draw("same");
    bkgTotE->Draw("e2same"); //e2same
    //data->Draw("pe1 same"); // NEW  //pesame
    obs->Draw("pe1 same");	
    //Draw signal plots
    if(plotsig){
        for(unsigned sig = 0; sig < nSig; ++sig){
            if(signorm && signal[sig]->GetSumOfWeights() != 0) signal[sig]->Scale(bkgTot->GetSumOfWeights()/ signal[sig]->GetSumOfWeights());
            signal[sig]->Draw("histsame");
        }
    }
    //redraw axis over histograms
    gPad->RedrawAxis();
    //Draw CMS header
    drawLumi(p1);
    c->cd(); 
    //Make ratio plot in second pad

    const unsigned nBins = data->GetNbinsX();
    //TH1D* dataErrors = new TH1D("dataerrors" + file, "dataerrors" + file, nBins, data->GetBinLowEdge(1), data->GetBinLowEdge(data->GetNbinsX()) + data->GetBinWidth(data->GetNbinsX()));
    TH1D* bkgStatErrors = new TH1D("bkgStaterrors" + file, "bkgStaterrors" + file, nBins, data->GetBinLowEdge(1), data->GetBinLowEdge(data->GetNbinsX()) + data->GetBinWidth(data->GetNbinsX()));
    //TH1D* bkgStatErrors = new TH1D("bkgStaterros" + file, "bkgStaterrors" + file, nBins, data->GetBinLowEdge(1), data->GetBinLowEdge(data->GetNbinsX()) + data->GetBinWidth(data->GetNbinsX()));
    for(unsigned b = 1; b < nBins + 1; ++b){
        /*
           if(data->GetBinContent(b) != 0){
           if(bkgTot->GetBinContent(b) != 0) dataErrors->SetBinContent(b, data->GetBinError(b)/bkgTot->GetBinContent(b));
           else dataErrors->SetBinContent(b, data->GetBinError(b)/data->GetBinContent(b));
           } else{
           dataErrors->SetBinContent(b, 0);
           }
         */
        bkgStatErrors->SetBinContent(b, 1.);
        //bkgStatErrors->SetBinContent(b, 1.);
        if(bkgTot->GetBinContent(b) != 0){
            bkgStatErrors->SetBinError(b, bkgTot->GetBinError(b)/bkgTot->GetBinContent(b));
            //bkgStatErrors->SetBinError(b, sqrt( *bkgTot->GetBinContent(b) ));
        } else{
            bkgStatErrors->SetBinError(b, 0.);
            //bkgStatErrors->SetBinError(b, 0.);
        }			
    }
    TH1D* bkgErrors = (TH1D*) bkgTotE->Clone();//new TH1D("bkgerrors" + file, "bkgerrors" + file, nBins, data->GetBinLowEdge(1), data->GetBinLowEdge(data->GetNbinsX()) + data->GetBinWidth(data->GetNbinsX()));
    /*
       if(analysis == "HNL"){
       TString bkgNames[nHist];
       for(int b = nHist - 1; b > -1; --b){
       bkgNames[b] = names[histI[b] + 1];
       }
       hnl::setSystUnc(bkg, nHist - 1, bkgNames);
       bkgTot = (TH1D*) bkg[0]->Clone();
       for(int i = 1; i <  nHist; ++i){
       bkgTot->Add(bkg[i]);
       }
     */
    for(unsigned b = 1; b < nBins + 1; ++b){
        bkgErrors->SetBinContent(b, 1.);
        //bkgStatErrors->SetBinContent(b, 1.);
        if(bkgTot->GetBinContent(b) != 0){
            bkgErrors->SetBinError(b, bkgTotE->GetBinError(b)/bkgTotE->GetBinContent(b));
            //bkgStatErrors->SetBinError(b, sqrt( *bkgTot->GetBinContent(b) ));
        } else{
            bkgErrors->SetBinError(b, 0.);
            //bkgStatErrors->SetBinError(b, 0.);
        }
    }			
    //}

    bkgStatErrors->SetFillStyle(1001);
    bkgErrors->SetFillStyle(1001);
    bkgStatErrors->SetFillColor(kCyan  - 4); //
    bkgErrors->SetFillColor(kOrange	- 4); //kOrange  //kOrange + 7 kYellow - 3   kOrange - 4
    //bkgStatErrors->SetFillColor(kOrange); 
    bkgStatErrors->SetMarkerStyle(1);
    bkgErrors->SetMarkerStyle(1);
    //bkgStatErrors->SetMarkerStyle(1);

    p2 = new TPad(file + "2","",0,0.0,1,xPad);
    p2->Draw();
    p2->cd();
    //p2->SetTopMargin(0);
    p2->SetTopMargin(0.01);
    p2->SetBottomMargin(0.4);

    //TH1D* dataC = (TH1D*) data->Clone();
    //TH1D* bkgTotC = (TH1D*) bkgTot->Clone();

    // dataC->Divide(bkgTotE);

    TGraphAsymmErrors* obsRatio = new TGraphAsymmErrors(data);
    for(unsigned b = 1; b < data->GetNbinsX() + 1; ++b){
        obsRatio->GetY()[b - 1] *= 1./bkgTotE->GetBinContent(b);
        obsRatio->SetPointError(b - 1, 0, 0, data->GetBinErrorLow(b)/bkgTotE->GetBinContent(b), data->GetBinErrorUp(b)/bkgTotE->GetBinContent(b));
        if(data->GetBinContent(b) == 0) obsRatio->GetY()[b - 1] += 5;
    }

    //Now reset bin errors 
    //for(unsigned b = 1; b < nBins + 1; ++b){
    //dataC->SetBinError(b, dataErrors->GetBinContent(b));
    //}
    //Legend for uncertainties
    TLegend* legend2 = new TLegend(0.18,0.85,0.8,0.98,NULL,"brNDC"); //0.18, 0.85, 0.65, 0.98
    legend2-> SetNColumns(3); //2
    //Avoid legend box
    legend2->SetFillStyle(0);
    //Add data to legend
    legend2->AddEntry(bkgStatErrors, "stat. pred. unc.", "f");
    legend2->AddEntry(bkgErrors, "total pred. unc.", "f");
    //legend2->AddEntry(dataC, "obs./pred. with total unc.", "pe12");
    legend2->AddEntry(obsRatio, "obs./pred.", "pe12");


    //legend2->AddEntry(bkgStatErrors, "stat. bkg. unc.", "f");
    /*
       dataC->SetMarkerColor(1);
       dataC->SetLineColor(1);
       dataC->GetYaxis()->SetRangeUser(0.,1.999);
       dataC->GetYaxis()->SetTitle("obs./pred.");
       dataC->GetYaxis()->SetTitleOffset(1.25/((1.-xPad)/xPad));
       dataC->GetYaxis()->SetTitleSize((1.-xPad)/xPad*0.06); //originally 0.06
       dataC->GetXaxis()->SetTitleSize((1.-xPad)/xPad*0.06); //originally 0.09
       dataC->GetYaxis()->SetLabelSize((1.-xPad)/xPad*0.05); //originally 0.05
       dataC->GetXaxis()->SetLabelSize((1.-xPad)/xPad*0.05); //originally 0.05
     */


    bkgErrors->SetMarkerColor(1);
    bkgErrors->SetLineColor(1);
    bkgErrors->GetYaxis()->SetRangeUser(0.,1.999);
    bkgErrors->GetYaxis()->SetTitle("obs./pred.");
    bkgErrors->GetYaxis()->SetTitleOffset(1.25/((1.-xPad)/xPad)); //1.25
    bkgErrors->GetYaxis()->SetTitleSize((1.-xPad)/xPad*0.06); //originally 0.06
    bkgErrors->GetXaxis()->SetTitleSize((1.-xPad)/xPad*0.06); //originally 0.09
    bkgErrors->GetYaxis()->SetLabelSize((1.-xPad)/xPad*0.05); //originally 0.05
    bkgErrors->GetXaxis()->SetLabelSize((1.-xPad)/xPad*0.05); //originally 0.05
    //NeW 
    bkgErrors->GetXaxis()->SetLabelOffset((1-xPad)/xPad*0.009);
    bkgErrors->GetXaxis()->SetTitleOffset((1-xPad)/xPad*0.32);

    //dataC->Draw("pe");
    bkgErrors->Draw("e2"); //e2same
    //draw bkg errors
    bkgErrors->Draw("e2same"); //e2same
    bkgStatErrors->Draw("e2same");
    //bkgStatErrors->Draw("e2same");
    //dataC->Draw("pe1same"); //esame
    obsRatio->Draw("pe1same");
    legend2->Draw("same");
    gPad->RedrawAxis();
    //Draw line at 1 on ratio plot
    //double xmax = dataC->GetBinCenter(dataC->GetNbinsX()) + dataC->GetBinWidth(dataC->GetNbinsX())/2;
    //double xmin = dataC->GetBinCenter(0) + dataC->GetBinWidth(0)/2;
    double xmax = data->GetBinCenter(data->GetNbinsX()) + data->GetBinWidth(data->GetNbinsX())/2;
    double xmin = data->GetBinCenter(0) + data->GetBinWidth(0)/2;
    TLine *line = new TLine(xmin, 1, xmax, 1);
    line->SetLineStyle(2);
    line->Draw("same");
    c->SaveAs("plots/" + file + ".pdf");
    c->SaveAs("plots/" + file + ".png");
}
