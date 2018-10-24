#include "../interface/PsUncertaintyReader.h"


//include c++ library classes 
#include <fstream>
#include <iostream>
#include <sstream>

//include other parts of code 
#include "../interface/systemTools.h"


PsUncertaintyReader::PsUncertaintyReader( const std::string& fileName ){
    
    if( ! systemTools::fileExists( fileName ) ){
        std::cerr << "Error in PsUncertaintyReader::PsUncertaintyReader, given input file does not exist! Returning control." << std::endl;
        return;
    } else if( systemTools::numberOfColumnsInFile( fileName ) != 7){
        std::cerr << "Error in PsUncertaintyReader::PsUncertaintyReader, given input file does not have 7 columns as expected. Returning control." << std::endl;
        return;
    }

    std::ifstream inputFile( fileName );
    std::string line;
    while( std::getline( inputFile, line ) ){
        std::istringstream stringStream(line);

        std::string process, category;
        unsigned bin;
        double uncertainty[4];

        stringStream >> process >> category >> bin;
        for(unsigned unc = 0; unc < 4; ++unc){
            stringStream >> uncertainty[unc];
            uncertaintyMaps[unc][process][category][bin] = uncertainty[unc];
        }
    }
}

double PsUncertaintyReader::getMapEntry( const std::map< std::string, std::map < std::string, std::map < unsigned, double > > >& uncMaps, const std::string& process, const std::string& category, const unsigned bin) const{
    auto processIt = uncMaps.find( process );
    if( processIt != uncMaps.end() ){
        auto categoryIt = (processIt->second).find( category );
        if( categoryIt != (processIt->second).end() ){
            auto binIt = (categoryIt->second).find( bin );
            if( binIt != (categoryIt->second).end() ){
                return binIt->second;
            } else{
                std::cerr << "Error in PsUncertaintyReader::getMapEntry, bin index not found! Returning 99999" << std::endl;
            }
        }
        else{
            std::cerr << "Error in PsUncertaintyReader::getMapEntry, category name not found! Returning 99999" << std::endl;
        }
    } else {
        std::cerr << "Error in PsUncertaintyReader::getMapEntry, process name not found! Returning 99999" << std::endl;
    }

    return 99999.;
}


unsigned PsUncertaintyReader::findBinIndex( TH1D* hist, const double entry ) const{
    unsigned binIndex = (unsigned) hist->FindBin( entry );
    return binIndex;
}


double PsUncertaintyReader::getMapEntry( const std::map< std::string, std::map < std::string, std::map < unsigned, double > > >& uncMaps, const std::string& process, 
    const std::string& category, TH1D* hist, const double entry) const
{
    unsigned binIndex = findBinIndex(hist, entry);
    return getMapEntry( uncMaps, process, category, binIndex);
}


double PsUncertaintyReader::getISRUncertaintyDown(const std::string& process, const std::string& category, TH1D* hist, const double entry) const{
    return getMapEntry( uncertaintyMaps[0], process, category, hist, entry);
}


double PsUncertaintyReader::getISRUncertaintyUp(const std::string& process, const std::string& category, TH1D* hist, const double entry) const{
    return getMapEntry( uncertaintyMaps[1], process, category, hist, entry);
}


double PsUncertaintyReader::getFSRUncertaintyDown(const std::string& process, const std::string& category, TH1D* hist, const double entry) const{
    return getMapEntry( uncertaintyMaps[2], process, category, hist, entry );
}


double PsUncertaintyReader::getFSRUncertaintyUp(const std::string& process, const std::string& category, TH1D* hist, const double entry) const{
    return getMapEntry( uncertaintyMaps[3], process, category, hist, entry);
}

