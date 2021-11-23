/*
Custom analyzer class for finding D mesons decaying to three tracked particles.
*/

#ifndef D_EFFICIENCY_ANALYZER_H
#define D_EFFICIENCY_ANALYZER_H

// include other parts of the heavyNeutrino framework
#include "heavyNeutrino/multilep/plugins/multilep.h"
#include "heavyNeutrino/multilep/interface/GenTools.h"
#include "heavyNeutrino/multilep/interface/DGenAnalyzer.h"

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

class DEfficiencyAnalyzer {
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
    static const unsigned nDs_max = 30;
    // primary vertex position
    double _primaryVertexX;
    double _primaryVertexY;
    double _primaryVertexZ;
    // variables from customized D fitter
    unsigned _DEffCut;

    multilep* multilepAnalyzer;

  public:
    DEfficiencyAnalyzer(const edm::ParameterSet& iConfig, multilep* vars);
    ~DEfficiencyAnalyzer();
    // template member functions
    void beginJob(TTree*);
    void analyze(const edm::Event&, const reco::Vertex&);
    // help member functions
    std::map<std::string, double> getTrackVariables(
	const reco::Track&, MagneticField* );
    double getSumPt( const std::vector<reco::Track>&,
                            double, double, double );
    unsigned DsEfficiencyFitter(
                const std::vector<reco::Track>& tracks,
                MagneticField* bfield,
                const edm::Event& iEvent);
};

#endif
