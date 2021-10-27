/*
Tools for neutrino reconstruction
*/


// include header
#include "../interface/nuReco.h"


// internal helper functions

std::tuple<bool, std::pair<double,double>> solveQuadraticEquation(
	    double A, double B, double C ){

    // initialize the solution
    std::pair<double,double> sol = {0,0};

    // special case (will not happen in practice but for testing)
    if(A==0) return std::make_tuple( true, std::make_pair(-C/B, -C/B) );

    // make the discriminant
    double D = B*B - 4*A*C;
    bool isvalid = (D>0);
    if( !isvalid ) D = 0;
    
    // make the solutions
    sol.first = (-B + std::sqrt(D))/(2*A);
    sol.second = (-B - std::sqrt(D))/(2*A);
    return std::make_tuple( isvalid, sol );  
}


std::tuple<bool, std::pair<double,double>> nuReco::pNuZCandidates( 
	    const PhysicsObject vis, 
	    double mvis,
            const Met met, 
	    double mres ){
    double px = vis.px();
    double py = vis.py();
    double pz = vis.pz();
    double e = vis.energy();
    double metpx = met.px();
    double metpy = met.py();
    return nuReco::pNuZCandidates( px, py, pz, e, mvis, metpx, metpy, mres );
}


std::tuple<bool, std::pair<double,double>> nuReco::pNuZCandidates( 
            double px, double py, double pz, double e, double m,
            double metpx, double metpy,
	    double mres ){

    // make the coefficients
    double ptprod = px*metpx + py*metpy;
    double metpt2 = metpx*metpx + metpy*metpy;
    double A = e*e - pz*pz;
    double B = -pz*( 2*ptprod + mres*mres - m*m );
    double C = e*e*metpt2 - (1./4)*(mres*mres-m*m)*(mres*mres-m*m);
    C += -ptprod*ptprod - (mres*mres-m*m)*ptprod;

    // solve the equation
    return solveQuadraticEquation( A, B, C );
}


std::tuple<bool, std::pair<double,double>> nuReco::pNuZCandidatesOld(
	    const Event& event, 
	    const Lepton& lW){
    // older method copied from Willem's code.
    // only here for equivalency check with the one above!
    double plx = lW.px();
    double ply = lW.py();
    double plz = lW.pz();
    double pmx = event.met().px();
    double pmy = event.met().py();
    double mW = particle::mW;
    return nuReco::pNuZCandidatesOld( plx, ply, plz, pmx, pmy, mW );
}


std::tuple<bool, std::pair<double,double>> nuReco::pNuZCandidatesOld(
	    double plx, double ply, double plz,
	    double pmx, double pmy, 
	    double mW ){
    std::pair<double,double> pmz = {0,0};
    double plt = std::sqrt(plx*plx + ply*ply);
    double pl = std::sqrt(plt*plt + plz*plz);
    double pmt = std::sqrt(pmx*pmx + pmy*pmy);
    double a = 0.5*mW*mW + plx*pmx + ply*pmy;
    double D = 1-plt*plt*pmt*pmt/(a*a);
    double sqrtD = 0;
    bool isvalid = false;
    if(D>0){ sqrtD = std::sqrt(D); isvalid=true; }
    //else std::cout<<"### WARNING ###"<<std::endl;
    pmz.first = a*(plz + pl*sqrtD)/(plt*plt);
    pmz.second = a*(plz - pl*sqrtD)/(plt*plt);
    //std::cout<<pmz.first<<std::endl;
    //std::cout<<pmz.second<<std::endl;
    return std::make_tuple( isvalid, pmz );
}
