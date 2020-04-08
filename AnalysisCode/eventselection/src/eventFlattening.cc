// include header
#include "../interface/eventFlattening.h"
// include constants (particle masses)
#include "../../../constants/particleMasses.h"

// event id variables
ULong_t _runNb;
ULong_t _lumiBlock;
ULong_t _eventNb;

// event weight for simulation
Double_t _weight;
Double_t _normweight;

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
UInt_t _nBJets;
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

    // event weight for simulation (fill with ones for data)
    outputTree->Branch("_weight", &_weight, "_weight/D");
    outputTree->Branch("_normweight", &_normweight, "_normweight/D");
    
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
    outputTree->Branch("_nBJets", &_nBJets, "_nBJets/i");
    outputTree->Branch("_dRlWrecoil", &_dRlWrecoil, "_dRlWrecoil/D");
    outputTree->Branch("_dRlWbtagged", &_dRlWbtagged, "_dRlWbtagged/D");
    outputTree->Branch("_M3l", &_M3l, "_M3l/D");
    outputTree->Branch("_abs_eta_max", &_abs_eta_max, "_abs_eta_max/D");
}
 
void eventToEntry(Event& event, double norm){
    // fill one entry in outputTree (initialized with initOutputTree), based on the info of one event.
    // Note that the event must be cleaned and processed by an event selection function first!

    //std::cout<<"------------"<<std::endl;    
    // sort leptons and jets by pt, get b-jet collection
    event.sortJetsByPt();
    event.sortLeptonsByPt();

    for(LeptonCollection::const_iterator lIt = event.leptonCollection().cbegin();
	    lIt!=event.leptonCollection().cend(); lIt++){
	Lepton& l = **lIt;
	//std::cout<<l<<std::endl;
	if(l.isElectron()) std::cout<<"found electron"<<std::endl;
    }

    // event id variables 
    _runNb = event.runNumber();
    _lumiBlock = event.luminosityBlock();
    _eventNb = event.eventNumber();

    // event weight
    _weight = event.weight();
    _normweight = event.weight()*norm;

    // other more or less precomputed event variables
    _lT = event.LT() + event.metPt();
    _HT = event.HT();
    _nJets = event.numberOfJets();
    _nBJets = event.numberOfMediumBTaggedJets();
    _MT = event.mtW();
    //std::cout<<"checkpoint 1"<<std::endl;

    // find lepton from W and set its properties
    // (will this work on e.g. the ZZ control region where in principle no lepton from W is present?)
    int lWindex = event.WLeptonIndex();
    LeptonCollection::const_iterator lIt = event.leptonCollection().cbegin();
    for(int i=0; i<lWindex; i++){++lIt;}
    Lepton& lW = **lIt;
    _lW_asymmetry = fabs(lW.eta())*lW.charge();
    //std::cout<<"checkpoint 2"<<std::endl;

    // find leptons from Z
    std::pair< int, int > zbosonresults = event.bestZBosonCandidateIndices();
    int lZ1index = zbosonresults.first; int lZ2index = zbosonresults.second;
    lIt = event.leptonCollection().cbegin();
    for(int i=0; i<lZ1index; i++){++lIt;}
    Lepton& lZ1 = **lIt;
    lIt = event.leptonCollection().cbegin();
    for(int i=0; i<lZ2index; i++){++lIt;}
    Lepton& lZ2 = **lIt;
    //std::cout<<"___________"<<std::endl;
    //std::cout<<lW<<std::endl;
    //std::cout<<lZ1<<std::endl;
    //std::cout<<lZ2<<std::endl;
    //std::cout<<""<<std::endl;
    //std::cout<<"checkpoint 4"<<std::endl;
    
    // top reconstruction
    std::pair< double, double > pmz = pmzcandidates(event, lW);
    //std::cout<<"pmz: "<<pmz.first<<" "<<pmz.second<<std::endl;
    std::pair< double, int > topresults = besttopcandidate(event, lW, pmz.first, pmz.second);
    //std::cout<<"top mass: "<<topresults.first<<std::endl;
    //std::cout<<"b-tagged jet: "<<topresults.second<<std::endl;
    int taggedbindex = topresults.second;
    if(event.numberOfMediumBTaggedJets()==0) taggedbindex = 0;
    //std::cout<<topresults.first<<std::endl;
    //std::cout<<"checkpoint 5"<<std::endl;

    // find index of recoiling jet
    int recoilindex = -1;
    for(JetCollection::const_iterator jIt = event.jetCollection().cbegin();
	    jIt != event.jetCollection().cend(); jIt++){
	if(jIt-event.jetCollection().cbegin() != taggedbindex){
	    recoilindex = jIt-event.jetCollection().cbegin();
	    break;
	}
    }
    //std::cout<<taggedbindex<<std::endl;
    //std::cout<<recoilindex<<std::endl;
    //std::cout<<"checkpoint 6"<<std::endl;

    // loop over jets and find relevant quantities
    _abs_eta_max = 0;
    _Mjj_max = 0;
    _pTjj_max = 0;
    _deepCSV_max = 0;
    for(JetCollection::const_iterator jIt = event.jetCollection().cbegin();
        jIt != event.jetCollection().cend(); jIt++){
	Jet& jet = **jIt;
        // if abs(eta) is larger than max, modify max
        if(fabs(jet.eta())>_abs_eta_max) _abs_eta_max = fabs(jet.eta());
        // if deepCSV is higher than maximum, modify max
        if(jet.deepCSV()>_deepCSV_max and jet.inBTagAcceptance()) _deepCSV_max = jet.deepCSV();
        // find maximum dijet mass and pt
        for(JetCollection::const_iterator jIt2 = jIt+1; jIt2 != event.jetCollection().cend(); jIt2++){
            Jet& jet2 = **jIt2;
            if((jet+jet2).mass()>_Mjj_max) _Mjj_max = (jet+jet2).mass();
            if((jet+jet2).pt()>_pTjj_max) _pTjj_max = (jet+jet2).pt();
        } 
    }
    //std::cout<<_deepCSV_max<<std::endl;
    //std::cout<<"checkpoint 7"<<std::endl;
    
    _dRlWrecoil = 0;
    _dRlWbtagged = 0;
    _abs_eta_recoil = 0;
    if(recoilindex>=0 and event.numberOfJets()>0){
	JetCollection::const_iterator jIt = event.jetCollection().cbegin();
	for(int i=0; i<recoilindex; i++){jIt++;}
	Jet& recoiljet = **jIt;
	_dRlWrecoil = deltaR(lW,recoiljet);
	_abs_eta_recoil = fabs(recoiljet.eta());
    }
    if(taggedbindex>=0 and event.numberOfJets()>0){
	JetCollection::const_iterator tbjIt = event.jetCollection().cbegin();
	for(int i=0; i<taggedbindex; i++){tbjIt++;}
	Jet& taggedbjet = **tbjIt;
	_dRlWbtagged = deltaR(lW,taggedbjet);
    }
    //std::cout<<"checkpoint 8"<<std::endl;

    // loop over leptons and find some kinematic properties
    _dRlb_min = 99;
    _dPhill_max = 0;
    PhysicsObject l3vec;
    JetCollection bjets = event.mediumBTagCollection();
    for(LeptonCollection::const_iterator lIt = event.leptonCollection().cbegin();
        lIt != event.leptonCollection().cend(); lIt++){
        Lepton& lep = **lIt;
        for(LeptonCollection::const_iterator lIt2 = lIt+1; lIt2 != event.leptonCollection().cend(); lIt2++){
            Lepton& lep2 = **lIt2;
            if(deltaPhi(lep,lep2)>_dPhill_max) _dPhill_max = deltaPhi(lep,lep2);
        }
        for(JetCollection::const_iterator jIt = bjets.cbegin(); jIt != bjets.cend(); jIt++){
            Jet& bjet = **jIt;
            if(deltaR(lep,bjet)<_dRlb_min) _dRlb_min = deltaR(lep,bjet);
        }
    }
    _M3l = event.leptonSystem().mass();
    //std::cout<<"checkpoint 9"<<std::endl;
}

