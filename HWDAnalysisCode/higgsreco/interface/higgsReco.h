/*
Definition of Higgs reconstruction methods
*/

#ifndef higgsReco_H
#define higgsReco_H

// include other parts of framework
#include "../../../Event/interface/Event.h"
#include "../../../objects/interface/PhysicsObject.h"

// include other parts of the analysis code
#include "../../eventselection/interface/eventSelections.h"
#include "../../tools/interface/fakeRateTools.h"
#include "../../nureco/interface/nuReco.h"

namespace higgsReco{

    // help functions
    std::pair<PhysicsObject,PhysicsObject> makeHiggsAndNeutrinoCandidate(
        const Lepton& lepton, const Met& met, double nupz, const DMeson& dmeson);
    std::vector<std::pair<PhysicsObject,PhysicsObject>> makeHiggsAndNeutrinoCandidates(
        const Lepton& lepton, const Met& met, std::pair<double,double> nupzcands,
        const DMeson& dmeson);

    // higgs reconstruction functions
    std::vector<std::string> genericHiggsRecoMethods();
    std::pair<PhysicsObject,PhysicsObject> genericHiggsReco( const Event& event,
                                                             const std::string& method );
}

#endif
