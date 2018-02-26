#ifndef HistCollectionDist_H
#define HistCollectionDist_H

//include c++ library classes
#include <set>

//include other parts of code 
#include "HistCollectionBase.h"
#include "Plot.h"

//include ROOT classes

class HistCollectionDist{
    public:
        //direct initialization
        //HistCollectionDist(const std::shared_ptr<HistInfo>&, const std::vector< std::shared_ptr< Sample > >&, const std::shared_ptr< Category >&);
        //initialize by reading histograms from directory (given as string) :
        HistCollectionDist(const std::string&, const std::shared_ptr< HistInfo >&, const std::vector< std::shared_ptr< Sample > >&, const std::shared_ptr< Category >&, const bool sideBand = false);
        HistCollectionDist(const std::string&, const HistInfo&, const std::vector< Sample >&, const Category&, const bool sideBand = false);
        HistCollectionDist(const std::string&, const HistInfo&, const std::vector< Sample >&, const std::vector< std::vector < std::string > >& categoryVec = std::vector< std::vector< std::string > >(), 
            const bool sideBand = false);

        //output all plots to file, given options will be given to Plot "draw" function
        //function is non-const because histogram merge might be called before plotting
        void printPlots(const std::string& outputDirectory, const bool is2016 = true, const std::string& analysis = "", bool log = false, bool normToData = false, TH1D** bkgSyst = nullptr, const bool sigNorm = true); 

        //routine to blind data in given category, if no name is given all categories are blinded!
        void blindData(const std::string& catName = "");
    private:
        std::vector< HistCollectionBase > collection;
        std::vector<std::string> getFileNames(const std::string&);      //get list of files in directory
        
        //merge histograms of same process
        bool merged = false;
        void mergeProcesses();

        //set negative bin contents to 0
        void negBinsToZero() const;

        //make file path for plot depending on category
        std::string plotPath(const size_t categoryIndex) const;

        //get distribution name
        std::string distributionName() const{
            return collection.front().infoName();
        }

        //get category name
        std::string categoryName(const size_t categoryIndex) const{ 
            return collection.front().categoryName(categoryIndex);
        }

        //get name = distribution's name + category's name
        std::string name(const size_t categoryIndex) const;

        //get total sideband histogram
        std::shared_ptr< TH1D > getTotalSideBand(const size_t categoryIndex) const;

        //check whether sideband is present
        bool hasSideBand() const {
            return collection.front().hasSideBand();
        }

        //amount of categories
        size_t categorySize() const{ return collection.front().size(); }

        //helper functions for getting plot
        std::map< std::string, std::pair< std::shared_ptr< TH1D >, bool > > getBkgMap(const size_t categoryIndex) const;
        std::shared_ptr<TH1D> getObsHist(const size_t categoryIndex) const;

        //return Plot corresponding to categoryIndex
        Plot getPlot(const size_t categoryIndex);

        //format plot header, depending on category name
        std::string plotHeader(const size_t categoryIndex, const bool is2016) const;

        //blind data given a category index
        void blindData(const size_t);

        //keep track of blinded categories for legend names in plots    
        std::set<size_t> blindedCategories;
        bool categoryIsBlinded(const size_t categoryIndex) const;
};
#endif