std::pair<double,double> pmzcandidates(Event& event, Lepton& lW){
    // this method returns two candidates for longitudinal component of missing momentum,
    // by imposing the W mass constraint on the system (lW,pmiss).
    
    // first define all four-vector quantities
    double El = lW.energy(); double plx = lW.px(); double ply = lW.py(); double plz = lW.pz();
    double pmx = event.met().px(); double pmy = event.met().py(); 
    double mW = particle::mW;
    std::pair<double,double> pmz = {0,0};
    
    //std::cout<<"lepton: "<<El<<" "<<plx<<" "<<ply<<" "<<plz<<std::endl;
    //std::cout<<"ptmiss: "<<pmx<<" "<<pmy<<std::endl;
    
    // then solve quadratic equation
    double A = El*El - plz*plz;
    double B = -plz*(2*(plx*pmx+ply*pmy)+mW*mW);
    double C = El*El*(pmx*pmx+pmy*pmy) - std::pow(mW*mW/2,2) - std::pow(plx*pmx+ply*pmy,2);
    C += -mW*mW*(plx*pmx+ply*pmy);
    double discr = B*B - 4*A*C;
    if(discr<0){
        //std::cout<<"### WARNING ###: negative discriminant found."<<std::endl;
        discr = 0;
    }
    pmz.first = (-B + std::sqrt(discr))/(2*A);
    pmz.second = (-B - std::sqrt(discr))/(2*A);
    return pmz;
}

