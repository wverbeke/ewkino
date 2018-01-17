/*
Class that builds a collection of histograms for a given sample.
Required inputs are a collection of HistInfo objects and a categorization.
*/

#ifndef HistCollection_H
#define HistCollection_H

//include c++ library classes
#include <vector>
#include <memory>
#include <iostream>

//include ROOT classes
#include "TH1D.h"

//include other parts of code 
#include "HistInfo.h"
#include "Category.h"
#include "Sample.h"

class HistCollectionSample{
    friend HistCollectionSample operator+(const HistCollectionSample&, const HistCollectionSample&); 
    friend class HistCollection;
    public:
        HistCollectionSample() = default;
        HistCollectionSample(std::shared_ptr<std::vector < HistInfo > >, std::shared_ptr<Sample>, std::shared_ptr<Category>, bool includeSB = false);
        HistCollectionSample(const std::vector<HistInfo>&, std::shared_ptr<Sample>, std::shared_ptr<Category>, bool includeSB = false);
        HistCollectionSample(std::shared_ptr<std::vector < HistInfo > >, std::shared_ptr<Sample>, const std::vector< std::vector< std::string > >& categorization = std::vector < std::vector <std::string> >()
                                , bool includeSB = false);
        HistCollectionSample(const std::vector<HistInfo>&, std::shared_ptr<Sample>, const std::vector< std::vector< std::string > >& categorization = std::vector < std::vector <std::string> >()
                                , bool includeSB = false);
        std::shared_ptr<TH1D> access(size_t, const std::vector<size_t>&, bool sb = false) const;         //access specific histogram
        std::string name(size_t, const std::vector<size_t>&) const;                                      //access name of histogram at this location
        void store(const std::string&);                                                                  //write all histograms to ROOT files in given location
        void setNegZero();
        HistCollectionSample& operator+=(const HistCollectionSample&);
    private:
        std::vector< std::vector < std::shared_ptr<TH1D> > > collection;
        std::vector< std::vector < std::shared_ptr<TH1D> > > sideBand;
        std::shared_ptr<Sample> sample;
        std::shared_ptr<Category> cat;
        std::shared_ptr< std::vector < HistInfo > > histInfo;
};

class HistCollection{
    public:
        HistCollection() = default;
        HistCollection(const std::vector<HistInfo>&, const std::vector<Sample>&, std::shared_ptr<Category>, bool includeSB = false);
        HistCollection(const std::vector<HistInfo>&, const std::vector<Sample>&, const std::vector< std::vector < std::string > >& categorization = std::vector < std::vector < std::string> >(), bool includeSB = false);
        std::shared_ptr<TH1D> access(size_t, size_t, const std::vector<size_t>&, bool sb = false) const;
        void mergeProcesses();
        void store();
        void setNegZero();
        void push_back(const HistCollectionSample& colSam) { fullCollection.push_back(colSam); }
        const HistCollectionSample& operator[](size_t ind) const{return fullCollection[ind];}
        const HistCollectionSample& operator[](const Sample&) const;
    private:
        std::vector< HistCollectionSample > fullCollection;
        std::shared_ptr<std::vector<HistInfo>> histInfoV;
};
#endif
