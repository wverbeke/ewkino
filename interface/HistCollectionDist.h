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
        HistCollectionDist(const std::shared_ptr<HistInfo>&, const std::vector< std::shared_ptr< Sample > >&, const std::shared_ptr< Category >&);
        //initialize by reading histograms from directory (given as string) :
        HistCollectionDist(const std::string&, const std::shared_ptr< HistInfo >&, const std::vector< std::shared_ptr< Sample > >&, const std::shared_ptr< Category >&);
        /*
        std::shared_ptr< TH1D > access(size_t sampleIndex, const std::vector< size_t>& categoryIndices, const bool sb = false) const {
            return collection[sampleIndex].access(categoryIndices, sb);
        }
        */
        /* 
        std::string categoryName(const std::vector<size_t>& categoryIndices) const{ 
            return collection.front.categoryName(categoryIndices);
        }
        std::string sampleFileName(size_t sampleIndex) const{
            return collection[sampleIndex].sampleFileName();
        }
        std::string sampleProcName(size_t sampleIndex) const{
            return collection[sampleIndex].procName();
        }
        */
        //HistCollectionBase& operator[](size_t sampleIndex) const{ return collection[sampleIndex]; }
        Plot getPlot(const size_t categoryIndex); //return plot corresponding to category
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

/*
//simple helper class to make list of output files
class fileList{
    public:
        fileList(const std::string&, const std::string&);
        fileList(const std::string& directory): fileList(directory, "fileList.txt");
        ~fileList();

        bool getNextFileName(std::string&);
    private:
        std::string fileName;
        std::ifstream& file;

};
*/
#endif
