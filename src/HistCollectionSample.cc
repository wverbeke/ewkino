#include "../interface/HistCollectionSample.h"

HistCollectionSample::HistCollectionSample(const std::vector< std::shared_ptr< HistInfo > >& infoList, const std::shared_ptr<Sample>& sam, const std::shared_ptr<Category>& cat){
    for(auto it = infoList.cbegin(); it != infoList.cend(); ++it){
        collection.push_back(HistCollectionBase(*it, sam, cat));
    }
}

void HistCollectionSample::store(const std::string& directory, const long unsigned begin, const long unsigned end) const{
    //extra string indicating event numbers when sample is split in multiple jobs
    std::string extra("");
    if(begin != 0 || end != 0){
        extra = "_" + std::to_string(begin) + "_" + std::to_string(end);
    }
    //create new root file
    TFile* outputFile = TFile::Open((const TString&) directory + "/tempHist_" + sampleFileName() + extra, "RECREATE");
    for(size_t dist = 0; dist < collection.size(); ++dist){
        for(size_t c = 0; c < collection[dist].size(); ++c){
            access(dist, c, false)->Write();
            if(hasSideBand() )  access(dist, c, true)->Write();
        }
    }
    outputFile->Close();
}


