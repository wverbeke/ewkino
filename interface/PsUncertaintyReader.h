/*
helper class to read ps uncertainties computed with 2017 tZq sample for 2016 analysis
*/
#ifndef PsUncertaintyReader_H
#define PsUncertaintyReader_H

//include c++ library classes 
#include <map>
#include <string>
#include <vector>

//include ROOT classes 
#include "TH1D.h"

class PsUncertaintyReader{
    public:
        PsUncertaintyReader( const std::string& fileName );

        double getISRUncertaintyDown( const std::string& process, const std::string& category, TH1D* hist, const double entry) const;
        double getISRUncertaintyUp( const std::string& process, const std::string& category, TH1D* hist, const double entry) const;
        double getFSRUncertaintyDown( const std::string& process, const std::string& category, TH1D* hist, const double entry) const;
        double getFSRUncertaintyUp( const std::string& process, const std::string& category, TH1D* hist, const double entry) const;

    private:
        std::vector< std::map< std::string, std::map< std::string, std::map< unsigned, double > > > > uncertaintyMaps =  
            std::vector< std::map< std::string, std::map< std::string, std::map< unsigned, double > > > >(4);

        double getMapEntry( const std::map< std::string, std::map < std::string, std::map < unsigned, double > > >&, const std::string& process, const std::string& category, const unsigned bin ) const;
        double getMapEntry( const std::map< std::string, std::map < std::string, std::map < unsigned, double > > >&, const std::string& process, const std::string& category, TH1D*, const double entry) const;
        unsigned findBinIndex( TH1D* hist, const double entry ) const;
};
#endif
