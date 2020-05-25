// include header
#include "../interface/eventFlattening.h"
// include constants (particle masses)
#include "../../../constants/particleMasses.h"

// event id variables
ULong_t _runNb = 0;
ULong_t _lumiBlock = 0;
ULong_t _eventNb = 0;

// event weight for simulation
Float_t _weight = 0;
Float_t _normweight = 0;

// event BDT variables
Float_t _abs_eta_recoil = 0;
Float_t _Mjj_max = 0;
Float_t _lW_asymmetry = 0;
Float_t _deepCSV_max = 0;
Float_t _lT = 0;
Float_t _MT = 0;
Float_t _pTjj_max = 0;
Float_t _dRlb_min = 0;
Float_t _dPhill_max = 0;
Float_t _HT = 0;
Float_t _nJets = 0;
Float_t _nBJets = 0;
Float_t _dRlWrecoil = 0;
Float_t _dRlWbtagged = 0;
Float_t _M3l = 0;
Float_t _abs_eta_max = 0;

// BDT output score
Float_t _eventBDT = 0.;

// other variables
Int_t _nMuons = 0;
Int_t _nElectrons = 0;

void initOutputTree(TTree* outputTree){
    // set branches for a flat output tree, to be used instead of ewkino/TreeReader/src/setOutputTree. 
    
    // event id variables
    outputTree->Branch("_runNb", &_runNb, "_runNb/l");
    outputTree->Branch("_lumiBlock", &_lumiBlock, "_lumiBlock/l");
    outputTree->Branch("_eventNb", &_eventNb, "_eventNb/l");

    // event weight for simulation (fill with ones for data)
    outputTree->Branch("_weight", &_weight, "_weight/F");
    outputTree->Branch("_normweight", &_normweight, "_normweight/F");
    
    // event BDT variables
    outputTree->Branch("_abs_eta_recoil", &_abs_eta_recoil, "_abs_eta_recoil/F");
    outputTree->Branch("_Mjj_max", &_Mjj_max, "_Mjj_max/F");
    outputTree->Branch("_lW_asymmetry", &_lW_asymmetry, "_lW_asymmetry/F");
    outputTree->Branch("_deepCSV_max", &_deepCSV_max, "_deepCSV_max/F");
    outputTree->Branch("_lT", &_lT, "_lT/F");
    outputTree->Branch("_MT", &_MT, "_MT/F");
    outputTree->Branch("_pTjj_max", &_pTjj_max, "_pTjj_max/F");
    outputTree->Branch("_dRlb_min", &_dRlb_min, "_dRlb_min/F");
    outputTree->Branch("_dPhill_max", &_dPhill_max, "_dPhill_max/F");
    outputTree->Branch("_HT", &_HT, "_HT/F");
    outputTree->Branch("_nJets", &_nJets, "_nJets/F");
    outputTree->Branch("_nBJets", &_nBJets, "_nBJets/F");
    outputTree->Branch("_dRlWrecoil", &_dRlWrecoil, "_dRlWrecoil/F");
    outputTree->Branch("_dRlWbtagged", &_dRlWbtagged, "_dRlWbtagged/F");
    outputTree->Branch("_M3l", &_M3l, "_M3l/F");
    outputTree->Branch("_abs_eta_max", &_abs_eta_max, "_abs_eta_max/F");

    // BDT output score (initialized here but filled in calling function!)
    outputTree->Branch("_eventBDT", &_eventBDT, "_eventBDT/F");

    // other variables
    outputTree->Branch("_nMuons", &_nMuons, "_nMuons/I");
    outputTree->Branch("_nElectrons", &_nElectrons, "_nElectrons/I");
}

TMVA::Reader* initializeReader( TMVA::Reader* reader, const std::string& pathToXMLFile ){
    // make sure it is consistent with bdt training!
    reader->AddVariable("_abs_eta_recoil", &_abs_eta_recoil);
    reader->AddVariable("_Mjj_max", &_Mjj_max);
    reader->AddVariable("_lW_asymmetry", &_lW_asymmetry);
    reader->AddVariable("_deepCSV_max",&_deepCSV_max);
    reader->AddVariable("_lT", &_lT);
    reader->AddVariable("_MT", &_MT);
    reader->AddVariable("_dPhill_max", &_dPhill_max);
    reader->AddVariable("_pTjj_max", &_pTjj_max);
    reader->AddVariable("_dRlb_min", &_dRlb_min);
    reader->AddVariable("_HT", &_HT);
    reader->AddVariable("_dRlWrecoil", &_dRlWrecoil);
    reader->AddVariable("_dRlWbtagged", &_dRlWbtagged);
    reader->AddVariable("_M3l", &_M3l);
    reader->AddVariable("_abs_eta_max", &_abs_eta_max);
    reader->AddVariable("_nJets", &_nJets);
    reader->AddVariable("_nBJets", &_nBJets);

    reader->BookMVA("BDT", pathToXMLFile);
    return reader;
}

