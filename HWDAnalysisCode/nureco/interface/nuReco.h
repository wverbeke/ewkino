/*
Tools for neutrino reconstruction
*/


#ifndef nuReco_h
#define nuReco_h

// include c++ library classes 
#include <string>
#include <exception>

// include other parts of the framework
#include "../../../objects/interface/PhysicsObject.h"
#include "../../../objects/interface/Met.h"
#include "../../../Event/interface/Event.h" // only for pNuZCandidatesOld
#include "../../../objects/interface/Lepton.h" // only for pNuZCandidatesOld
#include "../../../constants/particleMasses.h" // only for pNuZCandidatesOld

namespace nuReco{

    std::tuple<bool, std::pair<double,double>> pNuZCandidates( 
					    const PhysicsObject vis, 
					    double mvis,
					    const Met met, 
					    double mres );

    std::tuple<bool, std::pair<double,double>> pNuZCandidates( 
					    double px, double py, double pz, double e, double m,
					    double metpx, double metpy,
					    double mres );

    std::tuple<bool, std::pair<double,double>> pNuZCandidatesOld(
					    const Event& event,
					    const Lepton& lW);

    std::tuple<bool, std::pair<double,double>> pNuZCandidatesOld(
					    double plx, double ply, double plz,
					    double pmx, double pmy, double mW );

}

#endif
