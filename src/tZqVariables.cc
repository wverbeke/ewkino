
//include other parts of code
#include "../interface/treeReader.h"
#include "../interface/tZqTools.h"
#include "../interface/kinematicTools.h"


unsigned treeReader::setSearchVariablestZq(const std::string& uncertainty, const std::vector<unsigned>& ind, const std::pair<unsigned, unsigned>& bestZ, const bool isSearchRegion){

    static const std::map< std::string, unsigned > uncertaintyArguments = 
    {
        {"nominal", 0},
        {"JECDown", 1},
        {"JECUp", 2},
        {"unclDown", 0},
        {"unclUp", 0}
    };

    //check if uncertainty argument exists 
    if( uncertaintyArguments.find(uncertainty) == uncertaintyArguments.cend() ){
        std::cerr << "Error: can not set tZq search variables for unknown uncertainty source : " << uncertainty << ". Returning control." << std::endl;
        return 99;
    }

    //make ordered jet and bjet collections, indices represent selected jets 
    std::vector<unsigned> jetInd, bJetInd;
    unsigned jetCount = nJets(jetInd, (*uncertaintyArguments.find(uncertainty)).second );
    unsigned bJetCount = nBJets(bJetInd, (*uncertaintyArguments.find(uncertainty)).second );

    //Determine tZq analysis category
    unsigned tzqCat = tzq::cat(jetCount, bJetCount);

    //don't waste resources on events outside the search categories
    if( isSearchRegion && (tzqCat < 3 || tzqCat > 5) ) return tzqCat;

    //find highest eta jet
    unsigned highestEtaJ = (jetCount == 0) ? 99 : jetInd[0];
    for(unsigned j = 1; j < jetCount; ++j){
        if(fabs(_jetEta[jetInd[j]]) > fabs(_jetEta[highestEtaJ]) ) highestEtaJ = jetInd[j];
    }

    //make LorentzVector for all jets 
    TLorentzVector jetV[(const unsigned) _nJets];
    for(unsigned j = 0; j < _nJets; ++j){
        if( uncertainty == "JECDown"){
            jetV[j].SetPtEtaPhiE(_jetPt_JECDown[j], _jetEta[j], _jetPhi[j], _jetE[j]*(_jetPt_JECDown[j]/_jetPt[j]) );
        } else if( uncertainty == "JECUp"){
            jetV[j].SetPtEtaPhiE(_jetPt_JECUp[j], _jetEta[j], _jetPhi[j], _jetE[j]*(_jetPt_JECUp[j]/_jetPt[j]));
        } else {
            jetV[j].SetPtEtaPhiE(_jetPt[j], _jetEta[j], _jetPhi[j], _jetE[j]);
        }
    }

    //set lCount, which is equal to the amount of lepton indices
    const unsigned lCount = ind.size();

    //find W lepton 
    unsigned lw = 99;
    for(unsigned l = 0; l < lCount; ++l){
        if( l != bestZ.first && l != bestZ.second ) lw = l;
    }

    //Catch shifted met values of inf, this was observed to occur in a ttZ event (1:32731:5662311)
    bool metJECDownIsInf = false;
    if( std::isinf( _metJECDown ) ){
        metJECDownIsInf = true;
    } 
    bool metJECUpIsInf = false;
    if( std::isinf( _metJECUp ) ){
        metJECUpIsInf = true;
    }
    if( metJECDownIsInf || metJECUpIsInf){
        
        printEventTags();
        std::cerr << "Error: " << 
            ( metJECDownIsInf ? "_metJECDown " : " ") << ( (metJECUpIsInf && metJECUpIsInf) ? "and" : "") << 
            (metJECUpIsInf ? "_metJECUp" : "") << ( (metJECUpIsInf && metJECUpIsInf) ? " are" : " is") << 
            " inf. Setting varied met and metPhi to nominal values." << std::endl;

        if(metJECDownIsInf){
            _metJECDown = _met;
            _metPhiJECDown = _metPhi;
        } 
        if(metJECUpIsInf){
            _metJECUp = _met;
            _metPhiJECDown = _metPhi;
        }
    }

    //make met vector 
    TLorentzVector met;
    if(uncertainty == "nominal"){
        met.SetPtEtaPhiE(_met, _metPhi, 0, _met);    
    } else if( uncertainty == "JECDown"){
        met.SetPtEtaPhiE(_metJECDown, _metPhiJECDown, 0, _metJECDown);
    } else if( uncertainty == "JECUp"){
        met.SetPtEtaPhiE(_metJECUp, _metPhiJECUp, 0, _metJECUp);
    } else if( uncertainty == "unclDown"){
        met.SetPtEtaPhiE(_metUnclDown, _metPhiUnclDown, 0, _metUnclDown);
    } else if( uncertainty == "unclUp"){
        met.SetPtEtaPhiE(_metUnclUp, _metPhiUnclUp, 0, _metUnclUp);
    }

    //make lorentzvectors for leptons
    TLorentzVector lepV[lCount];
    for(unsigned l = 0; l < lCount; ++l) lepV[l].SetPtEtaPhiE(_lPt[ind[l]], _lEta[ind[l]], _lPhi[ind[l]], _lE[ind[l]]);
 
    //reconstruct top mass and tag jets
    std::vector<unsigned> taggedJetI; //0 -> b jet from tZq, 1 -> forward recoiling jet
    TLorentzVector neutrino = tzq::findBestNeutrinoAndTop(lepV[lw], met, taggedJetI, jetInd, bJetInd, jetV);


    //find jets with highest DeepCSV and CSVv2 values
    unsigned highestDeepCSVI = 99;
    unsigned counter = 0;
    for(unsigned j = 0; j < jetCount; ++j){
        if(fabs(_jetEta[jetInd[j]]) < 2.4) {
            if( (counter == 0) || ( deepCSV(jetInd[j]) > deepCSV(highestDeepCSVI) ) ) highestDeepCSVI = jetInd[j];
            ++counter;
        }
    }

    //initialize new vectors to make sure everything is defined for 0 jet events!
    TLorentzVector highestEtaJet(0,0,0,0);
    TLorentzVector recoilingJet(0,0,0,0);
    TLorentzVector taggedBJet(0,0,0,0);
    TLorentzVector highestDeepCSVJet(0,0,0,0);
    if(taggedJetI[0] != 99) taggedBJet = jetV[taggedJetI[0]];
    if(taggedJetI[1] != 99) recoilingJet = jetV[taggedJetI[1]];
    if(jetCount != 0){
        highestEtaJet = jetV[highestEtaJ];
        if(highestDeepCSVI != 99) highestDeepCSVJet = jetV[highestDeepCSVI];
    }

    //compute top vector
    TLorentzVector topV = (neutrino + lepV[lw] + taggedBJet);

    //Compute minimum and maximum masses and separations for several objects
    //initialize lepton indices 
    std::vector<unsigned> lepVecInd;
    for(unsigned l = 0; l < lCount; ++l) lepVecInd.push_back(l);


    //lepton bjet
    double minDeltaPhiLeptonbJet = kinematics::minDeltaR(lepV, lepVecInd, jetV, bJetInd);

    //jet jet
    double maxMJetJet = kinematics::maxMass(jetV, jetInd);
    double maxDeltaPhiJetJet = kinematics::maxDeltaPhi(jetV, jetInd);
    double maxpTJetJet = kinematics::maxPT(jetV, jetInd);

    //lepton lepton 
    double maxDeltaPhiLeptonLepton = kinematics::maxDeltaPhi(lepV, lepVecInd);

    //compute HT
    double HT = 0;
    for(unsigned j = 0; j < jetCount; ++j){
        HT += _jetPt[jetInd[j]];
    }

    double LT = 0.;
    for(unsigned l = 0; l < lCount; ++l){
        LT += _lPt[ind[l]];
    }
    
    bdtVariableMap["etaRecoilingJet"] = fabs(recoilingJet.Eta());
    bdtVariableMap["maxMjj"] = std::max(maxMJetJet, 0.);
    bdtVariableMap["asymmetryWlep"] = fabs(_lEta[ind[lw]])*_lCharge[ind[lw]];
    bdtVariableMap["highestDeepCSV"] = (jetCount == 0 || highestDeepCSVI == 99) ? 0. : deepCSV(highestDeepCSVI);
    bdtVariableMap["ltmet"] = LT + _met;
    bdtVariableMap["maxDeltaPhijj"] = maxDeltaPhiJetJet;
    bdtVariableMap["mTW"] = kinematics::mt(lepV[lw], met);
    bdtVariableMap["topMass"] =  std::max(topV.M(), 0.);
    bdtVariableMap["pTMaxjj"] =  maxpTJetJet;
    bdtVariableMap["minDeltaPhilb"] = minDeltaPhiLeptonbJet;
    bdtVariableMap["maxDeltaPhill"] = maxDeltaPhiLeptonLepton;
    bdtVariableMap["ht"] = HT;
    bdtVariableMap["deltaRTaggedbJetRecoilingJet"] = taggedBJet.DeltaR(recoilingJet);
    bdtVariableMap["deltaRWLeptonTaggedbJet"] = lepV[lw].DeltaR(taggedBJet);
    bdtVariableMap["m3l"] = (lepV[0] + lepV[1] + lepV[2]).M();
    bdtVariableMap["etaMostForward"] = fabs(highestEtaJet.Eta());
    bdtVariableMap["numberOfJets"] = jetCount;

    //variables not used in BDT training, but that are nonetheless plotted 
    bdtVariableMap["numberOfbJets"] = bJetCount;

    return tzqCat;
}

double treeReader::bdtOutput( unsigned tzqCat ){
    double bdt = 0;
    if(tzqCat == 3){
        bdt = bdtReader1bJet23Jets->computeBDT(bdtVariableMap);
    } else if(tzqCat == 4){
        bdt = bdtReader1bJet4Jets->computeBDT(bdtVariableMap);
    } else if(tzqCat == 5){
        bdt = bdtReader2bJets->computeBDT(bdtVariableMap);
    } else {
        std::cerr << "Error: requesting tZq BDT value for category " << tzqCat << " for which no BDT is available." << std::endl;
        return 999.;
    }
    return bdt;
}
