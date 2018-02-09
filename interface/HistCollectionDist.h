#ifndef HistCollectionDist_H
#define HistCollectionDist_H

//include c++ library classes

//include other parts of code 
#include "HistCollectionBase.h"
#include "Plot.h"

//include ROOT classes

class HistCollectionDist{
    public:
        //direct initialization
        //HistCollectionDist(const std::shared_ptr<HistInfo>&, const std::vector< std::shared_ptr< Sample > >&, const std::shared_ptr< Category >&);
        //initialize by reading histograms from directory (given as string) :
        HistCollectionDist(const std::string&, const std::shared_ptr< HistInfo >&, const std::vector< std::shared_ptr< Sample > >&, const std::shared_ptr< Category >&);
        HistCollectionDist(const std::string&, const HistInfo&, const std::vector< Sample >&, const Category&);
        HistCollectionDist(const std::string&, const HistInfo&, const std::vector< Sample >&, const std::vector< std::vector < std::string > >& categoryVec = std::vector< std::vector< std::string > >() );

        Plot getPlot(const size_t categoryIndex); //return plot corresponding to category
        size_t categorySize() const{ return collection.front().size(); }
    private:
        std::vector< HistCollectionBase > collection;
        std::vector<std::string> getFileNames(const std::string&);      //get list of files in directory
        bool merged = false;
        void mergeProcesses();                                          //merge histograms of same process
        void negBinsToZero() const;                                     //set negative bin contents to 0
        //make file path for plot depending on category
        std::string plotPath(const size_t categoryIndex) const;
        //get category name
        std::string categoryName(size_t categoryIndex) const{ 
            return collection.front().categoryName(categoryIndex);
        }
        //get distribution name
        std::string distributionName() const{
            return collection.front().infoName();
        }
        //get name = distribution's name + category's name
        std::string name(const size_t categoryIndex) const;
        //get total sideband histogram
        std::shared_ptr< TH1D > getTotalSideBand(const size_t categoryIndex) const;
        //check whether sideband is present
        bool hasSideBand() const {
            return collection.front().hasSideBand();
        }
};
#endif
