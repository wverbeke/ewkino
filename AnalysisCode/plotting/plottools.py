######################################################################################
# grouping some common functions for plotting too long to be included in main script #
######################################################################################

import ROOT
import array

def setTDRstyle():
    # copy from ewkino/plotting/tdrStyle.cc
    # note: some of the options here will be overwritten during the actual plotting.
    # note: most of the commented lines will not be tested and will probably need modification 
    # (from C++ to python).

    # For the canvas:
    ROOT.gStyle.SetCanvasBorderMode(0);
    ROOT.gStyle.SetCanvasColor(ROOT.kWhite);
    ROOT.gStyle.SetCanvasDefH(600); # height of canvas
    ROOT.gStyle.SetCanvasDefW(600); # width of canvas
    ROOT.gStyle.SetCanvasDefX(0); # position on screen
    ROOT.gStyle.SetCanvasDefY(0);

    # For the Pad:
    ROOT.gStyle.SetPadBorderMode(0);
    #ROOT.gStyle.SetPadBorderSize(Width_t size = 1);
    ROOT.gStyle.SetPadColor(ROOT.kWhite);
    ROOT.gStyle.SetPadGridX(ROOT.kFALSE);
    ROOT.gStyle.SetPadGridY(ROOT.kFALSE);
    ROOT.gStyle.SetGridColor(0);
    ROOT.gStyle.SetGridStyle(3);
    ROOT.gStyle.SetGridWidth(1);

    # For the frame:
    ROOT.gStyle.SetFrameBorderMode(0);
    ROOT.gStyle.SetFrameBorderSize(1);
    ROOT.gStyle.SetFrameFillColor(0);
    ROOT.gStyle.SetFrameFillStyle(0);
    ROOT.gStyle.SetFrameLineColor(1);
    ROOT.gStyle.SetFrameLineStyle(1);
    ROOT.gStyle.SetFrameLineWidth(1);

    # For the histo (probably overwritten for each histogram?):
    #ROOT.gStyle.SetHistFillColor(1);
    #ROOT.gStyle.SetHistFillStyle(0);
    ROOT.gStyle.SetHistLineColor(1);
    ROOT.gStyle.SetHistLineStyle(0);
    ROOT.gStyle.SetHistLineWidth(1);
    #ROOT.gStyle.SetLegoInnerR(Float_t rad = 0.5);
    #ROOT.gStyle.SetNumberContours(Int_t number = 20);

    ROOT.gStyle.SetEndErrorSize(2);
    #ROOT.gStyle.SetErrorMarker(20);
    #ROOT.gStyle.SetErrorX(0.);

    ROOT.gStyle.SetMarkerStyle(20);

    # For the fit/function:
    ROOT.gStyle.SetOptFit(1);
    ROOT.gStyle.SetFitFormat("5.4g");
    ROOT.gStyle.SetFuncColor(2);
    ROOT.gStyle.SetFuncStyle(1);
    ROOT.gStyle.SetFuncWidth(1);

    # For the date:
    ROOT.gStyle.SetOptDate(0);
    #ROOT.gStyle.SetDateX(Float_t x = 0.01);
    #ROOT.gStyle.SetDateY(Float_t y = 0.01);

    # For the statistics box:
    ROOT.gStyle.SetOptFile(0);
    ROOT.gStyle.SetOptStat(0); # to display the mean and RMS: SetOptStat("mr");
    ROOT.gStyle.SetStatColor(ROOT.kWhite);
    ROOT.gStyle.SetStatFont(42);
    ROOT.gStyle.SetStatFontSize(0.08); #0.025
    ROOT.gStyle.SetStatTextColor(1);
    ROOT.gStyle.SetStatFormat("6.4g");
    ROOT.gStyle.SetStatBorderSize(1);
    ROOT.gStyle.SetStatH(0.7); #0.1
    ROOT.gStyle.SetStatW(0.15); #0.15
    #ROOT.gStyle.SetStatTextSize(2.5);

    ROOT.gStyle.SetStatX(0.96);
    #ROOT.gStyle.SetStatY(0.35);
    #ROOT.gStyle.SetStatStyle(Style_t style = 1001);
    #ROOT.gStyle.SetStatX(Float_t x = 0);
    #ROOT.gStyle.SetStatY(Float_t y = 0);

    # Margins:
    ROOT.gStyle.SetPadTopMargin(0.05);
    ROOT.gStyle.SetPadBottomMargin(0.13);
    ROOT.gStyle.SetPadLeftMargin(0.14); #0.14
    ROOT.gStyle.SetPadRightMargin(0.04);

    # For the Global title:
    ROOT.gStyle.SetOptTitle(0);
    ROOT.gStyle.SetTitleFont(42);
    ROOT.gStyle.SetTitleColor(1);
    ROOT.gStyle.SetTitleTextColor(1);
    ROOT.gStyle.SetTitleFillColor(10);
    ROOT.gStyle.SetTitleFontSize(0.05);
    #ROOT.gStyle.SetTitleH(0); # set the height of the title box
    #ROOT.gStyle.SetTitleW(0); # set the width of the title box
    #ROOT.gStyle.SetTitleX(0); # set the position of the title box
    #ROOT.gStyle.SetTitleY(0.985); # set the position of the title box
    #ROOT.gStyle.SetTitleStyle(Style_t style = 1001);
    #ROOT.gStyle.SetTitleBorderSize(2);

    # For the axis titles:
    ROOT.gStyle.SetTitleColor(1, "XYZ");
    ROOT.gStyle.SetTitleFont(42, "XYZ");
    ROOT.gStyle.SetTitleSize(0.06, "XYZ");
    #ROOT.gStyle.SetTitleXSize(Float_t size = 0.02); # another way to set the size?
    #ROOT.gStyle.SetTitleYSize(Float_t size = 0.02);
    ROOT.gStyle.SetTitleXOffset(0.9);
    #ROOT.gStyle.SetTitleYOffset(1.25);
    ROOT.gStyle.SetTitleYOffset(1.25); #0.9
    #ROOT.gStyle.SetTitleOffset(1.1, "Y"); # another way to set the Offset

    # For the axis labels:
    ROOT.gStyle.SetLabelColor(1, "XYZ");
    ROOT.gStyle.SetLabelFont(42, "XYZ");
    ROOT.gStyle.SetLabelOffset(0.007, "XYZ");
    #ROOT.gStyle.SetLabelSize(0.1, "XYZ");
    ROOT.gStyle.SetLabelSize(0.05, "XYZ");

    # For the axis:
    ROOT.gStyle.SetAxisColor(1, "XYZ");
    ROOT.gStyle.SetStripDecimals(ROOT.kTRUE);
    ROOT.gStyle.SetTickLength(0.03, "XYZ");
    ROOT.gStyle.SetNdivisions(510, "XYZ");
    ROOT.gStyle.SetPadTickX(1);  # to get tick marks on the opposite side of the frame
    ROOT.gStyle.SetPadTickY(1);

    # Change for log plots:
    ROOT.gStyle.SetOptLogx(0);
    ROOT.gStyle.SetOptLogy(0);
    ROOT.gStyle.SetOptLogz(0);

    # Postscript options:
    ROOT.gStyle.SetPaperSize(20.,20.);
    #ROOT.gStyle.SetLineScalePS(Float_t scale = 3);
    #ROOT.gStyle.SetLineStyleString(Int_t i, const char* text);
    #ROOT.gStyle.SetHeaderPS(const char* header);
    #ROOT.gStyle.SetTitlePS(const char* pstitle);

    #ROOT.gStyle.SetBarOffset(Float_t baroff = 0.5);
    #ROOT.gStyle.SetBarWidth(Float_t barwidth = 0.5);
    #ROOT.gStyle.SetPaintTextFormat(const char* format = "g");
    #ROOT.gStyle.SetPalette(Int_t ncolors = 0, Int_t* colors = 0);
    #ROOT.gStyle.SetTimeOffset(Double_t toffset);
    #ROOT.gStyle.SetHistMinimumZero(kTRUE);

    # For error in SF maps
    #ROOT.gStyle.SetMarkerSize(0.9); # TEMPORARY SIZE FOR DILEPTON PLOTS. SET BACK TO DEFAULT FOR TRILEPTON
    ROOT.gStyle.SetPaintTextFormat("4.2f"); #4.2
    #ROOT.gStyle.SetHatchesLineWidth(5);
    #ROOT.gStyle.SetHatchesSpacing(0.05);

    ROOT.gStyle.SetLegendBorderSize(0);

