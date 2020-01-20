// include header
#include "../interface/eventCategorization.h"
// include constants (particle masses)
#include "../../../constants/particleMasses.h"

// event id variables
ULong_t _runNb;
ULong_t _lumiBlock;
ULong_t _eventNb;

// event BDT variables
Double_t _abs_eta_recoil;
Double_t _Mjj_max;
Double_t _lW_asymmetry;
Double_t _deepCSV_max;
Double_t _lT;
Double_t _MT;
Double_t _pTjj_max;
Double_t _dRlb_min;
Double_t _dPhill_max;
Double_t _HT;
UInt_t _nJets;
Double_t _dRlWrecoil;
Double_t _dRlWbtagged;
Double_t _M3l;
Double_t _abs_eta_max;

void initOutputTree(TTree* outputTree){
    // set branches for a flat output tree, to be used instead of ewkino/TreeReader/src/setOutputTree. 
    
    // event id variables
    outputTree->Branch("_runNb", &_runNb, "_runNb/l");
    outputTree->Branch("_lumiBlock", &_lumiBlock, "_lumiBlock/l");
    outputTree->Branch("_eventNb", &_eventNb, "_eventNb/l");
    
    // event BDT variables
    outputTree->Branch("_abs_eta_recoil", &_abs_eta_recoil, "_abs_eta_recoil/D");
    outputTree->Branch("_Mjj_max", &_Mjj_max, "_Mjj_max/D");
    outputTree->Branch("_lW_asymmetry", &_lW_asymmetry, "_lW_asymmetry/D");
    outputTree->Branch("_deepCSV_max", &_deepCSV_max, "_deepCSV_max/D");
    outputTree->Branch("_lT", &_lT, "_lT/D");
    outputTree->Branch("_MT", &_MT, "_MT/D");
    outputTree->Branch("_pTjj_max", &_pTjj_max, "_pTjj_max/D");
    outputTree->Branch("_dRlb_min", &_dRlb_min, "_dRlb_min/D");
    outputTree->Branch("_dPhill_max", &_dPhill_max, "_dPhill_max/D");
    outputTree->Branch("_HT", &_HT, "_HT/D");
    outputTree->Branch("_nJets", &_nJets, "_nJets/i");
    outputTree->Branch("_dRlWrecoil", &_dRlWrecoil, "_dRlWrecoil/D");
    outputTree->Branch("_dRlWbtagged", &_dRlWbtagged, "_dRlWbtagged/D");
    outputTree->Branch("_M3l", &_M3l, "_M3l/D");
    outputTree->Branch("_abs_eta_max", &_abs_eta_max, "_abs_eta_max/D");
} 

int eventCategory(Event& event){
    // determine the event category based on the number of jets and b-jets
    // note that it is assumed the event has been passed through a signal region selection!
    int njets = event.numberOfJets();
    int nbjets = event.numberOfMediumBTaggedJets();
    std::cout<<njets<<" "<<nbjets<<std::endl;
    if(nbjets == 0 or (nbjets==1 and njets==1)) return -1;
    if(nbjets == 1 and (njets==2 or njets==3)) return 1;
    if(nbjets == 1) return 2;
    return 3;
}

