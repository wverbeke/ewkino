/*
Custom analyzer class for finding D mesons decaying to three tracked particles.
*/
#include "heavyNeutrino/multilep/interface/DAnalyzer.h"

// constructor //
DAnalyzer::DAnalyzer(const edm::ParameterSet& iConfig, multilep* multilepAnalyzer):
    multilepAnalyzer(multilepAnalyzer){
};

// destructor //
DAnalyzer::~DAnalyzer(){
}

// beginJob //
void DAnalyzer::beginJob(TTree* outputTree){

    // beamspot and primary vertex variables
    outputTree->Branch("_primaryVertexX", &_primaryVertexX, "_primaryVertexX/D");
    outputTree->Branch("_primaryVertexY", &_primaryVertexY, "_primaryVertexY/D");
    outputTree->Branch("_primaryVertexZ", &_primaryVertexZ, "_primaryVertexZ/D");
    // D variables
    outputTree->Branch("_nDs", &_nDs, "_nDs/i");
    outputTree->Branch("_DInvMass", &_DInvMass, "_DInvMass[_nDs]/D");
    outputTree->Branch("_DType", &_DType, "_DType[_nDs]/i");
    outputTree->Branch("_DPx", &_DPx, "_DPx[_nDs]/D");
    outputTree->Branch("_DPy", &_DPy, "_DPy[_nDs]/D");
    outputTree->Branch("_DPz", &_DPz, "_DPz[_nDs]/D");
    outputTree->Branch("_DPt", &_DPt, "_DPt[_nDs]/D");
    outputTree->Branch("_DEta", &_DEta, "_DEta[_nDs]/D");
    outputTree->Branch("_DPhi", &_DPhi, "_DPhi[_nDs]/D");
    outputTree->Branch("_DE", &_DE, "_DE[_nDs]/D");
    outputTree->Branch("_DIsolation", &_DIsolation, "_DIsolation[_nDs]/D");
    outputTree->Branch("_DDR", &_DDR, "_DDR[_nDs]/D");
    outputTree->Branch("_DHasFastGenMatch", &_DHasFastGenMatch, "_DHasFastGenMatch[_nDs]/O");
    outputTree->Branch("_DHasGenMatch", &_DHasGenMatch, "_DHasGenMatch[_nDs]/O");
    // intermediate resonance variables
    outputTree->Branch("_DIntResMass", &_DIntResMass, "_DIntResMass[_nDs]/D");
    outputTree->Branch("_DIntResMassDiff", &_DIntResMassDiff, "_DIntResMassDiff[_nDs]/D");
    outputTree->Branch("_DIntResX", &_DIntResX, "_DIntResX[_nDs]/D");
    outputTree->Branch("_DIntResY", &_DIntResY, "_DIntResY[_nDs]/D");
    outputTree->Branch("_DIntResZ", &_DIntResZ, "_DIntResZ[_nDs]/D");
    outputTree->Branch("_DIntResVtxNormChi2", &_DIntResVtxNormChi2, "_DIntResVtxNormChi2[_nDs]/D");
    // track variables
    outputTree->Branch("_DFirstTrackPt", &_DFirstTrackPt, "_DFirstTrackPt[_nDs]/D");
    outputTree->Branch("_DFirstTrackEta", &_DFirstTrackEta, "_DFirstTrackEta[_nDs]/D");
    outputTree->Branch("_DFirstTrackPhi", &_DFirstTrackPhi, "_DFirstTrackPhi[_nDs]/D");
    outputTree->Branch("_DFirstTrackP", &_DFirstTrackP, "_DFirstTrackP[_nDs]/D");
    outputTree->Branch("_DSecondTrackPt", &_DSecondTrackPt, "_DSecondTrackPt[_nDs]/D");
    outputTree->Branch("_DSecondTrackEta", &_DSecondTrackEta, "_DSecondTrackEta[_nDs]/D");
    outputTree->Branch("_DSecondTrackPhi", &_DSecondTrackPhi, "_DSecondTrackPhi[_nDs]/D");
    outputTree->Branch("_DSecondTrackP", &_DSecondTrackP, "_DSecondTrackP[_nDs]/D");
    outputTree->Branch("_DThirdTrackPt", &_DThirdTrackPt, "_DThirdTrackPt[_nDs]/D");
    outputTree->Branch("_DThirdTrackEta", &_DThirdTrackEta, "_DThirdTrackEta[_nDs]/D");
    outputTree->Branch("_DThirdTrackPhi", &_DThirdTrackPhi, "_DThirdTrackPhi[_nDs]/D");
    outputTree->Branch("_DThirdTrackP", &_DThirdTrackP, "_DThirdTrackP[_nDs]/D");
    outputTree->Branch("_DFirstTrackX", &_DFirstTrackX, "_DFirstTrackX[_nDs]/D");
    outputTree->Branch("_DFirstTrackY", &_DFirstTrackY, "_DFirstTrackY[_nDs]/D");
    outputTree->Branch("_DFirstTrackZ", &_DFirstTrackZ, "_DFirstTrackZ[_nDs]/D");
    outputTree->Branch("_DSecondTrackX", &_DSecondTrackX, "_DSecondTrackX[_nDs]/D");
    outputTree->Branch("_DSecondTrackY", &_DSecondTrackY, "_DSecondTrackY[_nDs]/D");
    outputTree->Branch("_DSecondTrackZ", &_DSecondTrackZ, "_DSecondTrackZ[_nDs]/D");
    outputTree->Branch("_DThirdTrackX", &_DThirdTrackX, "_DThirdTrackX[_nDs]/D");
    outputTree->Branch("_DThirdTrackY", &_DThirdTrackY, "_DThirdTrackY[_nDs]/D");
    outputTree->Branch("_DThirdTrackZ", &_DThirdTrackZ, "_DThirdTrackZ[_nDs]/D");
}