############################################################

def drawLumi(pad, extratext="Preliminary", 
		lumitext="<lumi> fb^{-1} (13 TeV)",
		cms_in_grid=True):

    pad.Update()

    # get margins for pad
    lmargin = pad.GetLeftMargin()+0.03;
    tmargin = pad.GetTopMargin();
    rmargin = pad.GetRightMargin();

    # define text sizes and offsets relative to margin
    CMSTextSize = tmargin*0.8;
    lumiTextSize = tmargin*0.8;

    CMSTextVerticalOffset = tmargin*0.2;
    lumiTextVerticalOffset = tmargin*0.2;
    
    pad.cd();
    # define latex text to draw on plot
    #latex = ROOT.TLatex(lmargin,1+lumiTextVerticalOffset*tmargin,"CMS");
    latex = ROOT.TLatex()
    latex.SetNDC();
    latex.SetTextAngle(0);
    latex.SetTextColor(ROOT.kBlack); 

    latex.SetTextFont(61);
    latex.SetTextAlign(11); 
    latex.SetTextSize(CMSTextSize);
    latex.SetText(0,0,'CMS')
    #cmsX = latex.GetXsize(); # does not seem to work properly
    cmsX = CMSTextSize*2 # phenomenological alternative
    # old style: cms label outside of frame:
    if not cms_in_grid: latex.DrawLatex(lmargin, 1 - tmargin + CMSTextVerticalOffset, 'CMS');
    # new style: cms label inside frame:
    else: latex.DrawLatex( lmargin, 1-tmargin-CMSTextVerticalOffset-CMSTextSize, 'CMS')

    extraTextSize = CMSTextSize*0.8
    latex.SetTextFont(52);
    latex.SetTextSize(extraTextSize);
    latex.SetTextAlign(11);
    if not cms_in_grid: latex.DrawLatex(lmargin + 1.2*cmsX, 
                        1-tmargin+CMSTextVerticalOffset, extratext);
    else: latex.DrawLatex(lmargin + 1.2*cmsX, 
	    1-tmargin-CMSTextVerticalOffset-CMSTextSize, extratext);

    latex.SetTextFont(42);
    latex.SetTextAlign(31);
    latex.SetTextSize(lumiTextSize);  
    latex.DrawLatex(1-rmargin,1-tmargin+lumiTextVerticalOffset,lumitext);
    return;

