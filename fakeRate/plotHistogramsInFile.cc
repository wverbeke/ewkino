/*
executable to call already existing plotcode in C++ from a Python script... #
*/

//include c++ library classes 
#include <stdexcept>
#include <iostream>
#include <set>
#include <algorithm>

//include ROOT classes 
#include "TH1.h"
#include "TH1F.h"
#include "TH1D.h"
#include "TROOT.h"

//include other parts of framework
#include "../Tools/interface/stringTools.h"
#include "../Tools/interface/systemTools.h"
#include "../Tools/interface/analysisTools.h"
#include "../plotting/plotCode.h"
#include "interface/fakeRateTools.h"


std::shared_ptr<TH1D> convertHistType( TH1F* histf ){
    // under testing...
    // seems to work by itself but not in function below...
    // keep for reference but it is not used in this file

    TH1F histfref = *histf;
    TH1D histdref;
    histfref.Copy(histdref);
    std::shared_ptr<TH1D> histd = std::shared_ptr<TH1D>( dynamic_cast<TH1D*>(histdref.Clone() ) );
    return histd;
}

void plotHistogramsInFile( const std::string& histFileName, 
			    const std::string& outFigName, 
			    const std::string& colorScheme ){

    // get all histograms in the file
    TFile* filePtr = TFile::Open( histFileName.c_str() );
    std::vector< std::string > histogramNames = fakeRate::listHistogramNamesInFile( filePtr );

    // get data histogram and array of simulated histograms to plot
    // (assuming the data histogram contains "data" in its name 
    // and all other histograms in histogramNames are MC)
    std::shared_ptr< TH1D > dataHist;
    TH1D* predictedHists[ histogramNames.size()-1 ];
    std::string predictedLabels[ histogramNames.size() ];
    predictedLabels[0] = std::string("data");
    int counter = 0;
    for( const auto& name : histogramNames ){
	if( !dynamic_cast< TH1D* >( filePtr->Get( name.c_str() ) ) ){
	    std::cerr << "### WARNING ###: histogram " << name << " in file " << histFileName;
	    std::cerr << " is not of type TH1D*, which is required for plotting." << std::endl;
	    std::cerr << "Will continue without making plots." << std::endl;
	}
	TH1D* thishist = dynamic_cast< TH1D* >( filePtr->Get( name.c_str() ) );
	if( stringTools::stringContains(name,"data") ){
	    dataHist = std::shared_ptr<TH1D>(thishist);
	} else{
	    predictedHists[counter] = thishist;
	    predictedLabels[counter+1] = thishist->GetTitle();
	    counter++;
        }
    }

    // make plot
    plotDataVSMC( dataHist.get(), predictedHists, predictedLabels, histogramNames.size()-1,
		    outFigName, colorScheme, false, false, "(13 TeV)" );
    //filePtr->Close(); // seems to give segmentation violation
}

void plot2DHistogramInFile( const std::string& histFileName, const std::string& outFigName ){
    
    // get all histograms in the file
    TFile* filePtr = TFile::Open( histFileName.c_str() );
    std::vector< std::string > histogramNames = fakeRate::listHistogramNamesInFile( filePtr );
    
    // expect only one histogram so return if more
    if( histogramNames.size() > 1){ 
	std::cerr << "### WARNING ###: number of histograms in "<<histFileName;
	std::cerr << " is larger than 1, which is unexpected for plot2DHistogramInFile;";
	std::cerr << " returning without plotting." << std::endl;
	return;
    }

    std::shared_ptr< TH2D > frMap = std::shared_ptr<TH2D>( dynamic_cast<TH2D*>( 
				    filePtr->Get(histogramNames[0].c_str()) ) );

    plot2DHistogram( frMap.get(), outFigName.c_str() );
    //filePtr->Close(); // seems to give segmentation violation
}

int main( int argc, char* argv[] ){

   std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    if( !( argvStr.size() == 4 || argvStr.size() == 5) ){
        std::cerr<<"### ERROR ###: wrong number of command line arguments"<<std::endl;
        std::cerr<<"usage: ./plotTemplates histFileName outFigName type [colorscheme]"<<std::endl;
        return 1;
    }
    std::string histFileName = argvStr[1];
    std::string outFigName = argvStr[2];
    std::string type = argvStr[3];
    std::string colorScheme = "";
    if( argvStr.size() == 5) colorScheme = argvStr[4];

    if( type=="mcvsdata" ){
	plotHistogramsInFile(histFileName,outFigName,colorScheme);
    } else if( type=="2dmap" ){
	plot2DHistogramInFile(histFileName,outFigName);
    } else{
	std::cerr << "### WARNING ###: type " << type << "not recognized." << std::endl;
	std::cerr << "(should be 'mcvsdata' or '2dmap')" << std::endl;
	return 1;
    }

    return 0;
}