// analyze (main method) //
void DAnalyzer::analyze(const edm::Event& iEvent, const reco::Vertex& primaryVertex){
    edm::Handle<std::vector<pat::PackedCandidate>> packedCandidates;
    iEvent.getByToken(multilepAnalyzer->packedCandidatesToken, packedCandidates);
    edm::Handle<std::vector<pat::PackedCandidate>> lostTracks;
    iEvent.getByToken(multilepAnalyzer->lostTracksToken, lostTracks);
    MagneticField* bfield = new OAEParametrizedMagneticField("3_8T");

    // store primary vertex and beamspot position
    _primaryVertexX = primaryVertex.position().x();
    _primaryVertexY = primaryVertex.position().y();
    _primaryVertexZ = primaryVertex.position().z();

    // preselect PackedCandidates and lost tracks (see help function below)
    std::vector<reco::Track> seltracks;
    for(const pat::PackedCandidate pc: *packedCandidates){
	if(pc.hasTrackDetails()){
	    reco::Track tr = *pc.bestTrack();
            std::map<std::string, double> temp = getTrackVariables(tr,bfield);
	    if(temp["pass"]>0.5) seltracks.push_back(tr);
        }
    }
    for(const pat::PackedCandidate pc : *lostTracks){
	if(pc.hasTrackDetails()){
	    reco::Track tr = *pc.bestTrack();
	    std::map<std::string, double> temp = getTrackVariables(tr,bfield);
	    if(temp["pass"]>0.5) seltracks.push_back(tr);
	}
    }

    // initialize number of Ds
    _nDs = 0;

    // combine selected tracks into D candidates and write properties
    std::vector< std::map<std::string,double> > dcandidates;
    // method 1: fit three tracks directly
    //dcandidates = DStar2010ThreeTracksFitter( seltracks, bfield, iEvent);
    // method 2: fit an intermediate decay first
    //dcandidates = DStar2010Fitter( seltracks, bfield, iEvent );
    dcandidates = DsFitter( seltracks, bfield, iEvent );
    for( std::map<std::string,double> temp: dcandidates ){
	    if(temp["type"]<0.) continue;
	    // D variables
	    _DInvMass[_nDs] = temp["invmass"];
	    _DType[_nDs] = temp["type"];
	    _DPx[_nDs] = temp["px"];
            _DPy[_nDs] = temp["py"];
            _DPz[_nDs] = temp["pz"];
            _DPt[_nDs] = temp["pt"];
	    _DEta[_nDs] = temp["eta"];
	    _DPhi[_nDs] = temp["phi"];
	    _DE[_nDs] = temp["e"];
	    _DIsolation[_nDs] = temp["isolation"];
	    _DDR[_nDs] = temp["dR"];
	    _DHasFastGenMatch[_nDs] = (temp["hasfastgenmatch"]>0.5);
	    _DHasGenMatch[_nDs] = (temp["hasgenmatch"]>0.5);
	    // intermediate resonance variables
	    _DIntResMass[_nDs] = temp["resmass"];
            _DIntResMassDiff[_nDs] = temp["resmassdiff"];
	    _DIntResX[_nDs] = temp["resvtxx"];
            _DIntResY[_nDs] = temp["resvtxy"];
            _DIntResZ[_nDs] = temp["resvtxz"];
	    _DIntResVtxNormChi2[_nDs] = temp["resvtxnormchi2"];
	    // track variables
	    _DFirstTrackPt[_nDs] = temp["firsttrackpt"];
            _DFirstTrackEta[_nDs] = temp["firsttracketa"];
            _DFirstTrackPhi[_nDs] = temp["firsttrackphi"];
	    _DFirstTrackP[_nDs] = temp["firsttrackp"];
            _DSecondTrackPt[_nDs] = temp["secondtrackpt"];
            _DSecondTrackEta[_nDs] = temp["secondtracketa"];
            _DSecondTrackPhi[_nDs] = temp["secondtrackphi"];
	    _DSecondTrackP[_nDs] = temp["secondtrackp"];
            _DThirdTrackPt[_nDs] = temp["thirdtrackpt"];
            _DThirdTrackEta[_nDs] = temp["thirdtracketa"];
            _DThirdTrackPhi[_nDs] = temp["thirdtrackphi"];
	    _DThirdTrackP[_nDs] = temp["thirdtrackp"];
	    _DFirstTrackX[_nDs] = temp["firsttrackx"];
            _DFirstTrackY[_nDs] = temp["firsttracky"];
            _DFirstTrackZ[_nDs] = temp["firsttrackz"];
	    _DSecondTrackX[_nDs] = temp["secondtrackx"];
            _DSecondTrackY[_nDs] = temp["secondtracky"];
            _DSecondTrackZ[_nDs] = temp["secondtrackz"];
	    _DThirdTrackX[_nDs] = temp["thirdtrackx"];
	    _DThirdTrackY[_nDs] = temp["thirdtracky"];
	    _DThirdTrackZ[_nDs] = temp["thirdtrackz"];

	    ++_nDs; 
            if(_nDs == nDs_max) break;
    }
    delete bfield;
}

