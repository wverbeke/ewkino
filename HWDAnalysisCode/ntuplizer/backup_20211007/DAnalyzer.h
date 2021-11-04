/*
Custom analyzer class for finding D mesons decaying to three tracked particles.
*/
#ifndef D_ANALYZER_H
#define D_ANALYZER_H

// include other parts of the heavyNeutrino framework
#include "heavyNeutrino/multilep/plugins/multilep.h"

// system include files
#include <memory>
#include <unordered_map>

// root classes
#include <Math/SVector.h> // root high-performance vector class
#include <Math/SMatrix.h> // root high-performance matrix class
#include <Math/Vector4D.h>

// main include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

// general include files
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"
#include "TTree.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/Math/interface/deltaR.h"

// vertex fitter include files
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/ParametrizedEngine/src/OAEParametrizedMagneticField.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "TrackingTools/PatternTools/interface/TSCBLBuilderNoMaterial.h"
#include "TrackingTools/PatternTools/interface/ClosestApproachInRPhi.h"

// data format include files
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/Candidate/interface/VertexCompositeCandidate.h"
#include "DataFormats/Candidate/interface/VertexCompositeCandidateFwd.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/Candidate/interface/VertexCompositePtrCandidate.h"

class multilep;

class DAnalyzer {
  friend class multilep;
  private:
    // constants
    // - masses of final stat particles
    static constexpr double pimass = 0.13957;
    static constexpr double kmass = 0.4937;
    // - masses of intermediate resonances
    static constexpr double d0mass = 1.865;
    static constexpr double phimass = 1.019;
    // - masses of D particles
    static constexpr double dstar2010mass = 2.010;
    static constexpr double dsmass = 1.968;
    // - identification map
    std::map<std::string,int> didmap = {
	{"dstar2010", 1},
	{"ds", 2}
    };
    // Root tree variable declarations
    static const unsigned nDs_max = 20;
    // primary vertex position
    double _primaryVertexX;
    double _primaryVertexY;
    double _primaryVertexZ;
    // variables from customized D fitter
    unsigned _nDs = 0;    
    double _DInvMass[nDs_max];
    double _DIntResMass[nDs_max];
    double _DMassDiff[nDs_max];
    unsigned _DType[nDs_max];
    double _DX[nDs_max];
    double _DY[nDs_max];
    double _DZ[nDs_max];
    double _DPx[nDs_max];
    double _DPy[nDs_max];
    double _DPz[nDs_max];
    double _DPt[nDs_max];
    double _DEta[nDs_max];
    double _DPhi[nDs_max];
    double _DVtxNormChi2[nDs_max];
    double _DFirstTrackX[nDs_max];
    double _DFirstTrackY[nDs_max];
    double _DFirstTrackZ[nDs_max];
    double _DSecondTrackX[nDs_max];
    double _DSecondTrackY[nDs_max];
    double _DSecondTrackZ[nDs_max];
    double _DThirdTrackX[nDs_max];
    double _DThirdTrackY[nDs_max];
    double _DThirdTrackZ[nDs_max];

    unsigned debugcounter = 0;

    multilep* multilepAnalyzer;

  public:
    DAnalyzer(const edm::ParameterSet& iConfig, multilep* vars);
    ~DAnalyzer();
    // template member functions
    void beginJob(TTree*);
    void analyze(const edm::Event&, const reco::Vertex&);
    // help member functions
    std::map<std::string, double> getTrackVariables(
	const reco::Track&, MagneticField* );
    double getSumPt( const std::vector<reco::Track>&,
                            double, double, double );
    std::vector< std::map<std::string,double> > DStar2010Fitter(
                const std::vector<reco::Track>&,
                MagneticField*,
                const edm::Event& );
    std::vector< std::map<std::string,double> > DsFitter(
                const std::vector<reco::Track>& tracks,
                MagneticField* bfield,
                const edm::Event& iEvent);
    std::vector< std::map<std::string,double> > DStar2010ThreeTracksFitter(
                const std::vector<reco::Track>&,
                MagneticField*,
                const edm::Event& );
};

#endif
