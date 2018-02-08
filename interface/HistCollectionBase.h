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
        std::shared_ptr< TH1D > access(const std::vector<size_t>& categoryIndices) const{ 
            return collection[category->getIndex(categoryIndices)];
        }
    
        std::string infoName() const { return histInfo->name(); }
        std::string sampleFileName() const { return sample->getFileName(); }
        std::string sampleProcName() const { return sample->getFileName(); }
        std::string categoryName(size_t categoryIndex) const { return category->name(categoryIndex); }
        std::string categoryName(const std::vector<size_t>& categoryIndices) const { return category->name(categoryIndices); }
        size_t size() const{ return collection.size(); }
        //full name of every histogram
        std::string name(size_t categoryIndex, const bool sideband = false) const{
            return histInfo->name() + category->name(categoryIndex) + (sideband ? "sideband" : "");
        }
        //add two histcollections
        HistCollectionBase& operator+=(const HistCollectionBase& rhs);
    private:
        std::vector< std::shared_ptr < TH1D > > collection;
        std::vector< std::shared_ptr < TH1D > > sideBand;
        std::shared_ptr< Category > category;
        std::shared_ptr< Sample >   sample;
        std::shared_ptr< HistInfo > histInfo;
};
#endif