std::map<std::string, double> DAnalyzer::getTrackVariables(
	const reco::Track& tr,
	MagneticField* bfield){

    std::map<std::string, double> outputmap = {
	{"loosequality",0.}, 
	{"normchi2",0.},
	{"nvalidhits",0.},
	{"pt",0.},
	{"transip",0.},
	{"pass",0.} // combined flag for final selection
    };
    // set variables and apply minimal selection
    outputmap["loosequality"] = (tr.quality(reco::TrackBase::qualityByName("loose")))? 1.: 0.;
    //if(outputmap["loosequality"] < 0.5) return outputmap;
    outputmap["normchi2"] = tr.normalizedChi2();
    //if(outputmap["normchi2"] > 5.) return outputmap;
    outputmap["nvalidhits"] = tr.numberOfValidHits();
    //if(outputmap["nvalidhits"] < 6) return outputmap;
    outputmap["pt"] = tr.pt();
    //if(outputmap["pt"] < 0.5) return outputmap;
    outputmap["transip"] = tr.dxy()/tr.dxyError();
    //if(outputmap["transip"] > 2.) return outputmap;
    // selections are passed; put flag pass to true and return the map
    outputmap["pass"] = 1.;
    return outputmap;
}

double DAnalyzer::getSumPt( const std::vector<reco::Track>& tracks, 
			    double eta, double phi,
			    double coneSize ){
    // get the sum of track pt's in a cone around a given direction
    ROOT::Math::PtEtaPhiMVector axis( 1, eta, phi, pimass );
    double sumpt = 0;
    for(unsigned i=0; i<tracks.size(); i++){
	const reco::Track tr = tracks.at(i);
	ROOT::Math::PxPyPzMVector trvec( tr.px(), tr.py(), tr.pz(), pimass );
	double dR = ROOT::Math::VectorUtil::DeltaR( axis, trvec );
        if( dR>coneSize ) continue;
	sumpt += tr.pt();
    }
    return sumpt;
}

