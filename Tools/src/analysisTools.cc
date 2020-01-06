#include "../interface/analysisTools.h"

//Include c++ library classes
#include <iostream>
#include <string>
#include <fstream>

//include other parts of framework
#include "../interface/stringTools.h"


void analysisTools::printProgress( double progress ){
    const unsigned barWidth = 100;
    std::cout << "[";
    unsigned pos = barWidth * progress;
    for (unsigned i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << unsigned(progress * 100.0) << " %\r" << std::flush;
}


void analysisTools::setNegativeBinsToZero( TH1D* h ){

	//check each bin of the histogram, and set its binContent to 0 if it is negative
    for(int b = 1; b < h->GetNbinsX() + 1; ++b){
        if(h->GetBinContent(b) < 0.) h->SetBinContent(b, 0.);
    }
}


void analysisTools::setNegativeBinsToZero( const std::shared_ptr< TH1D >& h){
	analysisTools::setNegativeBinsToZero( h.get() );
}


//Function to print dataCard to be analysed by CMS combination tool
void analysisTools::printDataCard(const std::string& cardName, const double obsYield, const double sigYield, const std::string& sigName, const double* bkgYield, const unsigned nBkg, const std::string* bkgNames, const std::vector<std::vector<double> >& systUnc, const unsigned nSyst, const std::string* systNames, const std::string* systDist, const bool shapeCard, const std::string& shapeFileName, const bool autoMCStats ){

	//stream for writing card
    std::ofstream card;

    //add .txt to name if no file extension is given
    card.open(cardName + ((cardName.find(".") == std::string::npos) ? ".txt" : "") ); //add .txt to name if no file extension is given

    //define number of channels, background sources and systematics
    card << "imax 1 number of channels \n";
    card << "jmax " << nBkg << " number of backgrounds \n";
    card << "kmax " << nSyst << " number of nuisance parameters (sources of systematical uncertainties) \n";
    card << "---------------------------------------------------------------------------------------- \n";

    //define the channels and the number of observed events
    card << "bin bin1 \n";
    card << "observation " << obsYield << "\n";

    //define all backgrounds and their yields
    card << "---------------------------------------------------------------------------------------- \n";
    if(shapeCard){
        card << "shapes * * " << shapeFileName + ".root  $PROCESS $PROCESS_$SYSTEMATIC\n";
        card << "---------------------------------------------------------------------------------------- \n";
    }
    card << "bin	";
    for(unsigned proc = 0; proc < nBkg + 1; ++proc){
        card << "	" << "bin1";
    }
    card << "\n";
    card << "process";
    card << "	" << sigName;
    for(unsigned bkg = 0; bkg < nBkg; ++bkg){
        card << "	" << bkgNames[bkg];
    }
    card << "\n";
    card << "process";
    for(unsigned bkg = 0; bkg < nBkg + 1; ++bkg){
        card << "	" << bkg;
    }
    card << "\n";
    card <<	"rate";
    card << "	" << sigYield;
    for(unsigned bkg = 0; bkg < nBkg; ++bkg){
        if(bkgYield[bkg] <= 0) card << "	" << "0.00";
        else card << "	" << bkgYield[bkg];
    }
    card << "\n";
    card << "---------------------------------------------------------------------------------------- \n";

    //define sources of systematic uncertainty, what distibution they follow and how large their effect is
    if( nSyst != 0 ){
        for(unsigned syst = 0; syst < nSyst; ++syst){
            card << systNames[syst] << "	" << systDist[syst];
            for(unsigned proc = 0; proc < nBkg + 1; ++proc){
                card << "	";
                if(systUnc[syst][proc] == 0) card << "-";
                else card << systUnc[syst][proc];
            }
            card << "\n";
        }
    }

    //add line to automatically include statistical uncertainties from the MC shape histograms 
    if( autoMCStats ){
        card << "* autoMCStats 0\n";
    }
    
    card.close();		
}


//check what year a sample file corresponds to
bool analysisTools::fileIs2017( const std::string& filePath ){
    return ( stringTools::stringContains( filePath, "MiniAOD2017" ) 
        || stringTools::stringContains( filePath, "Run2017" ) 
        || stringTools::stringContains( filePath, "Fall17" )
        || stringTools::stringEndsWith( filePath, "data_2017.root" )
        || stringTools::stringEndsWith( filePath, "combined_2017.root" )
        || stringTools::stringEndsWith( filePath, "combined_2018.root" )
        || stringTools::stringContains( filePath, "_2017_v" )
        || stringTools::stringStartsWith( stringTools::fileNameFromPath( filePath ), "2017" )
    );
}


bool analysisTools::fileIs2018( const std::string& filePath ){
    return ( stringTools::stringContains( filePath, "MiniAOD2018" ) 
        || stringTools::stringContains( filePath, "Run2018" ) 
        || stringTools::stringContains( filePath, "Autumn18" )
        || stringTools::stringEndsWith( filePath, "data_2018.root" )
        || stringTools::stringEndsWith( filePath, "combined_2018.root" )
        || stringTools::stringContains( filePath, "_2018_v" )
        || stringTools::stringStartsWith( stringTools::fileNameFromPath( filePath ), "2018" )
    );
}


bool analysisTools::fileIs2016( const std::string& filePath ){
    return !( analysisTools::fileIs2017( filePath ) || analysisTools::fileIs2018( filePath ) );
}


std::pair< bool, bool > analysisTools::fileIs2017Or2018( const std::string& filePath ){
    bool is2017 = fileIs2017( filePath );
    bool is2018 = fileIs2018( filePath );

    //check consistency
    if( is2017 && is2018 ){
        throw std::invalid_argument( "File '" + filePath + "' is flagged as both 2017 and 2018, and should only be one of the two." );
    }
    return { is2017, is2018 };
}


void analysisTools::checkYearString( const std::string& yearString ){
    if( !( yearString == "2016" || yearString == "2017" || yearString == "2018" ) ){
        throw std::invalid_argument( "Year string is '" + yearString + "' while it must be either '2016', '2017' or '2018'" );
    }
}

