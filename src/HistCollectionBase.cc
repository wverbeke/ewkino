#include "../interface/HistCollectionBase.h"

//include ROOT classes
#include "TFile.h"
#include "TROOT.h"

HistCollectionBase::HistCollectionBase(const std::shared_ptr< HistInfo >& info, const std::shared_ptr< Sample >& sam, const std::shared_ptr< Category >& cat, const bool includeSB):
    histInfo(info), sample(sam), category(cat){
    //make histogram for every category
    for(auto catIt = cat->cbegin(); catIt != cat->cend(); ++catIt){
        collection.push_back( info->makeHist(*catIt + sample->getFileName() ) );
        if(includeSB) sideBand.push_back(  info->makeHist(*catIt + sample->getFileName() + "_sideband") );
    }
}


HistCollectionBase::HistCollectionBase(const std::string& inputFileName, const std::shared_ptr< HistInfo >& info, const std::shared_ptr< Sample >& sam, const std::shared_ptr< Category >& cat, const bool includeSB):
    histInfo(info), sample(sam), category(cat){
    //Open root file to read
    TFile* inputFile = TFile::Open( (const TString&) inputFileName, "READ");
    //read correct histogram for every category
    for(size_t c = 0; c < category->size(); ++c){
        collection.push_back(std::shared_ptr<TH1D>( (TH1D*) inputFile->Get( (const TString&) name(c) ) ) );
        //take away ownership from inputFile so root does not delete histogram
        collection[c]->SetDirectory(gROOT);
        if(includeSB){
            sideBand.push_back(std::shared_ptr<TH1D>( (TH1D*) inputFile->Get( (const TString&) name(c, true) ) ) );
            sideBand[c]->SetDirectory(gROOT);
        }
    }
    inputFile->Close();
}


std::shared_ptr< TH1D > HistCollectionBase::access(const size_t categoryIndex, const bool sb) const{
    if(!sb){
        return collection[categoryIndex];
    } else{
        return sideBand[categoryIndex];
    }
}

std::shared_ptr< TH1D > HistCollectionBase::access(const std::vector<size_t>& categoryIndices, const bool sb) const{
    size_t categoryIndex = category->getIndex(categoryIndices);
    return access(categoryIndex);
}

HistCollectionBase& HistCollectionBase::operator+=(const HistCollectionBase& rhs){
    if(size() != rhs.size()){
        std::cerr << "Error: attempting to add histCollections of different size: returning *this" << std::endl;
        return *this;
    }
    if(infoName() != rhs.infoName() || sampleProcessName() != rhs.sampleProcessName() ){
        std::cerr << "Error: attempting to add histCollections of different distribution or process: returning *this" << std::endl;
        std::cerr << "infoName() = " << infoName() << "\t" << "sampleProcessName() = " << sampleProcessName() << std::endl;
        std::cerr << "rhs.infoName() = " << rhs.infoName() << "\t" << "rhs.sampleProcessName() = " << rhs.sampleProcessName() << std::endl;
        return *this;
    }
    if(hasSideBand() && !rhs.hasSideBand() ){
        std::cerr << "Error: attempting to add histcollection with sideband to one without sideband: returning *this" << std::endl;
        return *this;
    }
    for(size_t h = 0; h < size(); ++h){
        collection[h]->Add( (rhs.collection[h]).get() );
        if(hasSideBand()){
            sideBand[h]->Add( (rhs.sideBand[h]).get() );
        }
    }
    return *this;
}

void setNegativeBinsToZero(const std::shared_ptr<TH1D>& h){
    //check each bin of the histogram, and set its binContent to 0 if it is negative
    for(unsigned b = 1; b < h->GetNbinsX() + 1; ++b){
        if(h->GetBinContent(b) < 0.) h->SetBinContent(b, 0.);
    }
}

void HistCollectionBase::negBinsToZero() const{
    /*
    Note that only collection is affected here since sideband is allowed to remain negative
    */
    //loop over all histograms in collection 
    for(auto colIt = collection.cbegin(); colIt != collection.cend(); ++colIt){ 
        setNegativeBinsToZero(*colIt); 
    }
}