// help functions for getting the right jet collection and MET //

JetCollection getjetcollection(const Event& event, const std::string& variation){
    if( variation == "nominal" ){
        return event.jetCollection().goodJetCollection();
    } else if( variation == "JECDown" ){
        return event.jetCollection().JECDownCollection().goodJetCollection();
    } else if( variation == "JECUp" ){
        return event.jetCollection().JECUpCollection().goodJetCollection();
    } else if( variation == "JERDown" ){
        return event.jetCollection().JERDownCollection().goodJetCollection();
    } else if( variation == "JERUp" ){
        return event.jetCollection().JERUpCollection().goodJetCollection();
    } else if( variation == "UnclDown" ){
        return event.jetCollection().goodJetCollection();
    } else if( variation == "UnclUp" ){
        return event.jetCollection().goodJetCollection();
    } else {
        throw std::invalid_argument( "Uncertainty source " + variation + " is unknown." );
    }
}

Met getmet(const Event& event, const std::string& variation){
    if( variation == "nominal" ){
        return event.met();
    } else if( variation == "JECDown" ){
        return event.met().MetJECDown();
    } else if( variation == "JECUp" ){
        return event.met().MetJECUp();
    } else if( variation == "JERDown" ){
        return event.met();
    } else if( variation == "JERUp" ){
        return event.met();
    } else if( variation == "UnclDown" ){
        return event.met().MetUnclusteredDown();
    } else if( variation == "UnclUp" ){
        return event.met().MetUnclusteredUp();
    } else {
        throw std::invalid_argument( "Uncertainty source " + variation + " is unknown." );
    }
}

int eventCategory(Event& event, const std::string& variation){
    // determine the event category based on the number of jets and b-jets
    // note that it is assumed the event has been passed through a signal region selection!

    JetCollection jetc = getjetcollection(event,variation);
    int njets = jetc.size();
    int nbjets = jetc.numberOfMediumBTaggedJets();
    if(nbjets == 0 or (nbjets==1 and njets==1)) return -1;
    if(nbjets == 1 and (njets==2 or njets==3)) return 1;
    if(nbjets == 1) return 2;
    return 3;
}

// help functions for getting the fake rate map and fake rate weight // 

std::shared_ptr< TH2D > readFRMap( const std::string& pathToFile, 
			    const std::string& flavor, const std::string& year ){
    // note: this function was copied (with slight modifications)
    // from ewkino/AnalysisCode/fakerate/closureTest_MC.cc
    TFile* frFile = TFile::Open( pathToFile.c_str() );
    std::shared_ptr< TH2D > frMap( dynamic_cast< TH2D* >( frFile->Get( (
                            "fakeRate_" + flavor + "_" + year ).c_str() ) ) );
    frMap->SetDirectory( gROOT );
    frFile->Close();

    /*std::cout<<"values:"<<std::endl;
    for(unsigned xbin=1; xbin<=5; ++xbin){
        for(unsigned ybin=1; ybin<=3; ++ybin){
            std::cout<<"bin: "<<xbin<<" "<<ybin<<std::endl;
            std::cout<<frMap->GetBinContent(xbin,ybin)<<std::endl;
        }
    }*/

    return frMap;
}

double fakeRateWeight( const Event& event,
			const std::shared_ptr< TH2D >& frMap_muon,
                        const std::shared_ptr< TH2D >& frMap_electron ){
    // note: this function was copied (with slight modifications)
    // from ewkino/AnalysisCode/fakerate/closureTest_MC.cc
    
    double weight = -1.;
    for( const auto& leptonPtr : event.lightLeptonCollection() ){
	if( !(leptonPtr->isFO() && !leptonPtr->isTight()) ) continue;
            
	double croppedPt = std::min( leptonPtr->pt(), 99. );
        double croppedAbsEta = std::min( leptonPtr->absEta(), (leptonPtr->isMuon() ? 2.4 : 2.5) );

        double fr;
        if( leptonPtr->isMuon() ){
            fr = frMap_muon->GetBinContent( frMap_muon->FindBin( croppedPt, croppedAbsEta ) );
        } else {
            fr = frMap_electron->GetBinContent( frMap_electron->FindBin( croppedPt, croppedAbsEta ) );
        }
        weight *= ( - fr / ( 1. - fr ) );
    }
    return weight;
}
 
// main function //