// Willems equivalent method:
/*std::pair<double,double> pmzcandidates(Event& event, Lepton& lW){
    double pl = lW.energy(); double plt = lW.pt(); double plz = lW.pz();
    double pmt = event.met().pt();
    double mW = particle::mW;
    std::pair<double,double> pmz = {0,0};

    double a = 0.5*mW*mW + lW.px()*event.met().px() + lW.py()*event.met().py();
    double D = 1-plt*plt*pmt*pmt/(a*a);
    double sqrtD = 0;
    if(D>0) sqrtD = std::sqrt(D);
    else std::cout<<"### WARNING ###"<<std::endl;
    pmz.first = a*(plz + pl*sqrtD)/(plt*plt);
    pmz.second = a*(plz - pl*sqrtD)/(plt*plt);
    std::cout<<pmz.first<<std::endl;
    std::cout<<pmz.second<<std::endl;
    return pmz;
}*/

std::pair<double,int> besttopcandidate(Event& event, Lepton& lW, double pmz1, double pmz2){
    // this method returns the reconstruced top mass closest to its nominal mass,
    // by combining the four-vectors of the lepton from W, the two missing momentum candidates,
    // and all medium b-jets in the event.
    
    std::pair<double,int> res = {0.,0};
    double bestmass = 0.;
    double massdiff = fabs(particle::mTop-bestmass);
    int bindex = -1;
    // set lorentz vectors for lepton and neutrino
    double metpx = event.met().px();
    double metpy = event.met().py();  
    LorentzVector nu1;
    nu1.setPxPyPzE(metpx,metpy,pmz1,std::sqrt(metpx*metpx+metpy*metpy+pmz1*pmz1));
    LorentzVector nu2;
    nu2.setPxPyPzE(metpx,metpy,pmz2,std::sqrt(metpx*metpx+metpy*metpy+pmz2*pmz2));
    LorentzVector lep;
    lep.setPxPyPzE(lW.px(),lW.py(),lW.pz(),lW.energy());
    // loop over jets
    LorentzVector jet;
    double mass = 0.;
    for(JetCollection::const_iterator jIt = event.jetCollection().cbegin(); 
	    jIt != event.jetCollection().cend(); jIt++){
        Jet& jetobject = **jIt;
	// only consdider medium b-tagged jets
	if(!jetobject.isBTaggedMedium()) continue;
        jet.setPxPyPzE(jetobject.px(),jetobject.py(),jetobject.pz(),jetobject.energy());
	// try neutrino hypothesis one
        mass = (nu1 + lep + jet).mass();
        if(fabs(particle::mTop-mass)<massdiff){
            bestmass = mass;
            massdiff = fabs(particle::mTop-mass);
            bindex = jIt - event.jetCollection().cbegin();
        }
	// try neutrino hypothesis two
        mass = (nu2 + lep + jet).mass();
        if(fabs(particle::mTop-mass)<massdiff){
            bestmass = mass;
            massdiff = fabs(particle::mTop-mass);
            bindex = jIt - event.jetCollection().cbegin();
        }
    }
    res.first = bestmass;
    res.second = bindex;
    return res;   
}   