###########################################################
# analysis-specific settings for colors and labels
###########################################################

def getcolormap(style):

    style=style.lower()
    if(style=='tzq' or style=='tzqanalysis'): return getcolormap_tzq()
    if(style=='systematic' or style=='systematics'): return getcolormap_systematics()
    else:
	print('### WARNING ### (in getcolormap): style not recognized, returning None')
	return None

def defineColorHex(hexstring):
    # note: the color object needs to stay in scope, else the index will refer to a nullpointer
    # and the color will turn out white...
    # however, this is solved by adding the color as a global ROOT feature
    # (not very clean, better to work with a custom color class, but good enough for now)
    r, g, b = tuple(int(hexstring[i:i+2], 16) for i in (1, 3, 5))
    cindex = ROOT.TColor.GetFreeColorIndex()
    color = ROOT.TColor(cindex, r/255., g/255., b/255.)
    setattr(ROOT,'temp_color_'+str(cindex),color)
    return cindex,color

def getcolormap_tzq():
    # map of histogram titles to colors
    cmap = {}
    # old:
    '''cmap['tZq'] = ROOT.kRed-7
    cmap['nonprompt'] = ROOT.kOrange
    cmap['WZ'] = ROOT.kCyan-7
    cmap['multiboson'] = ROOT.kYellow+1
    cmap['t#bar{t}/t + X'] = ROOT.kBlue-10 
    cmap['tX'] = ROOT.kBlue-10 # new histtitle convention
    cmap['t#bar{t} + Z'] = ROOT.kBlue-6
    cmap['ttZ'] = ROOT.kBlue-6 # new histtitle convention
    cmap['ZZ/H'] = ROOT.kTeal-5
    cmap['ZZH'] = ROOT.kTeal-5 # new histtitle convention
    cmap['X + #gamma'] = ROOT.kMagenta-7
    cmap['Xgamma'] = ROOT.kMagenta-7 # new histtitle convention'''
    # new:
    cmap['tZq'] = defineColorHex('#ff0000')[0]
    cmap['tZq_top'] = defineColorHex('#ff0000')[0]
    cmap['tZq_antitop'] = defineColorHex('#ff6500')[0]
    cmap['tZq_mmm'] = defineColorHex('#ff0000')[0]
    cmap['tZq_emm'] = defineColorHex('#ff5000')[0]
    cmap['tZq_eem'] = defineColorHex('#ff6500')[0]
    cmap['tZq_eee'] = defineColorHex('#ffb700')[0]
    cmap['nonprompt'] = defineColorHex('#ffe380')[0]
    cmap['WZ'] = defineColorHex('#81efd7')[0]
    cmap['multiboson'] = defineColorHex('#c6ff00')[0]
    cmap['t#bar{t}/t+X'] = defineColorHex('#d6a2e8')[0]
    cmap['tbarttX'] = cmap['t#bar{t}/t+X'] # yet another histtitle convention
    cmap['tX'] = cmap['t#bar{t}/t+X']# new histtitle convention
    cmap['t#bar{t}+Z'] = defineColorHex('#336fce')[0]
    cmap['tbartZ'] = cmap['t#bar{t}+Z'] # yet another histtitle convention
    cmap['ttZ'] = cmap['t#bar{t}+Z'] # new histtitle convention
    cmap['ZZ/H'] = defineColorHex('#2fbc6c')[0]
    cmap['ZZH'] = cmap['ZZ/H'] # new histtitle convention
    cmap['X+#gamma'] = defineColorHex('#9c88ff')[0]
    cmap['Xgamma'] = cmap['X+#gamma'] # new histtitle convention
    cmap['Xgamma_int'] = defineColorHex('#b288ff')[0] # split ZG
    cmap['Xgamma_ext'] = defineColorHex('#8888ff')[0] # split ZG
    return cmap