std::vector< std::map<std::string,double> > DAnalyzer::DStar2010Fitter(
                const std::vector<reco::Track>& tracks,
                MagneticField* bfield,
                const edm::Event& iEvent){
    // RECONSTRUCT A D*(2010) CANDIDATE IN THE DECAY CHANNEL D*(2010) -> D0+PI -> K+PI+PI
    // note: based on the approach described in AN_2017_235_v11.
    std::vector< std::map<std::string,double> > dcandidates;
 
    // loop over all 2-track combinations in the event
    for(unsigned tr1index=0; tr1index<tracks.size(); tr1index++){
    for(unsigned tr2index=tr1index+1; tr2index<tracks.size(); tr2index++){
    
	// initialize candidate variables
	std::map<std::string, double> dcandidate = {
            // vertex variables
            {"type",-1.}, {"invmass",0.}, {"resmass",0.}, {"massdiff",0.},
            {"vtxx",0.}, {"vtxy",0.}, {"vtxz",0.},
            {"px",0.}, {"py",0.}, {"pz",0.}, {"pt",0.},
            {"eta",0.},{"phi",0.},
            // variables to perform further selection on
            {"vtxnormchi2",0.},
	    {"thirdtrackx",0.}, {"thirdtracky", 0.}, {"thirdtrackz", 0.}
	};
	// get the two initial tracks
	const reco::Track tr1 = tracks.at(tr1index);
	const reco::Track tr2 = tracks.at(tr2index);
	// apply additional quality cuts on the tracks
	if( tr1.pt()<1. || tr2.pt()<1. ) continue;
	if( !tr1.quality(reco::TrackBase::qualityByName("highPurity"))
	   || !tr2.quality(reco::TrackBase::qualityByName("highPurity")) ) continue;
	// cuts on the two-track combination
	if( tr1.charge()*tr2.charge()>0 ) continue;
	const math::XYZPoint tr1refpoint = tr1.referencePoint();
	const math::XYZPoint tr2refpoint = tr2.referencePoint();
	if( std::abs(tr1refpoint.x()-tr2refpoint.x())>0.1 ) continue;
	if( std::abs(tr1refpoint.y()-tr2refpoint.y())>0.1 ) continue;
	if( std::abs(tr1refpoint.z()-tr2refpoint.z())>0.1 ) continue;
	// fit a vertex
	std::vector<reco::TransientTrack> transtracks;
	transtracks.push_back(reco::TransientTrack(tr1, bfield));
	transtracks.push_back(reco::TransientTrack(tr2, bfield));
	KalmanVertexFitter vtxFitter(true); // use option true to include track refitting!
	TransientVertex resvtx = vtxFitter.vertex(transtracks);
	// vertex must be valid
	if(!resvtx.isValid()) continue;
	// chi squared of fit must be small
	if(resvtx.normalisedChiSquared()>15.) continue;
	if(resvtx.normalisedChiSquared()<0.) continue;
	// (chi2 criteria not explicitly mentioned, maybe remove later)
	// calculate refitted tracks for further selection
	/*std::vector<reco::TransientTrack> refittedTracks;
	if(resvtx.hasRefittedTracks()) refittedTracks = resvtx.refittedTracks();
	else continue; // should not happen when using option true in vtxFitter. */
	// calculate invariant mass 
	// assumption 1: first track is a kaon, second a pion
	ROOT::Math::PxPyPzMVector tr1k( tr1.px(), tr1.py(), tr1.pz(), kmass );
	ROOT::Math::PxPyPzMVector tr2pi( tr2.px(), tr2.py(), tr2.pz(), pimass );
	double invmasskpi = (tr1k+tr2pi).M();
	// assumption 2: first track is a pion, second a kaon
	ROOT::Math::PxPyPzMVector tr1pi( tr1.px(), tr1.py(), tr1.pz(), pimass );
	ROOT::Math::PxPyPzMVector tr2k( tr2.px(), tr2.py(), tr2.pz(), kmass );
	double invmasspik = (tr1pi+tr2k).M();
	bool isCaseKPi = false;
	// invariant mass must be close to resonance mass
	ROOT::Math::PxPyPzMVector resvector( 0, 0, 0, 0 );
	if( std::abs(invmasskpi-d0mass)<0.035 
	    && std::abs(invmasskpi-d0mass)<std::abs(invmasspik-d0mass) ){
	    resvector = tr1k+tr2pi;
	    dcandidate["resmass"] = invmasskpi;
	    isCaseKPi = true;
	} else if( std::abs(invmasspik-d0mass)<0.035 
	    && std::abs(invmasspik-d0mass)<std::abs(invmasskpi-d0mass) ){
	    resvector = tr1pi+tr2k;
	    dcandidate["resmass"] = invmasspik;
	} else continue;

	// do some printouts
	/*std::cout << "found D0 candidate: ";
	std::cout << resvector.Pt() << " " << resvector.Eta() << " " << resvector.Phi() << std::endl;
	std::cout << resvector.M() << std::endl;
	std::cout << resvtx.normalisedChiSquared() << std::endl;
	std::cout << std::abs(tr1refpoint.x()-tr2refpoint.x()) << " ";
	std::cout << std::abs(tr1refpoint.y()-tr2refpoint.y()) << " ";
	std::cout << std::abs(tr1refpoint.z()-tr2refpoint.z()) << std::endl;
	std::cout << tr1.quality(reco::TrackBase::qualityByName("highPurity")) << " ";
	std::cout << tr2.quality(reco::TrackBase::qualityByName("highPurity")) << std::endl;*/

	// loop over all possible third tracks
	for(unsigned tr3index=0; tr3index<tracks.size(); tr3index++){
	    if( tr3index==tr1index || tr3index==tr2index ) continue;
	    const reco::Track tr3 = tracks.at(tr3index);

	    //std::cout << "  candidate track: ";
	    //std::cout << tr3.pt() << " " << tr3.eta() << " " << tr3.phi() << std::endl;

	    // track must have opposite charge to kaon
	    if( isCaseKPi && tr1.charge()*tr3.charge()>0 ) continue;
	    else if( tr2.charge()*tr3.charge()>0 ) continue;

	    // additional quality cuts on the third track
	    if( tr3.pt()<0.35 ) continue;

	    /*if( tr3.pt()<0.5){
		std::cout << "  found low pt track: ";
		std::cout << tr3.pt() << " " << tr3.eta() << " " << tr3.phi() << std::endl;
	    }*/
	
	    // selections on the track-resonance combination
	    const math::XYZPoint tr3refpoint = tr3.referencePoint();
	    if( std::abs(tr3refpoint.x()-resvtx.position().x())>0.1 ) continue;
	    if( std::abs(tr3refpoint.y()-resvtx.position().y())>0.1 ) continue;
	    if( std::abs(tr3refpoint.z()-resvtx.position().z())>0.1 ) continue;

	    ROOT::Math::PxPyPzMVector tr3pi( tr3.px(), tr3.py(), tr3.pz(), pimass );
	    double dR = ROOT::Math::VectorUtil::DeltaR( tr3pi, resvector );
	    if( dR>0.15 ) continue;

	    // make the D candidate
	    ROOT::Math::LorentzVector dvector = tr3pi + resvector;

	    // do some printouts
	    /*std::cout << "  --> found D candidate: "; 
	    std::cout << dvector.Pt() << " ";
	    std::cout << dvector.Eta() << " ";
	    std::cout << dvector.Phi() << std::endl;
	    std::cout << "      " << dvector.M() << std::endl;*/
    
	    // further selections on the D candidate
	    if( dvector.pt()<5 ) continue;
	    double iso = dvector.Pt()/getSumPt( tracks, dvector.Eta(), dvector.Phi(), 0.4);
	    if( iso < 0.2 ) continue;

	    // write the output map
	    dcandidate["invmass"] = dvector.M();
	    dcandidate["massdiff"] = dvector.M() - dcandidate["resmass"];
	    dcandidate["type"] = didmap["dstar2010"];
	    dcandidate["vtxx"] = 0.; // a vertex position is not really defined
	    dcandidate["vtxy"] = 0.; // a vertex position is not really defined
	    dcandidate["vtxz"] = 0.; // a vertex position is not really defined
	    dcandidate["px"] = dvector.Px();
	    dcandidate["py"] = dvector.Py();
	    dcandidate["pz"] = dvector.Pz();
	    dcandidate["pt"] = dvector.Pt();
	    dcandidate["eta"] = dvector.Eta();
	    dcandidate["phi"] = dvector.Phi();
	    dcandidate["vtxnormchi2"] = 0.; // chi2 of vertex fit is not really defined
	    dcandidate["thirdtrackx"] = tr3refpoint.x();
	    dcandidate["thirdtracky"] = tr3refpoint.y();
	    dcandidate["thirdtrackz"] = tr3refpoint.z();
	    // break the loop over the third track if a suitable candidate was found
	    if( dcandidate["type"]==didmap["dstar2010"] ) break;
	}
	// add the D candidate to the list
	if( dcandidate["type"]==didmap["dstar2010"] ) dcandidates.push_back( dcandidate );
    // close the loop over all 2-track combinations
    }}
    // return the list of valid candidates
    return dcandidates;
}

