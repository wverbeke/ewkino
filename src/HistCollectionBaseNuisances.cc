/*
   class implementation of HistCollectionBaseNuisances
 */

#include "../interface/HistCollectionBaseNuisances.h"

//include ROOT classes
#include "TROOT.h"


HistCollectionBaseNuisances::HistCollectionBaseNuisances(const std::shared_ptr<HistInfo>& info, const std::shared_ptr<Sample>& sam, const std::shared_ptr<Category>& cat, 
    const std::shared_ptr< std::vector < std::string > >& nuisanceNames, const bool includeSB): HistCollectionBase(info, sam, cat, includeSB)
{
    for(size_t n = 0; n < nuisanceNames->size(); ++n){
        nuisanceCollection.push_back( std::vector< std::shared_ptr< TH1D > >(size() ) );
        if(hasSideBand()){
            nuisanceSideBand.push_back( std::vector< std::shared_ptr< TH1D > >(size() ) );
        }
        for(unsigned c = 0; c < size(); ++c){
            nuisanceCollection[n][c] = info->makeHist( cat->name(c) +sam->getFileName() + "_" + (*nuisanceNames)[n] );
            if(hasSideBand()){   
                nuisanceSideBand[n][c] = info->makeHist( cat->name(c) +sam->getFileName() + "_sideband_" + (*nuisanceNames)[n] );
            }
        }
    }
}

HistCollectionBaseNuisances::HistCollectionBaseNuisances(const std::string& inputFileName, const std::shared_ptr< HistInfo>& info, const std::shared_ptr<Sample>& sam,
    const std::shared_ptr< Category >& cat, const std::shared_ptr< std::vector < std::string > >& nuisanceNames, const bool includeSB): HistCollectionBase(inputFileName, info, sam, cat, includeSB)
{
    //Open root file to read
    TFile* inputFile = TFile::Open( (const TString&) inputFileName, "READ");
    //read histpgram for every nuisance
    for(size_t n = 0; n < nuisanceNames->size(); ++n){
        nuisanceCollection.push_back( std::vector < std::shared_ptr < TH1D > >( size() ) );
        if(hasSideBand()){
            nuisanceSideBand.push_back( std::vector < std::shared_ptr < TH1D > >( size() ) );
        }
        //read histogram for every category
        for(size_t c = 0; c < size(); ++c){
            nuisanceCollection[n][c] = std::shared_ptr<TH1D>( (TH1D*) inputFile->Get( (const TString&) name(c) ) );
            //take away ownership from inputFile so root does not delete histogram
            nuisanceCollection[n][c]->SetDirectory(gROOT);
            if(includeSB){
                nuisanceSideBand[n][c] = std::shared_ptr<TH1D>( (TH1D*) inputFile->Get( (const TString&) name(c, true) ) );
                nuisanceSideBand[n][c]->SetDirectory(gROOT);
            }
        }
        inputFile->Close(); 
    }
}



std::shared_ptr<TH1D> HistCollectionBaseNuisances::accessNuisance(const size_t nuisanceIndex, const size_t categoryIndex, const bool sb){
    if(!sb){
        return nuisanceCollection[nuisanceIndex][categoryIndex];
    } else{
        return nuisanceSideBand[nuisanceIndex][categoryIndex];
    }
}

std::shared_ptr<TH1D> HistCollectionBaseNuisances::accessNuisance(const size_t nuisanceIndex, const std::vector< size_t >& categoryIndices, const bool sb){
    size_t categoryIndex = getCategoryIndex(categoryIndices);
    return accessNuisance(nuisanceIndex, categoryIndex, sb);
}
