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
#include "Plot.h"

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
        std::shared_ptr<TH1D> access(size_t, const std::vector<size_t>&, bool sb = false) const;                //access specific histogram
        std::string name(size_t, const std::vector<size_t>&, bool sb = false) const;                            //access name of histogram at this location
        size_t catRange(size_t c) const { return cat->getRange(c);}                                             //return range of i-th category
        size_t infoRange() const { return histInfo->size();}
        void store(const std::string&);                                                                         //write all histograms to ROOT files in given location
        void setNegZero();
        bool isData() const { return sample->isData(); }
        std::string infoName(size_t infoInd) const { return (*histInfo)[infoInd].name(); }
        std::string procName() const { return sample->getProc(); }
        std::string catName(const std::vector<size_t>& catIndices) const { return cat->name(catIndices); }
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
        HistCollection(std::shared_ptr< std::vector < HistInfo > >, const std::vector<Sample>&, std::shared_ptr<Category>, bool includeSB = false);
        HistCollection(const std::vector<HistInfo>&, const std::vector<Sample>&, std::shared_ptr<Category>, bool includeSB = false);
        HistCollection(const std::vector<HistInfo>&, const std::vector<Sample>&, const std::vector< std::vector < std::string > >& categorization = std::vector < std::vector < std::string> >(), bool includeSB = false);
       // std::shared_ptr<TH1D> access(size_t, size_t, const std::vector<size_t>&, bool sb = false) const{
        std::shared_ptr<TH1D> access(size_t samIndex, size_t infoIndex, const std::vector<size_t>& catIndices, bool sb = false) const;
        std::string name(size_t, size_t, const std::vector<size_t>&, bool sb = false) const;
        size_t samRange() const { return fullCollection.size(); }
        size_t catRange(size_t c) const{ return (*this)[0].catRange(c); }
        size_t infoRange() const { return (*this)[0].infoRange(); }
        void mergeProcesses();
        void store();
        void setNegZero();
        void push_back(const HistCollectionSample& colSam) { fullCollection.push_back(colSam); }
        const HistCollectionSample& operator[](size_t ind) const{return fullCollection[ind];}
        const HistCollectionSample& operator[](const Sample&) const;
        Plot getPlot(size_t, const std::vector<size_t>&) const;
    private:
        std::vector< HistCollectionSample > fullCollection;
};
#endif
