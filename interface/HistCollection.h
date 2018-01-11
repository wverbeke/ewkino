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

class HistCollection{
    public:
        HistCollection(const std::vector<HistInfo>&, const std::vector< std::vector< std::string > >&, const std::string&);
        std::shared_ptr<TH1D> access(size_t, const std::vector<size_t>&);
    private:
        std::vector< std::vector < std::shared_ptr<TH1D> > > collection;
        Category cat;
        std::string sampleName;
};
#endif
