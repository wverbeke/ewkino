/*
version of HistCollectionBase class that also includes nuistances
*/
#ifndef HistCollectionBaseNuisances_H
#define HistCollectionBaseNuisances_H

//include other parts of code 
#include "HistCollectionBase.h"

//include c++ library classes

//include ROOT classes 

class HistCollectionBaseNuisances : public HistCollectionBase{
    
    public:
        HistCollectionBaseNuisances(const std::shared_ptr<HistInfo>&, const std::shared_ptr<Sample>&, const std::shared_ptr<Category>&, const std::vector < std::string >&, const bool includeSB = false);
        HistCollectionBaseNuisances(const std::string&, const std::shared_ptr<HistInfo>&, const std::shared_ptr<Sample>&, const std::shared_ptr<Category>&, const std::vector < std::string >&, const bool includeSB = false);

        std::shared_ptr<TH1D> accessNuisance(const size_t nuisanceIndex, const size_t categoryIndex, const bool sb);
        std::shared_ptr<TH1D> accessNuisance(const size_t nuisanceIndex, const std::vector< size_t >& categoryIndices, const bool sb);

        void negBinsToZero() const;

   private:
        std::vector < std::vector < std::shared_ptr < TH1D > > > nuisanceCollection;
        std::vector < std::vector < std::shared_ptr < TH1D > > > nuisanceSideBand;
};


#endif