void entryFromEvent(Event& event){
    // fill one entry in outputTree (initialized with initOutputTree), based on the info of one event.
    // note that it is assumed the event has been passed through a signal region selection!
    // this is important for e.g. the number of leptons in the event.
    // later find a way to extend this to control regions; not enirely clear how this is done in the AN.
    
    // sort leptons and jets by pt, get b-jet collection
    event.sortJetsByPt();
    event.sortLeptonsByPt();
    JetCollection bjets = event.mediumBTagCollection();

    // event id variables 
    _runNb = event.runNumber();
    _lumiBlock = event.luminosityBlock();
    _eventNb = event.eventNumber();

    // other more or less precomputed event variables
    _lT = event.LT() + event.metPt();
    _HT = event.HT();
    _nJets = event.numberOfJets();
    
    // find lepton from W and set its properties
    int lWindex = event.WLeptonIndex();
    LeptonCollection::const_iterator lIt = event.leptonCollection().cbegin();
    for(int i=0; i<lWindex; i++){
        ++lIt;
    }
    Lepton& lW = **lIt;
    _lW_asymmetry = fabs(lW.eta())*lW.charge();

    /*std::cout<<"lW index: "<<lWindex<<std::endl;
    for(LeptonCollection::const_iterator lIt = event.leptonCollection().cbegin();
        lIt != event.leptonCollection().cend(); lIt++){
        Lepton& lep = **lIt;
        std::cout<<"lepton: "<<lep.charge()<<" "<<lep.isElectron()<<" "<<lep.isMuon()<<" "<<lep.isTau()<<std::endl;
    }*/
    
    // top reconstruction
    std::pair< double, double > pmz = pmzcandidates(event, lW);
    //std::cout<<"pmz: "<<pmz.first<<" "<<pmz.second<<std::endl;
    std::pair< double, int > topresults = besttopcandidate(event, lW, pmz.first, pmz.second, bjets);
    //std::cout<<"top mass: "<<topresults.first<<std::endl;
    //std::cout<<"b-tagged jet: "<<topresults.second<<std::endl;
    int btagindex = topresults.second;
    
    // find lepton indices from Z
    std::pair< int, int > zbosonresults = event.bestZBosonCandidateIndices();
    int lZ1index = zbosonresults.first; int lZ2index = zbosonresults.second;
    //std::cout<<"lZ1, lZ2, lW: "<<lZ1index<<" "<<lZ2index<<" "<<lWindex<<std::endl;

    // loop over jets and find relevant quantities
    _abs_eta_max = 0;
    _Mjj_max = 0;
    _pTjj_max = 0;
    _deepCSV_max = 0;
    int recoilindex = -1;
    for(JetCollection::const_iterator jIt = event.jetCollection().cbegin();
        jIt != event.jetCollection().cend(); jIt++){
	    Jet& jet = **jIt;
        // if abs(eta) is larger than max, modify max
        if(std::abs(jet.eta())>_abs_eta_max) _abs_eta_max = std::abs(jet.eta());
        // if jet is not equal to b-tagged jet and recoiling jet was not set yet, set it.
        if(recoilindex < 0 and jIt-event.jetCollection().cbegin() != btagindex){
            recoilindex=jIt-event.jetCollection().cbegin();
        }
        // if deepCSV is higher than maximum, modify max
        if(jet.deepCSV()>_deepCSV_max) _deepCSV_max = jet.deepCSV();
        // find maximum dijet mass and pt
        for(JetCollection::const_iterator jIt2 = jIt+1; jIt2 != event.jetCollection().cend(); jIt2++){
            Jet& jet2 = **jIt2;
            if((jet+jet2).mass()>_Mjj_max) _Mjj_max = (jet+jet2).mass();
            if((jet+jet2).pt()>_pTjj_max) _pTjj_max = (jet+jet2).pt();
        } 
    }
    
    // find b-tagged and recoiling jet from indices
    JetCollection::const_iterator jIt = event.jetCollection().cbegin();
    for(int i=0; i<recoilindex; i++) ++jIt;
    Jet& recoiljet = **jIt;
    jIt = event.jetCollection().cbegin();
    for(int i=0; i<btagindex; i++) ++jIt;
    Jet& btagjet = **jIt;
    
    // set angular separation between lW and b-tagged / recoiling jet
    _dRlWrecoil = deltaR(lW,recoiljet);
    _dRlWbtagged = deltaR(lW,btagjet);

    // loop over leptons and find some kinematic properties
    _dRlb_min = 99;
    _dPhill_max = 0;
    _M3l = 0;
    PhysicsObject l3vec;
    for(LeptonCollection::const_iterator lIt = event.leptonCollection().cbegin();
        lIt != event.leptonCollection().cend(); lIt++){
        Lepton& lep = **lIt;
        l3vec += lep;
        for(LeptonCollection::const_iterator lIt2 = lIt+1; lIt2 != event.leptonCollection().cend(); lIt2++){
            Lepton& lep2 = **lIt2;
            if(deltaPhi(lep,lep2)>_dPhill_max) _dPhill_max = deltaPhi(lep,lep2);
        }
        for(JetCollection::const_iterator jIt = bjets.cbegin(); jIt != bjets.cend(); jIt++){
            Jet& bjet = **jIt;
            if(deltaR(lep,bjet)<_dRlb_min) _dRlb_min = deltaR(lep,bjet);
        }
    }
    _M3l = l3vec.mass();
}