def getcolormap_systematics():
    cmap = {}
    # nominal in black
    cmap['nominal'] = ROOT.kBlack
    # acceptance uncertainties in shades of red
    cmap['JEC'] = ROOT.kRed
    cmap['JER'] = ROOT.kRed-4
    cmap['JER_2016'] = ROOT.kRed-4
    cmap['JER_2017'] = ROOT.kRed-4
    cmap['JER_2018'] = ROOT.kRed-4
    cmap['Uncl'] = ROOT.kRed-9
    cmap['JECSqSumAll'] = ROOT.kYellow+1
    cmap['JECSqSumGrouped'] = ROOT.kYellow-7
    cmap['JECAll'] = ROOT.kGray
    cmap['JECGrouped'] = ROOT.kGray

    # lepton uncertainties in shades of blue
    cmap['muonID'] = ROOT.kBlue
    cmap['muonIDSyst'] = ROOT.kBlue
    cmap['muonIDStat'] = ROOT.kBlue+2
    cmap['electronID'] = ROOT.kBlue-9
    cmap['electronIDSyst'] = ROOT.kBlue-9
    cmap['electronIDStat'] = ROOT.kBlue-10
    cmap['electronReco'] = ROOT.kBlue-5

    # other weights in shades of green
    cmap['pileup'] = ROOT.kGreen-6
    cmap['bTag_heavy'] = ROOT.kGreen+1
    cmap['bTag_light'] = ROOT.kGreen+3
    cmap['prefire'] = ROOT.kGreen-9

    # btag weights also in green
    cmap['bTag_shape_lf'] = ROOT.kGreen+1
    cmap['bTag_shape_lfstats1'] = ROOT.kGreen+1
    cmap['bTag_shape_lfstats2'] = ROOT.kGreen+1
    cmap['bTag_shape_hf'] = ROOT.kGreen+3
    cmap['bTag_shape_hfstats1'] = ROOT.kGreen+3
    cmap['bTag_shape_hfstats2'] = ROOT.kGreen+3
    cmap['bTag_shape_cferr1'] = ROOT.kSpring+6
    cmap['bTag_shape_cferr2'] = ROOT.kGreen+6

    # scales in shaded of purple
    # first three are obsolete and replaced by qcdScalesShapeEnv and qcdScalesNorm
    # last two are obsolete and replaced by isrShape and isrNorm
    cmap['fScale'] = ROOT.kMagenta
    cmap['rScale'] = ROOT.kMagenta+2
    cmap['rfScales'] = ROOT.kMagenta-9
    cmap['isrScale'] = ROOT.kViolet+1
    cmap['fsrScale'] = ROOT.kViolet+2

    # isr/fsr in shades of violet
    cmap['isrShape'] = ROOT.kViolet+1
    cmap['isrNorm'] = ROOT.kViolet+1
    cmap['fsrShape'] = ROOT.kViolet+2
    cmap['fsrNorm'] = ROOT.kViolet+2

    # qcd scale variations in magenta
    cmap['qcdScalesShapeVar'] = ROOT.kGray
    cmap['qcdScalesShapeEnv'] = ROOT.kMagenta-9
    cmap['qcdScalesNorm'] = ROOT.kViolet+6

    # pdf variations in yellow
    cmap['pdfShapeVar'] = ROOT.kGray
    cmap['pdfShapeEnv'] = ROOT.kOrange-3
    cmap['pdfShapeRMS'] = ROOT.kOrange+7
    cmap['pdfNorm'] = ROOT.kOrange-2

    # underlyiing event and color reconnection in blue
    cmap['CR_QCD'] = ROOT.kCyan+1
    cmap['CR_GluonMove'] = ROOT.kCyan+3
    cmap['UE'] = ROOT.kAzure+7

    return cmap

