/*
Some useful functions used at multiple places in this directory 
*/

#include "systematicsTools.h"

std::vector< std::shared_ptr< TH1 > > getHistogramsFromFile( const std::string& fileName){
    // get a list of all histograms stored in a root file, assuming top-level directory.
    // based on ewkino/skimmer/skimmer.cc

    std::vector< std::shared_ptr< TH1 > > histogramVector;

    std::cout << "opening file " << fileName << std::endl;
    TFile* f = TFile::Open(fileName.c_str());
    TList* keyList = f->GetListOfKeys();

    for( const auto objectPtr : *keyList ){
        TH1* histPtr = dynamic_cast< TH1* >( f->Get( objectPtr->GetName() ) );
        if( histPtr ){
            histPtr->SetDirectory( gROOT );
            histogramVector.emplace_back( histPtr );
        }
    }

    f->Close();
    return histogramVector;
}


std::vector< std::shared_ptr< TH1 > > selectHistograms(
                std::vector<std::shared_ptr<TH1>> histogramVector,
                std::vector<std::string> mustContain,
                std::vector<std::string> mayNotContain ){

    std::vector< std::shared_ptr<TH1> > selHists;
    for( std::shared_ptr<TH1> histPtr : histogramVector ){
        // first check if histogram name contains necessary tags
        bool keep = true;
        for( std::string mustContainTag : mustContain ){
            if(!stringTools::stringContains(histPtr->GetName(),mustContainTag)){
                keep = false;
                break;
            }
        }
        if(!keep) continue;
        // then check if histogram does not contain forbidden tags
        for( std::string mayNotContainTag : mayNotContain ){
            if(stringTools::stringContains(histPtr->GetName(),mayNotContainTag)){
                keep = false;
                break;
            }
        }
        if(!keep) continue;
        selHists.push_back( std::shared_ptr<TH1>( (TH1*) histPtr->Clone() ) );
    }
    return selHists;
}


std::shared_ptr< TH1 > addHistograms( std::vector<std::shared_ptr<TH1>> histogramVector ){
    // add all histograms in the input vector of histograms
    int nHists=0;
    std::shared_ptr<TH1> sumHist;
    for( std::shared_ptr<TH1> histPtr : histogramVector ){
        std::cout << "adding histogram " << histPtr->GetName() << std::endl;
        // if it is the first one to be found, simply copy it
        if(nHists==0){
            sumHist = std::shared_ptr<TH1>( (TH1*) histPtr->Clone() );
            sumHist->SetName("sum");
            sumHist->SetTitle("sum");
            ++nHists;
            continue;
        }
        // else add it
        sumHist.get()->Add(histPtr.get());
        ++nHists;
    }
    std::cout << "added " << nHists << " histograms" << std::endl;
    return sumHist;
}


std::map< std::string, std::shared_ptr<TH1> > getEnvelope( 
    std::vector< std::shared_ptr<TH1> > hists, std::shared_ptr<TH1> refHist){
    // calculate bin-per-bin envelope of a series of histograms
    // refHist is used to initialize the up and down envelopes
    // and catch exceptions if vector is empty
    
    std::map< std::string, std::shared_ptr<TH1> > res;
    // initialize the up and down variations to the first histogram in the vector
    res["up"] = std::shared_ptr<TH1>( (TH1*) refHist->Clone() );
    res["down"] = std::shared_ptr<TH1>( (TH1*) refHist->Clone() );
    // calculate envelope
    for( const std::shared_ptr<TH1> hist: hists ){
        for(int i=1; i<hist->GetNbinsX()+1; ++i){
            if( hist->GetBinContent(i) > res["up"]->GetBinContent(i) ){
                res["up"]->SetBinContent(i,hist->GetBinContent(i));
            }
            if( hist->GetBinContent(i) < res["down"]->GetBinContent(i) ){
                res["down"]->SetBinContent(i,hist->GetBinContent(i));
            }
        }
    }
    return res;
}


std::map< std::string, std::shared_ptr<TH1> > getRMS( 
    std::vector< std::shared_ptr<TH1> > hists, std::shared_ptr<TH1> refHist ){
    // calculate bin-per-bin rms of a series of histograms wrt refHist
    
    std::map< std::string, std::shared_ptr<TH1> > res;
    // initialize the up and down variations to the refHist
    res["up"] = std::shared_ptr<TH1>( (TH1*) refHist->Clone() );
    res["down"] = std::shared_ptr<TH1>( (TH1*) refHist->Clone() );
    // further initializations
    std::shared_ptr<TH1> varRMSHist = std::shared_ptr<TH1>( (TH1*) refHist->Clone() );
    varRMSHist->Reset();
    int nElements = hists.size();
    // catch exception of zero elements to average over 
    if( nElements==0 ) return res;
    // fill varRMSHist with the sum of squared differences wrt refHist
    for( const std::shared_ptr<TH1> hist: hists ){
        for(int i=1; i<hist->GetNbinsX()+1; ++i){
            double var = hist->GetBinContent(i) - refHist->GetBinContent(i);
            varRMSHist->SetBinContent(i,varRMSHist->GetBinContent(i)+var*var);
        }
    }
    // average and take sqrt
    for(int i=1; i<refHist->GetNbinsX()+1; ++i){
        varRMSHist->SetBinContent(i,std::sqrt(varRMSHist->GetBinContent(i)/nElements));
        res["up"]->SetBinContent(i,refHist->GetBinContent(i)
                                                +varRMSHist->GetBinContent(i));
        res["down"]->SetBinContent(i,refHist->GetBinContent(i)
                                                -varRMSHist->GetBinContent(i));
    }
    return res;
}

