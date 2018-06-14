
//include c++ library classes
#include <fstream>

//include other parts of code
#include "../interface/treeReader.h"
#include "../interface/analysisTools.h"
#include "../interface/kinematicTools.h"

void treeReader::computeBTagEff(const std::string& analysis, const bool clean, const bool deepCSV, const bool is2016){

    //read sample list for computing btagging efficiency
    std::string sampleList;
    if( is2016 ){
        sampleList = "samples_bTagEff_2016.txt";
    } else {
        sampleList = "samples_bTagEff_2017.txt";
    }
    readSamples("sampleLists/" + sampleList);

    //make 2D b-tagging efficiency histograms for all jet flavors
    std::shared_ptr<TH2D> bTagEff[2][3][3]; //2: num and denom 3: udsg, c, b
    const double ptBins[17] = {25,30,35,40,45,50,60,70,80,90,100,120,150,200,300,400,600};
    const double etaBins[7] = {0, 0.4, 0.8, 1.2, 1.6, 2.0, 2.4};
    const std::string denNum[2] = {"numerator", "denominator"};
    const std::string quarkFlav[3] = {"udsg", "charm", "beauty"};
    const std::string wpNames[3] = {"loose", "medium", "tight"};
    for(unsigned wp = 0; wp < 3; ++wp){
        for(unsigned i = 0; i < 2; ++i){
            for(unsigned flav = 0; flav < 3; ++flav){
                bTagEff[i][flav][wp] = std::make_shared<TH2D>((const TString&) "bTagEff" + "_" + wpNames[wp] + "_" + quarkFlav[flav] + "_" + denNum[i], (const TString&) "bTagEff" + "_" + wpNames[wp] + "_" + quarkFlav[flav] + "_" + denNum[i] + ";" + "P_{T}(jet) (GeV)" + "; |#eta(jet)|", 16, ptBins, 6, etaBins);
                bTagEff[i][flav][wp]->Sumw2();
            }
        }
    }

    //loop over all samples 
    for(auto& sam : samples){
        initSample(); 
        std::cout<<"Entries in "<< sam.getFileName() << " " << nEntries << std::endl;
        for(long unsigned it = 0; it < nEntries; ++it){

            GetEntry(sam, it);

            //make all weights equal to 1 to get maximum statistics
            if(weight > 1){
                weight = 1;
            } else if(weight < 1){
                weight = -1;
            } else {
                std::cerr << "Error: weight of event is zero!" << std::endl;
            }

            //loop over jets 
            for(unsigned j = 0; j < _nJets; ++j){

                //separately apply cleaning (not as an argument to jetIsGood) to allow for different lepton ID cleaning 
                bool cleanJet = true;
                if(clean){
                    for(unsigned l = 0; l < _nLight; ++l){
                        if( lepIsGoodMultiAnalysis(analysis, l) ){
                            double deltaR = kinematics::deltaR(_lPhi[l], _lEta[l], _jetPhi[j], _jetEta[j]);
                            if(deltaR < 0.4){
                                cleanJet = false;
                                break;
                            }
                        }
                    }
                }

                //only consider jets passing loose selection 
                if(cleanJet && jetIsGood(j, 25., 0, false) && fabs(_jetEta[j]) < 2.4){
                    if(_jetHadronFlavor[j] == 0 || _jetHadronFlavor[j] == 4 || _jetHadronFlavor[j] == 5){
                        unsigned flav = 0 + (_jetHadronFlavor[j] == 4) + 2*(_jetHadronFlavor[j] == 5);

                        //loop over all working points (loose, medium, tight)
                        for(unsigned wp = 0; wp < 3; ++wp){

                            //fill denominator
                            if(bTagged(j, wp, deepCSV)){
                                bTagEff[0][flav][wp]->Fill(std::min(_jetPt[j], 599.), std::min( fabs(_jetEta[j]), 2.4), weight);
                            }

                            //fill numerator
                            bTagEff[1][flav][wp]->Fill(std::min(_jetPt[j], 599.), std::min( fabs(_jetEta[j]), 2.4), weight);
                        }
                    }
                }
            }
        }
    }

    //determine name of outputfile
    const std::string taggerNames[2] = {"CSVv2", "deepCSV"};
    const std::string cleanNames[2] = {"uncleanded", "cleaned"};
    const std::string outputName = "bTagEff_" + taggerNames[deepCSV] + "_" + cleanNames[clean] + "_" + analysis + "_" + (is2016 ? "2016" : "2017"); 

    //save histograms to file
    TFile* bTagEffFile = TFile::Open( (const TString&) "weights/" + outputName + ".root", "recreate");

    //divide histograms to compute the efficiency and save to file
    for(unsigned wp = 0; wp < 3; ++wp){
        for(unsigned flav = 0; flav < 3; ++flav){
            bTagEff[0][flav][wp]->Divide(bTagEff[1][flav][wp].get());
            bTagEff[0][flav][wp]->Write((const TString&) "bTagEff_" + wpNames[wp] + quarkFlav[flav]);
        }
    }
    bTagEffFile->Close();    
}

int main(int argc, char* argv[]){

    //convert all input to std::string format for easier handling
    std::vector<std::string> argvStr;
    for(int i = 0; i < argc; ++i){
        argvStr.push_back(std::string(argv[i]));
    }

    if(argc == 1){
        const std::vector< std::string> years = {"2016", "2017"};
        const std::vector< std::string> analyses = {"tZq", "ttZ3l", "ttZ4l", "ttW"}; 

        for(auto & year : years){
            for(auto& analysis : analyses){
                std::string scriptName = ( "bTagEff_" + year + "_" + analysis + ".sh" );
                std::ofstream submitScript(scriptName);
                tools::initScript(submitScript);
                submitScript << ( "./bTagEff " + analysis + " clean deepCSV " + year ); 
                submitScript.close();
                tools::submitScript(scriptName, "100:00:00");
                tools::deleteFile(scriptName);
            }
        }
    } else if(argc == 5){
        
        //catch wrong arguments given to code
        bool wrongArguments = false;
        if( ! (argvStr[1] == "tZq" || argvStr[1] == "ttZ3l" ||  argvStr[1] == "ttZ4l" || argvStr[1] == "ttW") ){
            wrongArguments = true;
        }
        if( ! (argvStr[2] == "clean" || argvStr[2] == "unclean") ){
            wrongArguments = true;
        }
        if( ! (argvStr[3] == "deepCSV" || argvStr[3] == "CSVv2") ){
            wrongArguments = true;
        }
        if( ! (argvStr[4] == "2016" || argvStr[4] == "2017") ){
            wrongArguments = true;
        }
        if( wrongArguments ){
            std::cerr << "Error: unrecognized arguments given to bTagEff program. Aborting." << std::endl;
            std::cerr << "Give in arguments as follows : <tZq, ttZ3l, ttZ4l, ttW> <clean,unclean> <deepCSV, CSVv2> <2016, 2017>" << std::endl;
            return 1;
        }

        bool clean = (argvStr[2] == "clean");
        bool deepCsv = (argvStr[3] == "deepCSV");
        bool is2016 = (argvStr[4] == "2016");
        
        treeReader reader;
        reader.computeBTagEff(argvStr[1], clean, deepCsv, is2016);
    }
    return 0;
}
