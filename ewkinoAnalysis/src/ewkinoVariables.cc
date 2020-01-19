#include "../interface/ewkinoVariables.h"

//include c++ library classes
#include <stdexcept>

//include other parts of framework


std::map< std::string, double > ewkino::computeVariables( Event& event, const std::string& unc ){
    Met variedMet; 
    double ht; 
    size_t numberOfJets;
    size_t numberOfBJets;
    if( unc == "nominal" ){
        variedMet = event.met();
        JetCollection variedCollection = event.jetCollection().goodJetCollection();
        ht = variedCollection.scalarPtSum();
        numberOfJets = variedCollection.size();
        numberOfBJets = variedCollection.numberOfTightBTaggedJets();
    } else if( unc == "JECDown" ){
        variedMet = event.met().MetJECDown();
        JetCollection variedCollection = event.jetCollection().JECDownCollection();
        variedCollection.selectGoodJets();
        ht = variedCollection.scalarPtSum();
        numberOfJets = variedCollection.size();
        numberOfBJets = variedCollection.numberOfTightBTaggedJets();
    } else if( unc == "JECUp" ){
        variedMet = event.met().MetJECUp();
        JetCollection variedCollection = event.jetCollection().JECUpCollection();
        variedCollection.selectGoodJets();
        ht = variedCollection.scalarPtSum();
        numberOfJets = variedCollection.size();
        numberOfBJets = variedCollection.numberOfTightBTaggedJets();
    } else if( unc == "JERDown" ){
        variedMet = event.met();
        JetCollection variedCollection = event.jetCollection().JERDownCollection();
        variedCollection.selectGoodJets();
        ht = variedCollection.scalarPtSum();
        numberOfJets = variedCollection.size();
        numberOfBJets = variedCollection.numberOfTightBTaggedJets();
    } else if( unc == "JERUp" ){
        variedMet = event.met();
        JetCollection variedCollection = event.jetCollection().JERUpCollection();
        variedCollection.selectGoodJets();
        ht = variedCollection.scalarPtSum();
        numberOfJets = variedCollection.size();
        numberOfBJets = variedCollection.numberOfTightBTaggedJets();
    } else if( unc == "UnclDown" ){
        variedMet = event.met().MetUnclusteredDown();
        JetCollection variedCollection = event.jetCollection().goodJetCollection();
        ht = variedCollection.scalarPtSum();
        numberOfJets = variedCollection.size();
        numberOfBJets = variedCollection.numberOfTightBTaggedJets();
    } else if( unc == "UnclUp" ){
        variedMet = event.met().MetUnclusteredUp();
        JetCollection variedCollection = event.jetCollection().goodJetCollection();
        ht = variedCollection.scalarPtSum();
        numberOfJets = variedCollection.size();
        numberOfBJets = variedCollection.numberOfTightBTaggedJets();
    } else {
        throw std::invalid_argument( "Uncertainty source " + unc + " is unknown." );
    }

    PhysicsObject leptonSum = event.leptonCollection().objectSum();
    std::map< std::string, double > ret = {
        { "met", variedMet.pt() },
        { "mll", event.bestZBosonCandidateMass() },
        { "mtW", mt( event.WLepton(), variedMet ) },
        { "ltmet", event.LT() + variedMet.pt() },
        { "m3l", leptonSum.mass() },
        { "mt3l", mt( leptonSum, variedMet ) },
        { "ht", ht },
        { "numberOfJets", numberOfJets },
        { "numberOfBJets", numberOfBJets }
    };
    return ret;
}
