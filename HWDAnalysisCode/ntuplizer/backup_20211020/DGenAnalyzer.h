/*
Custom analyzer class for investigating gen-level D meson decays.
*/
#ifndef DGEN_ANALYZER_H
#define DGEN_ANALYZER_H

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

// dataformats include files
#include "DataFormats/Math/interface/LorentzVector.h"

class multilep;

class DGenAnalyzer {
  friend class multilep;
  private:

    multilep* multilepAnalyzer;

    int _genDEventType = 0;

  public:
    DGenAnalyzer(const edm::ParameterSet& iConfig, multilep* vars);
    ~DGenAnalyzer();
    // template member functions
    void beginJob(TTree*);
    void analyze(const edm::Event&);

    static void find_DStar_To_D0Pi_To_KPiPi(const std::vector<reco::GenParticle>&);
    static std::map< std::string, const reco::GenParticle* > find_Ds_To_PhiPi_To_KKPi(const std::vector<reco::GenParticle>&);
};

#endif
