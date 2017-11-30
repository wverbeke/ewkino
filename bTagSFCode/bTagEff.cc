#include "treeReader"

void treeReader::computeBTagEff(const unsigned wp = 1, const bool clean = true, const bool deepCSV = true){
    //read sample list for computing btagging efficiency
    readSamples("samples_bTagEff.txt");
    //make 2D b-tagging efficiency histograms for all jet flavors
    TH2D* bTaggEff[2][3]; //2: num and denom 3: udsg, c, b
    const double ptBins[17] = {25,30,35,40,45,50,60,70,80,90,100,120,150,200,300,400,600};
    const double etaBins[7] = {0, 0.4, 0.8, 1.2, 1.6, 2.0, 2.4};
    const std::string denNum[2] = {"numerator", "denominator"};
    const std::string quarkFlav[3] = {"udsg", "charm", "beauty"};
    for(unsigned i = 0; i < 2; ++i){
        for(unsigned flav = 0; flav < 3; ++flav){
            bTaggEff[i][flav] = new TH2D((const TString&) "bTagEff" + quarkFlav[flav] + denNum[i], (const TString&) "bTagEff" + quarkFlav[flav] + denNum[i] + ";" + "P_{T}(jet) (GeV)" + "; |#eta(jet)|", 16, ptBins, 6, etaBins);
            bTaggEff[i][flav]->Sumw2();
        }
    }
    //loop over all samples 
    for(size_t sam = 0; sam < samples.size(); ++sam){
        initSample(); 
        std::cout<<"Entries in "<< std::get<1>(samples[sam]) << " " << nEntries << std::endl;
        double progress = 0;    //for printing progress bar
        for(long unsigned it = 0; it < nEntries; ++it){
            //print progress bar        
            if(it%100 == 0 && it != 0){
                progress += (double) (100./nEntries);
                tools::printProgress(progress);
            } else if(it == nEntries -1){
                progress = 1.;
                tools::printProgress(progress);
            }
            GetEntry(it);
            //vector containing good lepton indices
            std::vector<unsigned> ind;
            //select leptons
            const unsigned lCount = selectLep(ind);
            for(unsigned j = 0; j < nJets; ++j){
                if(jetIsGood(j, 25., 0, clean) && fabs(_jetEta[j]) < 2.4){
                    if(_jetHadronFlavor[j] == 0 || _jetHadronFlavor[j] == 4 || _jetHadronFlavor[j] == 5){
                        unsigned flav = 0 + (_jetHadronFlavor[j] == 4) + 2*(_jetHadronFlavor[j] == 5);
                        //fill denominator
                        if(bTagged(j, wp, deepCSV)){
                            bTagEff[0][flav]->Fill(std::min(_jetPt[j], 599.), std::min( fabs(_jetEta[j]), 2.4), weight);
                        }
                        //fill numerator
                        bTagEff[1][flav]->Fill(std::min(_jetPt[j], 599.), std::min( fabs(_jetEta[j]), 2.4), weight);
                    }
                }
            }
        }
    }
    //determine name of outputfile
    const std::string wpNames[3] = {"loose", "medium", "tight"};
    const std::string taggerNames[2] = {"CSVv2", "deepCSV"};
    const std::string cleanNames[2] = {"uncleanded", "cleaned"};
    const std::string outputName = "bTaggEff_" + taggerNames[deepCSV] + "_" + wpNames[wp] + "_" + cleanNames[cleaned];
    //save histograms to file
    TFile* bTagEffFile = TFile::Open( (const TString&) "../weights/" + outputName, "recreate");
    //divide histograms to compute the efficiency and save to file
    for(unsigned flav = 0; flav < 3; ++flav){
        bTagEff[0][flav]->Divide(bTagEff[1][flav]);
        bTagEff[0][flav]->Write("bTagEff_" + quarkFlav[flav]);
    }
    bTagEffFile->Close();    
}

int main(){

    return 0;
}
