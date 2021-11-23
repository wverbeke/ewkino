/*
Custom analyzer class for finding D mesons decaying to three tracked particles.
*/

#include "heavyNeutrino/multilep/interface/DEfficiencyAnalyzer.h"

// constructor //
DEfficiencyAnalyzer::DEfficiencyAnalyzer(const edm::ParameterSet& iConfig, multilep* multilepAnalyzer):
    multilepAnalyzer(multilepAnalyzer){
};

// destructor //
DEfficiencyAnalyzer::~DEfficiencyAnalyzer(){
}

// beginJob //
void DEfficiencyAnalyzer::beginJob(TTree* outputTree){

    outputTree->Branch("_DEffCut", &_DEffCut, "_DEffCut/i");
}

// analyze (main method) //
void DEfficiencyAnalyzer::analyze(const edm::Event& iEvent, const reco::Vertex& primaryVertex){
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

    // check at what stage the reconstruction fails
    // and write the result to variable _DEffCut
    _DEffCut = DsEfficiencyFitter( seltracks, bfield, iEvent );
}

std::map<std::string, double> DEfficiencyAnalyzer::getTrackVariables(
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

double DEfficiencyAnalyzer::getSumPt( const std::vector<reco::Track>& tracks, 
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

unsigned DEfficiencyAnalyzer::DsEfficiencyFitter(
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

    // if not possible to do matching, return 0
    // note: 0 will also be returned in case gen matching is possible
    //       but no track combinations have a gen match!
    if(!doMatching) return 0;

    // initializations for loop over track combinations
    unsigned ngenmatch = 0;
    unsigned res = 0;
    double dRThreshold = 0.05;

    // loop over all 2-track combinations in the event
    for(unsigned tr1index=0; tr1index<tracks.size(); tr1index++){
    for(unsigned tr2index=tr1index+1; tr2index<tracks.size(); tr2index++){
    
	// get the two initial tracks, pt-ordered
        bool ordered = (tracks.at(tr1index).pt()>tracks.at(tr2index).pt());
        const reco::Track tr1 = ordered ? tracks.at(tr1index) : tracks.at(tr2index);
        const reco::Track tr2 = ordered ? tracks.at(tr2index) : tracks.at(tr1index);

	// do gen-matching of first two tracks
	if( ! ((GenTools::isGeometricTrackMatch( tr1, *DGenParticles["K1"], dRThreshold)
                && GenTools::isGeometricTrackMatch( tr2, *DGenParticles["K2"], dRThreshold))
              || (GenTools::isGeometricTrackMatch( tr1, *DGenParticles["K2"], dRThreshold)
                && GenTools::isGeometricTrackMatch( tr2, *DGenParticles["K1"], dRThreshold)) ) ){
	    continue; }

	// loop over possible third tracks
	for(unsigned tr3index=0; tr3index<tracks.size(); tr3index++){
            if( tr3index==tr1index || tr3index==tr2index ) continue;
            const reco::Track tr3 = tracks.at(tr3index);
	    // do gen-matching of third track
	    if( ! GenTools::isGeometricTrackMatch( tr3, *DGenParticles["Pi"], dRThreshold ) ){
		continue; }

	    // current track combination is gen-matched
	    ngenmatch++;

	    // hopefully there is only one combination at this stage
	    /*if( ngenmatch>1 ){
		std::string msg = "WARNING: found " + std::to_string(ngenmatch);
		msg += " gen-matched track combinations in the same event...";
		std::cerr << msg << std::endl; 
	    }*/
	    // conclusion from test: it can happen quite regularly that there are >1 candidates.
	    // strategy: return the largest value for res among the different candidates!

	    // from here on: apply additional cuts and return increasing integer numbers 
	    // (starting from 1!) when a cut is not passed.

	    // apply additional quality cuts on the tracks
	    if( tr1.pt()<2.0 || tr2.pt()<2.0 ){ res=(res<1)?1:res; continue; }
	    if( !tr1.quality(reco::TrackBase::qualityByName("highPurity"))
		|| !tr2.quality(reco::TrackBase::qualityByName("highPurity")) ){ 
		res=(res<2)?2:res; continue; }
	    // cuts on the two-track combination
	    if( tr1.charge()*tr2.charge()>0 ){ res=(res<3)?3:res; continue; }
	    const math::XYZPoint tr1refpoint = tr1.referencePoint();
	    const math::XYZPoint tr2refpoint = tr2.referencePoint();
	    if( ( std::abs(tr1refpoint.x()-tr2refpoint.x())>0.1 )
		|| ( std::abs(tr1refpoint.y()-tr2refpoint.y())>0.1 )
		|| ( std::abs(tr1refpoint.z()-tr2refpoint.z())>0.1 ) ){ 
		res=(res<4)?4:res; continue;}
	    // fit a vertex
	    std::vector<reco::TransientTrack> transtracks;
	    transtracks.push_back(reco::TransientTrack(tr1, bfield));
	    transtracks.push_back(reco::TransientTrack(tr2, bfield));
	    KalmanVertexFitter vtxFitter(true); // use option true to include track refitting!
	    TransientVertex resvtx = vtxFitter.vertex(transtracks);
	    // vertex must be valid
	    if(!resvtx.isValid()){ res=(res<5)?5:res; continue; }
	    // chi squared of fit must be small
	    if( (resvtx.normalisedChiSquared()>15.)
		|| (resvtx.normalisedChiSquared()<0.) ){ res=(res<6)?6:res; continue; }
	    // (chi2 criteria not explicitly mentioned, maybe remove later)
	    // calculate invariant mass 
	    ROOT::Math::PxPyPzMVector tr1vec( tr1.px(), tr1.py(), tr1.pz(), kmass );
	    ROOT::Math::PxPyPzMVector tr2vec( tr2.px(), tr2.py(), tr2.pz(), kmass );
	    ROOT::Math::PxPyPzMVector resvector = tr1vec + tr2vec;
	    double resinvmass = resvector.M();
	    if( std::abs(resinvmass - phimass)>0.07 ){ res=(res<7)?7:res; continue; }

            // additional quality cuts on the third track
            if( tr3.pt()<2.0 ){ res=(res<8)?8:res; continue; }
            // selections on the track-resonance combination
            //const math::XYZPoint tr3refpoint = tr3.referencePoint();
            /*if( ( std::abs(tr3refpoint.x()-resvtx.position().x())>0.5 )
		|| ( std::abs(tr3refpoint.y()-resvtx.position().y())>0.5 )
		|| ( std::abs(tr3refpoint.z()-resvtx.position().z())>0.5 ) ){ 
		res=(res<9)?9:res; continue; }*/
            ROOT::Math::PxPyPzMVector tr3pi( tr3.px(), tr3.py(), tr3.pz(), pimass );
            double dR = ROOT::Math::VectorUtil::DeltaR( tr3pi, resvector );
            if( dR>0.2 ){ res=(res<9)?9:res;  continue; }
            // make the D candidate
            ROOT::Math::LorentzVector dvector = tr3pi + resvector;
            // further selections on the D candidate
            if( dvector.pt()<1 ){ res=(res<10)?10:res; continue; }
	    double sumpt = getSumPt( tracks, dvector.Eta(), dvector.Phi(), 0.4);
	    double iso = 0;
	    if( sumpt>0 ) iso = dvector.Pt()/sumpt;
            if( iso < 0.2 ){ res=(res<11)?11:res; continue; }

	    // cuts are passed
	    res=12;

        } // close the loop over third track
    }} // close the loop over all 2-track combinations
    return res;
}