def getlabelmap(style):
    
    style=style.lower()
    if(style=='tzq' or style=='tzqanalysis'): return getlabelmap_tzq()
    else:
        print('### WARNING ### (in getcolormap): style not recognized, returning None')
        return None

def getlabelmap_tzq():
    # map of histogram titles to legend labels
    lmap = {}
    lmap['tZq'] = 'tZq'
    lmap['tZq_top'] = 'tZq (t)'
    lmap['tZq_antitop'] = 'tZq (#bar{t})'
    lmap['tZq_mmm'] = 'tZq (mmm)'
    lmap['tZq_emm'] = 'tZq (emm)'
    lmap['tZq_eem'] = 'tZq (eem)'
    lmap['tZq_eee'] = 'tZq (eee)'
    lmap['nonprompt'] = 'Nonprompt'
    lmap['WZ'] = 'WZ'
    lmap['multiboson'] = 'Multiboson'
    lmap['t#bar{t}/t+X'] = 't(#bar{t})X'
    lmap['tbarttX'] = lmap['t#bar{t}/t+X']
    lmap['tX'] = lmap['t#bar{t}/t+X'] # new histtitle convention 
    lmap['t#bar{t}+Z'] = 't#bar{t}Z'
    lmap['tbartZ'] = lmap['t#bar{t}+Z']
    lmap['ttZ'] = lmap['t#bar{t}+Z'] # new histtitle convention
    lmap['ZZ/H'] = 'ZZ/H'
    lmap['ZZH'] = 'ZZ/H' # new histtitle convention
    lmap['X+#gamma'] = 'X#gamma'
    lmap['Xgamma'] = 'X#gamma' # new histtitle convention
    lmap['Xgamma_int'] = 'X#gamma (int.)' # split ZG
    lmap['Xgamma_ext'] = 'X#gamma (ext.)' # split ZG
    return lmap
