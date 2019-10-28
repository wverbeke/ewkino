/*
Class that collects information on the quality of the fit related to the object cuts on the lepton.
This is used for finding the cuts which give the best fake-rate closure 
*/

#ifndef CutsFitInfo_H
#define CutsFitInfo_H

//include c++ library classes
#include <map>
#include <string>
#include <functional>
#include <memory>
#include <iostream>

//include ROOT classes 
#include "TH1D.h"

//include other parts of framework
#include "ConstantFit.h"


class CutsFitInfo{

    friend std::ostream& operator<<( std::ostream&, const CutsFitInfo& );

    public:
        CutsFitInfo( const std::shared_ptr< TH1D >& heavyFlavorFakeRateHist, const std::shared_ptr< TH1D >& lightFlavorFakeRateHist, const std::map< std::string, double >& cutMap );

        double cut( const std::string& ) const;
        double normalizedChi2() const{ return fitInfo.normalizedChi2(); }
        double fitValue() const{ return fitInfo.value(); }
    
        std::shared_ptr< TH1D > heavyFlavorFakeRateHist() const{ return heavyFlavorFakeRate; }
        std::shared_ptr< TH1D > lightFlavorFakeRateHist() const{ return lightFlavorFakeRate; }
        std::shared_ptr< TH1D > fakeRateRatioHist() const{ return fakeRateRatio; }

        std::string cutsString() const;
        void makePlots( const std::string& outputDirectory ) const;

    private:
        std::shared_ptr< TH1D > heavyFlavorFakeRate;
        std::shared_ptr< TH1D > lightFlavorFakeRate;
        std::shared_ptr< TH1D > fakeRateRatio;
        ConstantFit fitInfo;
        std::map< std::string, double > cutValueMap;
};


std::ostream& operator<<( std::ostream&, const CutsFitInfo& );



class CutsFitInfoCollection{
    
    public:
        using size_type = std::vector< CutsFitInfo >::size_type;
        using iterator = std::vector< CutsFitInfo >::iterator; 
        using const_iterator = std::vector< CutsFitInfo >::const_iterator;

        CutsFitInfoCollection() = default;

        void sortByChi2();
        void sortByDiffFromUnity();
        void sortByLossFunction( const double epsilon = 0.01 );

        void push_back( const CutsFitInfo& entry ){ collection.push_back( entry ); }
        CutsFitInfo& operator[]( const size_type index ){ return collection[index]; }

        iterator begin(){ return collection.begin(); }
        const_iterator begin() const{ return collection.cbegin(); }
        const_iterator cbegin() const{ return collection.cbegin(); }

        iterator end(){ return collection.end(); }
        const_iterator end() const{ return collection.cend(); }
        const_iterator cend() const{ return collection.cend(); }

        size_type size() const{ return collection.size(); }

        void printBestCuts( size_type numberOfCuts = 0, std::ostream& os = std::cout ) const;
        void plotBestCuts( size_type numberOfCuts = 0, const std::string& outputDirectory = "./" ) const;
         

    private:
        std::vector< CutsFitInfo > collection;
        std::function< double (const CutsFitInfo& x) > diffFromUnity = []( const CutsFitInfo& x ){ return fabs( ( x.fitValue() - 1. )/x.fitValue() ); };
    
};

#endif