std::pair<double,double> pmzcandidates(Event& event, Lepton& lW){
    // this method returns two candidates for longitudinal component of missing momentum,
    // by imposing the W mass constraint on the system (lW,pmiss).
    
    // first define all four-vector quantities
    double El = lW.energy(); double plx = lW.px(); double ply = lW.py(); double plz = lW.pz();
    double pmx = event.met().px(); double pmy = event.met().py(); 
    double mW = particle::mW;
    std::pair<double,double> pmz = {0,0};
    
    /*// to test with W initially at rest
    double theta = 0.45; double phi = 1.7;
    double ml = 0.000511; double p = (mW*mW - ml*ml)/(2*mW);
    double El = std::sqrt(p*p+ml*ml); double plx = p*std::sin(theta)*std::cos(phi);
    double ply = p*std::sin(theta)*std::sin(phi); double plz = p*std::cos(theta);
    double pmx = -plx; double pmy = -ply;
    //std::cout<<"lepton: "<<El<<" "<<plx<<" "<<ply<<" "<<plz<<std::endl;
    //std::cout<<"ptmiss: "<<pmx<<" "<<pmy<<std::endl;
    // extension where W moves along x-axis
    double pWx = 80.; double gamma = std::sqrt(mW*mW+pWx*pWx)/mW; double beta = std::sqrt(1-1/(gamma*gamma));
    double plxnew = gamma*(plx+beta*El); double Elnew = gamma*(El+beta*plx);
    double pmxnew = gamma*(pmx+beta*p);
    El = Elnew; plx = plxnew; pmx = pmxnew;*/
    std::cout<<"lepton: "<<El<<" "<<plx<<" "<<ply<<" "<<plz<<std::endl;
    std::cout<<"ptmiss: "<<pmx<<" "<<pmy<<std::endl;
    
    // then solve quadratic equation
    double A = El*El - plz*plz;
    double B = -plz*(2*(plx*pmx+ply*pmy)+mW*mW);
    double C = El*El*(pmx*pmx+pmy*pmy) - std::pow(mW*mW/2,2) - std::pow(plx*pmx+ply*pmy,2);
    C += -mW*mW*(plx*pmx+ply*pmy);
    std::cout<<"equation: "<<A<<" "<<B<<" "<<C<<std::endl;
    double discr = B*B - 4*A*C;
    if(discr<0){
        std::cout<<"WARNING: missing pz reconstruction found negative discriminant."<<std::endl;
        std::cout<<"         returning parabola minimum as candidate..."<<std::endl;
        pmz.first = -B/(2*A);
        pmz.second = -B/(2*A);
        return pmz;
    }
    pmz.first = (-B + std::sqrt(discr))/(2*A);
    pmz.second = (-B - std::sqrt(discr))/(2*A);
    return pmz;
}

std::pair<double,int> besttopcandidate(Event& event, Lepton& lW, double pmz1, double pmz2,
                                        JetCollection bjets){
    // this method returns the reconstruced top mass closest to its nominal mass,
    // by combining the four-vectors of the lepton from W, the two missing momentum candidates,
    // and all b-jets in the event. Also the index of the b-jet in its collection is returned.
    
    std::pair<double,int> res = {0.,0};
    double bestmass = 0.;
    double massdiff = fabs(particle::mTop-bestmass);
    int bindex = -1;  
    double metpx = event.met().px();
    double metpy = event.met().py();  
    LorentzVector nu1;
    nu1.setPxPyPzE(metpx,metpy,pmz1,std::sqrt(metpx*metpx+metpy*metpy+pmz1*pmz1));
    LorentzVector nu2;
    nu2.setPxPyPzE(metpx,metpy,pmz2,std::sqrt(metpx*metpx+metpy*metpy+pmz2*pmz2));
    LorentzVector lep;
    lep.setPxPyPzE(lW.px(),lW.py(),lW.pz(),lW.energy());
    LorentzVector jet;
    double mass = 0.;
    for(JetCollection::const_iterator jIt = bjets.cbegin(); jIt != bjets.cend(); jIt++){
        Jet& jetobject = **jIt;
        jet.setPxPyPzE(jetobject.px(),jetobject.py(),jetobject.pz(),jetobject.energy());
        mass = (nu1 + lep + jet).mass();
        if(fabs(particle::mTop-mass)<massdiff){
            bestmass = mass;
            massdiff = fabs(particle::mTop-mass);
            bindex = jIt - bjets.cbegin();
        }
        mass = (nu2 + lep + jet).mass();
        if(fabs(particle::mTop-mass)<massdiff){
            bestmass = mass;
            massdiff = fabs(particle::mTop-mass);
            bindex = jIt - bjets.cbegin();
        }
    }
    res.first = bestmass;
    res.second = bindex;
    return res;   
}   