std::vector< std::map<std::string,double> > DAnalyzer::DsFitter(
                const std::vector<reco::Track>& tracks,
                MagneticField* bfield,
                const edm::Event& iEvent){
    // RECONSTRUCT A Ds CANDIDATE IN THE DECAY CHANNEL Ds -> PHI+PI -> K+K+PI
    std::vector< std::map<std::string,double> > dcandidates;

    // get the gen-level decay for this event and decide whether to do matching
    edm::Handle<std::vector<reco::GenParticle>> genParticles;
    genParticles = getHandle(iEvent, multilepAnalyzer->genParticleToken);
    std::map< std::string, const reco::GenParticle* > DGenParticles;
    bool doMatching = true;
    if( multilepAnalyzer->isData() ) doMatching = false;
    if( !genParticles.isValid() ) doMatching = false;
    if( doMatching ){ 
	DGenParticles = DGenAnalyzer::find_Ds_To_PhiPi_To_KKPi( *genParticles );
	if( DGenParticles["Ds"]==nullptr ) doMatching = false;
    }

    // loop over all 2-track combinations in the event
    for(unsigned tr1index=0; tr1index<tracks.size(); tr1index++){
    for(unsigned tr2index=tr1index+1; tr2index<tracks.size(); tr2index++){

        // get the two initial tracks, pt-ordered
	bool ordered = (tracks.at(tr1index).pt()>tracks.at(tr2index).pt());
	const reco::Track tr1 = ordered ? tracks.at(tr1index) : tracks.at(tr2index);
	const reco::Track tr2 = ordered ? tracks.at(tr2index) : tracks.at(tr1index);
        // apply additional quality cuts on the tracks
        if( tr1.pt()<1.0 || tr2.pt()<1.0 ) continue;
        if( !tr1.quality(reco::TrackBase::qualityByName("highPurity"))
           || !tr2.quality(reco::TrackBase::qualityByName("highPurity")) ) continue;
        // cuts on the two-track combination
        if( tr1.charge()*tr2.charge()>0 ) continue;
        const math::XYZPoint tr1refpoint = tr1.referencePoint();
        const math::XYZPoint tr2refpoint = tr2.referencePoint();
        if( std::abs(tr1refpoint.x()-tr2refpoint.x())>0.1 ) continue;
        if( std::abs(tr1refpoint.y()-tr2refpoint.y())>0.1 ) continue;
        if( std::abs(tr1refpoint.z()-tr2refpoint.z())>0.1 ) continue;
        // fit a vertex
        std::vector<reco::TransientTrack> transtracks;
        transtracks.push_back(reco::TransientTrack(tr1, bfield));
        transtracks.push_back(reco::TransientTrack(tr2, bfield));
        KalmanVertexFitter vtxFitter(true); // use option true to include track refitting!
        TransientVertex resvtx = vtxFitter.vertex(transtracks);
        // vertex must be valid
        if(!resvtx.isValid()) continue;
        // chi squared of fit must be small
        if(resvtx.normalisedChiSquared()>15.) continue;
        if(resvtx.normalisedChiSquared()<0.) continue;
        // (chi2 criteria not explicitly mentioned, maybe remove later)
        // calculate invariant mass 
        ROOT::Math::PxPyPzMVector tr1vec( tr1.px(), tr1.py(), tr1.pz(), kmass );
        ROOT::Math::PxPyPzMVector tr2vec( tr2.px(), tr2.py(), tr2.pz(), kmass );
	ROOT::Math::PxPyPzMVector resvector = tr1vec + tr2vec;
        double resinvmass = resvector.M();
        if( std::abs(resinvmass - phimass)>0.07 ) continue;

        // loop over all possible third tracks
        for(unsigned tr3index=0; tr3index<tracks.size(); tr3index++){
            if( tr3index==tr1index || tr3index==tr2index ) continue;
            const reco::Track tr3 = tracks.at(tr3index);
            // additional quality cuts on the third track
            if( tr3.pt()<1.0 ) continue;
            // selections on the track-resonance combination
            const math::XYZPoint tr3refpoint = tr3.referencePoint();
            if( std::abs(tr3refpoint.x()-resvtx.position().x())>0.1 ) continue;
            if( std::abs(tr3refpoint.y()-resvtx.position().y())>0.1 ) continue;
            if( std::abs(tr3refpoint.z()-resvtx.position().z())>0.1 ) continue;
            ROOT::Math::PxPyPzMVector tr3pi( tr3.px(), tr3.py(), tr3.pz(), pimass );
            double dR = ROOT::Math::VectorUtil::DeltaR( tr3pi, resvector );
            if( dR>0.2 ) continue;
            // make the D candidate
            ROOT::Math::LorentzVector dvector = tr3pi + resvector;
            // further selections on the D candidate
            if( dvector.pt()<1 ) continue;
            double iso = dvector.Pt()/getSumPt( tracks, dvector.Eta(), dvector.Phi(), 0.4);
            //if( iso < 0.2 ) continue;

	    // do some printouts
            /*std::cout << "  --> found D candidate: "; 
            std::cout << dvector.Pt() << " ";
            std::cout << dvector.Eta() << " ";
            std::cout << dvector.Phi() << std::endl;
            std::cout << "      " << dvector.M() << std::endl;*/
	
	    // initialize candidate variables
	    // note: must be done here since must be re-initialized for each candidate!
	    std::map<std::string, double> dcandidate = {
		// Ds variables
		{"type",-1.}, {"invmass",0.},
		{"px",0.}, {"py",0.}, {"pz",0.},
		{"pt",0.}, {"eta",0.},{"phi",0.}, {"e",0.},
		{"isolation", 0.}, {"dR", 0.},
		{"hasfastgenmatch", 0.}, {"hasgenmatch", 0.},
		// intermediate resonance variables
		{"resmass",0.}, {"resmassdiff",0.},
		{"resvtxx",0.}, {"resvtxy",0.}, {"resvtxz",0.},
		{"resvtxnormchi2",0.},
		// track variables
		{"firsttrackpt",0.}, {"secondtrackpt", 0.}, {"thirdtrackpt", 0.},
		{"firsttracketa", 0.}, {"secondtracketa", 0.}, {"thirdtracketa", 0.},
		{"firsttrackphi", 0.}, {"secondtrackphi", 0.}, {"thirdtrackphi", 0.},
		{"firsttrackp",0.}, {"secondtrackp", 0.}, {"thirdtrackp", 0.},
		{"firsttrackx",0.}, {"firsttracky", 0.}, {"firsttrackz", 0.},
		{"secondtrackx",0.}, {"secondtracky", 0.}, {"secondtrackz", 0.},
		{"thirdtrackx",0.}, {"thirdtracky", 0.}, {"thirdtrackz", 0.},
	    };
    
	    // write the output map
	    // Ds variables
	    dcandidate["invmass"] = dvector.M();
            dcandidate["type"] = didmap["ds"];
	    dcandidate["px"] = dvector.Px();
            dcandidate["py"] = dvector.Py();
            dcandidate["pz"] = dvector.Pz();
            dcandidate["pt"] = dvector.Pt();
            dcandidate["eta"] = dvector.Eta();
            dcandidate["phi"] = dvector.Phi();
	    dcandidate["e"] = dvector.E();
	    dcandidate["isolation"] = iso;
	    dcandidate["dR"] = dR;
	    // intermediate resonance variables
	    dcandidate["resmass"] = resinvmass;
            dcandidate["resmassdiff"] = dvector.M() - resinvmass;
	    dcandidate["resvtxx"] = resvtx.position().x();
            dcandidate["resvtxy"] = resvtx.position().y();
            dcandidate["resvtxz"] = resvtx.position().z();
            dcandidate["resvtxnormchi2"] = resvtx.normalisedChiSquared();
	    // track variables
	    dcandidate["firsttrackpt"] = tr1.pt();
	    dcandidate["firsttracketa"] = tr1.eta();
	    dcandidate["firsttrackphi"] = tr1.phi();
	    dcandidate["firsttrackp"] = tr1.p();
	    dcandidate["secondtrackpt"] = tr2.pt();
            dcandidate["secondtracketa"] = tr2.eta();
            dcandidate["secondtrackphi"] = tr2.phi();
	    dcandidate["secondtrackp"] = tr2.p();
	    dcandidate["thirdtrackpt"] = tr3.pt();
            dcandidate["thirdtracketa"] = tr3.eta();
            dcandidate["thirdtrackphi"] = tr3.phi();
	    dcandidate["thirdtrackp"] = tr3.p();
	    dcandidate["firsttrackx"] = tr1refpoint.x();
            dcandidate["firsttracky"] = tr1refpoint.y();
            dcandidate["firsttrackz"] = tr1refpoint.z();
	    dcandidate["secondtrackx"] = tr2refpoint.x();
            dcandidate["secondtracky"] = tr2refpoint.y();
            dcandidate["secondtrackz"] = tr2refpoint.z();
            dcandidate["thirdtrackx"] = tr3refpoint.x();
            dcandidate["thirdtracky"] = tr3refpoint.y();
            dcandidate["thirdtrackz"] = tr3refpoint.z();

	    // check if this candidate can be matched to gen-level
	    if( doMatching ){
		// method 1: fast matching using particles from DGenAnalyzer
		double dRThreshold = 0.05;
		if(   GenTools::isGeometricTrackMatch( tr3, *DGenParticles["Pi"], dRThreshold )
		 && ((GenTools::isGeometricTrackMatch( tr1, *DGenParticles["K1"], dRThreshold)
		      && GenTools::isGeometricTrackMatch( tr2, *DGenParticles["K2"], dRThreshold))
		    || (GenTools::isGeometricTrackMatch( tr1, *DGenParticles["K2"], dRThreshold)
		      && GenTools::isGeometricTrackMatch( tr2, *DGenParticles["K1"], dRThreshold)) ) ){
		    dcandidate["hasfastgenmatch"] = 1.0;
		}
		// method 2: regular matching using all genParticles
		const reco::GenParticle* tr3match;
		tr3match = GenTools::geometricTrackMatch( tr3, *genParticles, 211, dRThreshold );
		const reco::GenParticle* tr2match;
		tr2match = GenTools::geometricTrackMatch( tr2, *genParticles, 321, dRThreshold );
		const reco::GenParticle* tr1match;
		tr1match = GenTools::geometricTrackMatch( tr1, *genParticles, 321, dRThreshold );
		if( !( tr1match==nullptr || tr2match==nullptr || tr3match==nullptr ) ){
		    // method 2a: check the matched particle mothers
		    // note: mother particles seem to be not always stored, 
                    // giving nullptrs and segfaults 
		    /*if( std::abs(GenTools::getMother(*tr3match,*genParticles)->pdgId())==431
			&& std::abs(GenTools::getMother(*tr1match,*genParticles)->pdgId())==333
			&& std::abs(GenTools::getMother(*tr2match,*genParticles)->pdgId())==333 ){
			dcandidate["hasgenmatch"] = 1.0; 
		    }*/
		    // method 2b: compare the matched particles to DGenParticles
		    // printouts for testing
		    /*std::cout << "track 3: " << tr3.eta() << " " << tr3.phi() << std::endl;
		    std::cout << "track 3 match: " << tr3match->eta() << " " << tr3match->phi() << std::endl;
		    std::cout << "track 2: " << tr2.eta() << " " << tr2.phi() << std::endl;
		    std::cout << "track 2 match: " << tr2match->eta() << " " << tr2match->phi() << std::endl;
		    std::cout << "track 1: " << tr1.eta() << " " << tr1.phi() << std::endl;
		    std::cout << "track 1 match: " << tr1match->eta() << " " << tr1match->phi() << std::endl;
		    std::cout << "pi: " << DGenParticles["Pi"]->eta() << " " << DGenParticles["Pi"]->phi() << std::endl;
		    std::cout << "K1: " << DGenParticles["K1"]->eta() << " " << DGenParticles["K1"]->phi() << std::endl;
		    std::cout << "K2: " << DGenParticles["K2"]->eta() << " " << DGenParticles["K2"]->phi() << std::endl;*/
		    // determine if matched particles correspond to DGenParticles
		    if(    GenTools::isGeometricGenParticleMatch( *tr3match, *DGenParticles["Pi"], 1e-6 )
			&& ((GenTools::isGeometricGenParticleMatch( *tr1match, *DGenParticles["K1"], 1e-6)
                             && GenTools::isGeometricGenParticleMatch( *tr2match, *DGenParticles["K2"], 1e-6))
                           || (GenTools::isGeometricGenParticleMatch( *tr1match, *DGenParticles["K2"], 1e-6)
                             && GenTools::isGeometricGenParticleMatch( *tr2match, *DGenParticles["K1"], 1e-6)) ) ){
			dcandidate["hasgenmatch"] = 1.0;
		    }
		}
	    } // endif doMatching
	    // add the D candidate to the list
            if( dcandidate["type"]==didmap["ds"] ) dcandidates.push_back( dcandidate );
        } // close the loop over third track
    }} // close the loop over all 2-track combinations
    // return the list of valid candidates
    return dcandidates;
}

    
std::vector< std::map<std::string,double> > DAnalyzer::DStar2010ThreeTracksFitter(
		const std::vector<reco::Track>& tracks,
		MagneticField* bfield,
		const edm::Event& iEvent){
    // LOOP OVER ALL THREE-TRACK COMBINATIONS AND FIT A D*(2010) RESONANCE
    std::vector< std::map<std::string,double> > dcandidates;
    // loop over all three-track combinations
    for(unsigned tr1index=0; tr1index<tracks.size(); tr1index++){
    for(unsigned tr2index=tr1index+1; tr2index<tracks.size(); tr2index++){
    for(unsigned tr3index=tr2index+1; tr3index<tracks.size(); tr3index++){

	const reco::Track tr1 = tracks.at(tr1index);
        const reco::Track tr2 = tracks.at(tr2index);
	const reco::Track tr3 = tracks.at(tr3index);
    
	std::map<std::string, double> dcandidate = {
            // vertex variables
            {"type",-1.},{"invmass",0.},
            {"vtxx",0.}, {"vtxy",0.}, {"vtxz",0.},
            {"px",0.}, {"py",0.}, {"pz",0.}, {"pt",0.},
            {"eta",0.},{"phi",0.},
            // variables to perform selection on (in order of appearance)
            {"vtxnormchi2",0.},
	};
	// condition: candidates must have charges {+,+,-} or {+,-,-},
	// i.e. everything except for {+,+,+} and {-,-,-} is allowed
	if(tr1.charge()>0 && tr2.charge()>0 && tr3.charge()>0) continue;
	if(tr1.charge()<0 && tr2.charge()<0 && tr3.charge()<0) continue;
	// condition: mininum pt
	if(tr1.pt()<0.35 || tr2.pt()<0.35 || tr3.pt()<0.35) continue;
	// condition: invariant mass in coarse window (assuming massless particles)
	ROOT::Math::PxPyPzMVector tr1vec( tr1.px(), tr1.py(), tr1.pz(), 0. );
	ROOT::Math::PxPyPzMVector tr2vec( tr2.px(), tr2.py(), tr2.pz(), 0. );
	ROOT::Math::PxPyPzMVector tr3vec( tr3.px(), tr3.py(), tr3.pz(), 0. );
	double norm = (tr1vec+tr2vec+tr3vec).M();
	if( norm > 4. ) continue;
	// preliminary conditions are met; now fit a vertex and perform further selections    
	std::vector<reco::TransientTrack> transtracks;
	transtracks.push_back(reco::TransientTrack(tr1, bfield));
	transtracks.push_back(reco::TransientTrack(tr2, bfield));
	transtracks.push_back(reco::TransientTrack(tr2, bfield));
	KalmanVertexFitter vtxFitter(true); // use option true to include track refitting!
	TransientVertex dvtx = vtxFitter.vertex(transtracks);
	// condition: vertex must be valid
	if(!dvtx.isValid()) continue;
	// condition: chi squared of fit must be small
	if(dvtx.normalisedChiSquared()>15.) continue;
	if(dvtx.normalisedChiSquared()<0.) continue;
	dcandidate["vtxnormchi2"] = dvtx.normalisedChiSquared();
	// calculate refitted tracks for further selection
	std::vector<reco::TransientTrack> refittedTracks;
	if(dvtx.hasRefittedTracks()) refittedTracks = dvtx.refittedTracks();
	else continue; // should not happen when using option true in vtxFitter.
	// calculate vertex position
	GlobalPoint vtxXYZ(dvtx.position().x(), dvtx.position().y(), dvtx.position().z());
	dcandidate["vtxx"] = vtxXYZ.x();
	dcandidate["vtxy"] = vtxXYZ.y();
	dcandidate["vtxz"] = vtxXYZ.z();
	reco::Vertex drecovtx = dvtx;
	// calculate charge of D candidate
	//int DCharge = -tr1.charge()*tr2.charge()*tr3.charge();
	// calculate invariant mass
	double invmass = norm; // temp 
	// condition: mass must be close to D resonance mass
	// case 1: D*(2010)
	if(std::abs(invmass-dstar2010mass) < 0.1){
	    dcandidate["invmass"] = invmass;
	    dcandidate["type"] = didmap["dstar2010"];
	}
	if( dcandidate["type"]>0 ) dcandidates.push_back(dcandidate);

    // close the loop over all three-track combinations
    }}}
    return dcandidates;
}
