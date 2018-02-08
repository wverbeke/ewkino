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

        std::shared_ptr< TH1D > access(size_t sampleIndex, const std::vector< size_t>& categoryIndices) const {
            return collection[sampleIndex].access(categoryIndices);
        }
        /* 
        std::string infoName() const{
            return collection.front.infoName();
        }
        std::string categoryName(size_t categoryIndex) const{ 
            return collection.front.categoryName(categoryIndex);
        }
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
        Plot getPlot() const; //return plot corresponding to histogram
    private:
        std::vector< HistCollectionBase > collection;
        std::vector<std::string> getFileNames(const std::string&);      //get list of files in directory
        bool merged = false;
        void mergeProcesses();                                          //merge histograms of same process
        void negBinsToZero() const;                                     //set negative bin contents to 0
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