void eventToEntry(Event& event, const double norm,
		    const bool isdataforbackground, 
		    const std::shared_ptr< TH2D>& frMap_muon, 
		    const std::shared_ptr< TH2D>& frMap_electron,
		    const std::string& variation){
    // fill one entry in outputTree (initialized with initOutputTree), based on the info of one event.
    // Note that the event must be cleaned and processed by an event selection function first!

    //std::cout<<"------------"<<std::endl;    
    // sort leptons and jets by pt, get b-jet collection
    event.sortJetsByPt();
    event.sortLeptonsByPt();

    // event id variables 
    _runNb = event.runNumber();
    _lumiBlock = event.luminosityBlock();
    _eventNb = event.eventNumber();

    // event weight (note: 0 for data!)
    _weight = event.weight();
    _normweight = event.weight()*norm;

    // multiply weight by fake-rate if needed
    if(isdataforbackground){
	_normweight = 1*fakeRateWeight(event,frMap_muon,frMap_electron);
    }   

    // get correct jet collection and met (defined in eventSelections.cc!)
    JetCollection jetcollection = getjetcollection(event, variation);
    JetCollection bjetcollection = jetcollection.mediumBTagCollection();
    Met met = getmet(event, variation);
    // get lepton collection as well (warning: a lot of event methods work on this collection implicitly,
    // so changing the definition here is not enough to consistently use another collection of leptons!)
    LeptonCollection lepcollection = event.leptonCollection();

    // number of muons and electrons
    _nMuons = lepcollection.numberOfMuons();
    _nElectrons = lepcollection.numberOfElectrons();

    // other more or less precomputed event variables
    _lT = lepcollection.scalarPtSum() + met.pt();
    _HT = jetcollection.scalarPtSum();
    _nJets = jetcollection.size();
    _nBJets = bjetcollection.size();

    // set default values for when no OSSF light pair is present (in principle only noOSSF sideband)
    _MT = 0;
    int lWindex = 0;

    // set mT
    if(event.hasOSSFLightLeptonPair()){ _MT = event.mtW(); }
    //std::cout<<"checkpoint 1"<<std::endl;

    // find lepton from W and set its properties
    // (will this work on e.g. the ZZ control region where in principle no lepton from W is present?)
    if(event.hasOSSFLightLeptonPair()){ lWindex = event.WLeptonIndex(); }
    LeptonCollection::const_iterator lIt = lepcollection.cbegin();
    for(int i=0; i<lWindex; i++){++lIt;}
    Lepton& lW = **lIt;
    _lW_asymmetry = fabs(lW.eta())*lW.charge();
    //std::cout<<"checkpoint 2"<<std::endl;

    // find leptons from Z
    // NOT YET SAFE WHEN NO OSSF PAIR IS PRESENT
    //std::pair< int, int > zbosonresults = event.bestZBosonCandidateIndices();
    //int lZ1index = zbosonresults.first; int lZ2index = zbosonresults.second;
    //lIt = event.leptonCollection().cbegin();
    //for(int i=0; i<lZ1index; i++){++lIt;}
    //Lepton& lZ1 = **lIt;
    //lIt = event.leptonCollection().cbegin();
    //for(int i=0; i<lZ2index; i++){++lIt;}
    //Lepton& lZ2 = **lIt;
    //std::cout<<"___________"<<std::endl;
    //std::cout<<lW<<std::endl;
    //std::cout<<lZ1<<std::endl;
    //std::cout<<lZ2<<std::endl;
    //std::cout<<""<<std::endl;
    //std::cout<<"checkpoint 4"<<std::endl;
    
    // top reconstruction
    std::pair< double, double > pmz = pmzcandidates(lW, met);
    //std::cout<<"pmz: "<<pmz.first<<" "<<pmz.second<<std::endl;
    std::pair< double, int > topresults = besttopcandidate(jetcollection,
					    lW, met, pmz.first, pmz.second);
    //std::cout<<"top mass: "<<topresults.first<<std::endl;
    //std::cout<<"b-tagged jet: "<<topresults.second<<std::endl;
    int taggedbindex = topresults.second;
    if(jetcollection.numberOfMediumBTaggedJets()==0) taggedbindex = 0;
    //std::cout<<topresults.first<<std::endl;
    //std::cout<<"checkpoint 5"<<std::endl;

    // find index of recoiling jet
    int recoilindex = -1;
    for(JetCollection::const_iterator jIt = jetcollection.cbegin();
	    jIt != jetcollection.cend(); jIt++){
	if(jIt-jetcollection.cbegin() != taggedbindex){
	    recoilindex = jIt-jetcollection.cbegin();
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
    for(JetCollection::const_iterator jIt = jetcollection.cbegin();
        jIt != jetcollection.cend(); jIt++){
	Jet& jet = **jIt;
        // if abs(eta) is larger than max, modify max
        if(fabs(jet.eta())>_abs_eta_max) _abs_eta_max = fabs(jet.eta());
        // if deepCSV is higher than maximum, modify max
        if(jet.deepCSV()>_deepCSV_max and jet.inBTagAcceptance()) _deepCSV_max = jet.deepCSV();
        // find maximum dijet mass and pt
        for(JetCollection::const_iterator jIt2 = jIt+1; jIt2 != jetcollection.cend(); jIt2++){
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
    if(recoilindex>=0 and jetcollection.size()>0){
	JetCollection::const_iterator jIt = jetcollection.cbegin();
	for(int i=0; i<recoilindex; i++){jIt++;}
	Jet& recoiljet = **jIt;
	_dRlWrecoil = deltaR(lW,recoiljet);
	_abs_eta_recoil = fabs(recoiljet.eta());
    }
    if(taggedbindex>=0 and jetcollection.size()>0){
	JetCollection::const_iterator tbjIt = jetcollection.cbegin();
	for(int i=0; i<taggedbindex; i++){tbjIt++;}
	Jet& taggedbjet = **tbjIt;
	_dRlWbtagged = deltaR(lW,taggedbjet);
    }
    //std::cout<<"checkpoint 8"<<std::endl;

    // loop over leptons and find some kinematic properties
    _dRlb_min = 99;
    _dPhill_max = 0;
    PhysicsObject l3vec;
    for(LeptonCollection::const_iterator lIt = lepcollection.cbegin();
        lIt != lepcollection.cend(); lIt++){
        Lepton& lep = **lIt;
        for(LeptonCollection::const_iterator lIt2 = lIt+1; lIt2 != lepcollection.cend(); lIt2++){
            Lepton& lep2 = **lIt2;
            if(deltaPhi(lep,lep2)>_dPhill_max) _dPhill_max = deltaPhi(lep,lep2);
        }
        for(JetCollection::const_iterator jIt = bjetcollection.cbegin(); jIt != bjetcollection.cend(); jIt++){
            Jet& bjet = **jIt;
            if(deltaR(lep,bjet)<_dRlb_min) _dRlb_min = deltaR(lep,bjet);
        }
    }
    _M3l = event.leptonSystem().mass();
    //std::cout<<"checkpoint 9"<<std::endl;
}

std::pair<double,double> pmzcandidates(Lepton& lW, Met& met){
    // this method returns two candidates for longitudinal component of missing momentum,
    // by imposing the W mass constraint on the system (lW,pmiss).
    
    // first define all four-vector quantities
    double El = lW.energy(); double plx = lW.px(); double ply = lW.py(); double plz = lW.pz();
    double pmx = met.px(); double pmy = met.py(); 
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

std::pair<double,int> besttopcandidate(JetCollection& alljets, Lepton& lW, 
					Met& met, double pmz1, double pmz2){
    // This method returns the reconstruced top mass closest to its nominal mass,
    // by combining the four-vectors of the lepton from W, the two missing momentum candidates,
    // and all medium b-jets in the event.
    // The index of the jet that gives the best top mass is returned as well.
    
    std::pair<double,int> res = {0.,0};
    double bestmass = 0.;
    double massdiff = fabs(particle::mTop-bestmass);
    int bindex = -1;
    // set lorentz vectors for lepton and neutrino
    double metpx = met.px();
    double metpy = met.py();  
    LorentzVector nu1;
    nu1.setPxPyPzE(metpx,metpy,pmz1,std::sqrt(metpx*metpx+metpy*metpy+pmz1*pmz1));
    LorentzVector nu2;
    nu2.setPxPyPzE(metpx,metpy,pmz2,std::sqrt(metpx*metpx+metpy*metpy+pmz2*pmz2));
    LorentzVector lep;
    lep.setPxPyPzE(lW.px(),lW.py(),lW.pz(),lW.energy());
    // loop over jets
    LorentzVector jet;
    double mass = 0.;
    for(JetCollection::const_iterator jIt = alljets.cbegin(); jIt != alljets.cend(); jIt++){
        Jet& jetobject = **jIt;
	// consider only medium tagged b-jets
	if(!jetobject.isBTaggedMedium()) continue;
        jet.setPxPyPzE(jetobject.px(),jetobject.py(),jetobject.pz(),jetobject.energy());
	// try neutrino hypothesis one
        mass = (nu1 + lep + jet).mass();
        if(fabs(particle::mTop-mass)<massdiff){
            bestmass = mass;
            massdiff = fabs(particle::mTop-mass);
            bindex = jIt - alljets.cbegin();
        }
	// try neutrino hypothesis two
        mass = (nu2 + lep + jet).mass();
        if(fabs(particle::mTop-mass)<massdiff){
            bestmass = mass;
            massdiff = fabs(particle::mTop-mass);
            bindex = jIt - alljets.cbegin();
        }
    }
    res.first = bestmass;
    res.second = bindex;
    return res;   
}   
