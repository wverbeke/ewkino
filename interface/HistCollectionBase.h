#ifndef HistCollectionBase_H
#define HistCollectionBase_H

//include c++ library classes
#include <vector>
#include <memory>

//include other parts of code 
#include "Category.h"
#include "HistInfo.h"
#include "Sample.h"

//include ROOT classes
#include "TH1D.h"

class HistCollectionBase{
    public:
        //direct construction
        HistCollectionBase(const std::shared_ptr<HistInfo>&, const std::shared_ptr<Sample>&, const std::shared_ptr<Category>&, const bool includeSB = false);
        //construction by reading info from file
        HistCollectionBase(const std::string&, const std::shared_ptr<HistInfo>&, const std::shared_ptr<Sample>&, const std::shared_ptr<Category>&, const bool includeSB = false);
        //access routines
        std::shared_ptr< TH1D > access(const size_t categoryIndex, const bool sb = false) const;  
        std::shared_ptr< TH1D > access(const std::vector<size_t>& categoryIndices, const bool sb = false) const;
        
        std::string infoName() const { return histInfo->name(); }
        //std::string sampleFileName() const { return sample->getFileName(); }
        std::string sampleUniqueName() const { return sample->getUniqueName(); }
        std::string sampleProcessName() const { return sample->getProcessName(); }
        std::string categoryName(size_t categoryIndex) const { return category->name(categoryIndex); }
        std::string categoryName(const std::vector<size_t>& categoryIndices) const { return category->name(categoryIndices); }

        //full name of every histogram
        std::string name(size_t categoryIndex, const bool sideband = false) const{
            return infoName() + categoryName(categoryIndex) + (sideband ? "sideband" : "") + sampleUniqueName(); 
        }
        size_t size() const{ return collection.size(); }

        //add two histcollections
        HistCollectionBase& operator+=(const HistCollectionBase& rhs);

        //set negative bin contents to 0
        void negBinsToZero() const;

        //rebin all histograms in category with certain value
        void rebin(const std::string&, const int) const;

        //check if collection contains sideband
        bool hasSideBand() const { return ! (sideBand.empty()); }

        //check if this is a data collection
        bool isData() const { return sample->isData(); }

        //check if this is a SM signal
        bool isSMSignal() const { return sample->isSMSignal(); }

        //check if this a new physics signal
        bool isNewPhysicsSignal() const { return sample->isNewPhysicsSignal(); }

        //range for every categorization
        size_t categoryRange(const size_t c) const { return category->getRange(c); }

        size_t getCategoryIndex(const std::vector<size_t>& categoryIndices) const { return category->getIndex(categoryIndices); }

    private:
        std::vector< std::shared_ptr < TH1D > > collection;
        std::vector< std::shared_ptr < TH1D > > sideBand;
        std::shared_ptr< Category > category;
        std::shared_ptr< Sample >   sample;
        std::shared_ptr< HistInfo > histInfo;
       
        //rebin all histograms of given category index
        void rebin(const size_t, const int) const;
};
//set all negative bins in histogram to zero
void setNegativeBinsToZero(const std::shared_ptr< TH1D >&);
#endif
