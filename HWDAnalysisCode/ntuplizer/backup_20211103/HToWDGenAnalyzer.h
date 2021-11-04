/*
Custom analyzer class for investigating gen-level H->W+Ds decays.
*/
#ifndef HTOWDGEN_ANALYZER_H
#define HTOWDGEN_ANALYZER_H

// include other parts of the heavyNeutrino framework
#include "heavyNeutrino/multilep/plugins/multilep.h"

// system include files
#include <memory>
#include <unordered_map>

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

class HToWDGenAnalyzer {
  friend class multilep;
  private:

    multilep* multilepAnalyzer;

    int _genhtowdEventType = 0;
    double _genhtowdHPt = 0;
    double _genhtowdHEta = 0;
    double _genhtowdHPhi = 0;
    double _genhtowdHE = 0;
    double _genhtowdWPt = 0;
    double _genhtowdWEta = 0;
    double _genhtowdWPhi = 0;
    double _genhtowdWE = 0;
    double _genhtowdWCharge = 0;
    double _genhtowdNuPt = 0;
    double _genhtowdNuEta = 0;
    double _genhtowdNuPhi = 0;
    double _genhtowdNuE = 0;
    double _genhtowdLPt = 0;
    double _genhtowdLEta = 0;
    double _genhtowdLPhi = 0;
    double _genhtowdLE = 0;
    double _genhtowdLCharge = 0;
    double _genhtowdLFlavour = 0;
    double _genhtowdDPt = 0;
    double _genhtowdDEta = 0;
    double _genhtowdDPhi = 0;
    double _genhtowdDE = 0;
    double _genhtowdDCharge = 0;

  public:
    HToWDGenAnalyzer(const edm::ParameterSet& iConfig, multilep* vars);
    ~HToWDGenAnalyzer();
    // template member functions
    void beginJob(TTree*);
    void analyze(const edm::Event&);

    static std::map< std::string, const reco::GenParticle* > find_H_to_WD(
	const std::vector<reco::GenParticle>&);
};

#endif
