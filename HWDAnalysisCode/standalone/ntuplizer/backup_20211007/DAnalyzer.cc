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
    // D compound variables
    // - ID variables
    outputTree->Branch("_nDs", &_nDs, "_nDs/i");
    outputTree->Branch("_DInvMass", &_DInvMass, "_DInvMass[_nDs]/D");
    outputTree->Branch("_DIntResMass", &_DIntResMass, "_DIntResMass[_nDs]/D");
    outputTree->Branch("_DMassDiff", &_DMassDiff, "_DMassDiff[_nDs]/D");
    outputTree->Branch("_DType", &_DType, "_DType[_nDs]/i");
    // - vertex position and distance
    outputTree->Branch("_DX", &_DX, "_DX[_nDs]/D");
    outputTree->Branch("_DY", &_DY, "_DY[_nDs]/D");
    outputTree->Branch("_DZ", &_DZ, "_DZ[_nDs]/D");
    // - momentum and direction
    outputTree->Branch("_DPx", &_DPx, "_DPx[_nDs]/D");
    outputTree->Branch("_DPy", &_DPy, "_DPy[_nDs]/D");
    outputTree->Branch("_DPz", &_DPz, "_DPz[_nDs]/D");
    outputTree->Branch("_DPt", &_DPt, "_DPt[_nDs]/D");
    outputTree->Branch("_DEta", &_DEta, "_DEta[_nDs]/D");
    outputTree->Branch("_DPhi", &_DPhi, "_DPhi[_nDs]/D");
    // - other
    outputTree->Branch("_DVtxNormChi2", &_DVtxNormChi2, "_DVtxNormChi2[_nDs]/D");
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
	    _DInvMass[_nDs] = temp["invmass"];
	    _DIntResMass[_nDs] = temp["resmass"];
	    _DMassDiff[_nDs] = temp["massdiff"];
	    _DType[_nDs] = temp["type"];
	    _DX[_nDs] = temp["vtxx"];
	    _DY[_nDs] = temp["vtxy"];
	    _DZ[_nDs] = temp["vtxz"];
	    _DPx[_nDs] = temp["px"];
            _DPy[_nDs] = temp["py"];
            _DPz[_nDs] = temp["pz"];
            _DPt[_nDs] = temp["pt"];
	    _DEta[_nDs] = temp["eta"];
	    _DPhi[_nDs] = temp["phi"];
	    _DVtxNormChi2[_nDs] = temp["vtxnormchi2"];
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
	    {"firsttrackx",0.}, {"firsttracky", 0.}, {"firsttrackz", 0.},
	    {"secondtrackx",0.}, {"secondtracky", 0.}, {"secondtrackz", 0.},
            {"thirdtrackx",0.}, {"thirdtracky", 0.}, {"thirdtrackz", 0.},
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
        // calculate invariant mass 
        ROOT::Math::PxPyPzMVector tr1vec( tr1.px(), tr1.py(), tr1.pz(), kmass );
        ROOT::Math::PxPyPzMVector tr2vec( tr2.px(), tr2.py(), tr2.pz(), kmass );
	ROOT::Math::PxPyPzMVector resvector = tr1vec + tr2vec;
        double resinvmass = resvector.M();
	dcandidate["resmass"] = resinvmass;
        if( std::abs(resinvmass - phimass)>0.035 ) continue;

        // loop over all possible third tracks
        for(unsigned tr3index=0; tr3index<tracks.size(); tr3index++){
            if( tr3index==tr1index || tr3index==tr2index ) continue;
            const reco::Track tr3 = tracks.at(tr3index);
            // additional quality cuts on the third track
            if( tr3.pt()<1 ) continue;
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
            // further selections on the D candidate
            if( dvector.pt()<5 ) continue;
            double iso = dvector.Pt()/getSumPt( tracks, dvector.Eta(), dvector.Phi(), 0.4);
            if( iso < 0.2 ) continue;

	    // do some printouts
            std::cout << "  --> found D candidate: "; 
            std::cout << dvector.Pt() << " ";
            std::cout << dvector.Eta() << " ";
            std::cout << dvector.Phi() << std::endl;
            std::cout << "      " << dvector.M() << std::endl;
	    
	    // write the output map
            dcandidate["invmass"] = dvector.M();
            dcandidate["massdiff"] = dvector.M() - dcandidate["resmass"];
            dcandidate["type"] = didmap["ds"];
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
	    dcandidate["firsttrackx"] = tr1refpoint.x();
            dcandidate["firsttracky"] = tr1refpoint.y();
            dcandidate["firsttrackz"] = tr1refpoint.z();
	    dcandidate["secondtrackx"] = tr2refpoint.x();
            dcandidate["secondtracky"] = tr2refpoint.y();
            dcandidate["secondtrackz"] = tr2refpoint.z();
            dcandidate["thirdtrackx"] = tr3refpoint.x();
            dcandidate["thirdtracky"] = tr3refpoint.y();
            dcandidate["thirdtrackz"] = tr3refpoint.z();
            // break the loop over the third track if a suitable candidate was found
            if( dcandidate["type"]==didmap["ds"] ) break;
        }
        // add the D candidate to the list
        if( dcandidate["type"]==didmap["ds"] ) dcandidates.push_back( dcandidate );
    // close the loop over all 2-track combinations
    }}
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
