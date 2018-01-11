/*
Class that builds a collection of histograms for a given sample.
Required inputs are a collection of HistInfo objects and a categorization.
*/

#ifndef HistCollection_H
#define HistCollection_H

//include c++ library classes
#include <vector>
#include <memory>

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
        HistCollectionSample(const std::vector<HistInfo>&, std::shared_ptr<Category>, std::shared_ptr<Sample>);
        HistCollectionSample(const std::vector<HistInfo>&, const std::vector< std::vector< std::string > >&, std::shared_ptr<Sample>);
        std::shared_ptr<TH1D> access(size_t, const std::vector<size_t>&) const; //access specific histogram
        void store(const std::string&);                                         //write all histograms to ROOT files in given location
        void setNegZero();
        HistCollectionSample& operator+=(const HistCollectionSample&);
    private:
        std::vector< std::vector < std::shared_ptr<TH1D> > > collection;
        std::shared_ptr<Sample> sample;
        std::shared_ptr<Category> cat;
};

class HistCollection{
    public:
        HistCollection() = default;
        HistCollection(const std::vector<HistInfo>&, std::shared_ptr<Category>, const std::vector<Sample>&);
        HistCollection(const std::vector<HistInfo>&, const std::vector< std::vector < std::string > >&, const std::vector<Sample>&);
        std::shared_ptr<TH1D> access(size_t, size_t, const std::vector<size_t>&) const;
        void mergeProcesses();
        void store();
        void setNegZero();
    private:
        std::vector< HistCollectionSample > fullCollection;
};
#endif
