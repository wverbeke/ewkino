/*
Executable for testing the neutrino reconstruction
*/

// inlcude c++ library classes
#include <string>
#include <vector>
#include <exception>
#include <iostream>

// include other parts of framework
#include "../interface/nuReco.h"

typedef std::tuple<double,double,double> threevector;
typedef std::tuple<double,double,double,double> fourvector;

double getx( threevector t ){ return std::get<0>(t); }
double gety( threevector t ){ return std::get<1>(t); }
double getz( threevector t ){ return std::get<2>(t); }

double getx( fourvector f ){ return std::get<0>(f); }
double gety( fourvector f ){ return std::get<1>(f); }
double getz( fourvector f ){ return std::get<2>(f); }
double gete( fourvector f ){ return std::get<3>(f); }


double getm( fourvector f ){
    double px = getx(f);
    double py = gety(f);
    double pz = getz(f);
    double e = gete(f);
    double mass2 = e*e - px*px - py*py - pz*pz;
    double mass = 0;
    if( mass2>0 ) mass = std::sqrt(mass2);
    return mass;
}


double generate_random_double(){
    // generate one random number in double precision in (0,1)
    int n1 = std::rand()%1000+1;
    int n2 = std::rand()%1000+1;
    if(n1>n2) return (double)(n2)/n1;
    return (double)(n1)/n2;
}


threevector generate_random_vector(){
    // generate a random three-vector with components in (0,1)
    double px = generate_random_double();
    double py = generate_random_double();
    double pz = generate_random_double();
    return std::make_tuple( px, py, pz );
}


std::tuple<fourvector,fourvector,fourvector> generate_input( double mvis ){
    // make (truth-level) input for neutrino reconstruction
    
    // step 1: generate a random three-vector for the resonance
    threevector resvector = generate_random_vector();
    
    // step 2: generate a random three-vector for the visible particle
    threevector visvector = generate_random_vector();

    // step 3: calculate the three-vector of the neutrino
    threevector nuvector = { getx(resvector)-getx(visvector),
			     gety(resvector)-gety(visvector),
			     getz(resvector)-getz(visvector) };

    // step 4: calculate the four-vector of the visible particle with given mass
    double visenergy = std::sqrt( getx(visvector)*getx(visvector)
				+ gety(visvector)*gety(visvector)
				+ getz(visvector)*getz(visvector)
				+ mvis*mvis );
    fourvector visvec = { getx(visvector),
                          gety(visvector),
                          getz(visvector),
			  visenergy };

    // step 5: calculate the four-vector of the neutrino (with zero mass)
    double nuenergy = std::sqrt(  getx(nuvector)*getx(nuvector)
                                + gety(nuvector)*gety(nuvector)
                                + getz(nuvector)*getz(nuvector) );
    fourvector nuvec = { getx(nuvector),
                          gety(nuvector),
                          getz(nuvector), 
                          nuenergy };
    
    // step 6: calculate the resonance four-vector
    double resenergy = nuenergy + visenergy;
    fourvector resvec = { getx(resvector),
                             gety(resvector),
                             getz(resvector),
			     resenergy };
    
    return std::make_tuple( resvec, visvec, nuvec );
}


std::string to_string( fourvector f ){
    std::string msg = "fourvector";
    msg += ": px: " + std::to_string(getx(f));
    msg += ", py: " + std::to_string(gety(f));
    msg += ", pz: " + std::to_string(getz(f));
    msg += ", e: " + std::to_string(gete(f)); 
    msg += ", m: " + std::to_string(getm(f));
    return msg;
}


int main( int argc, char* argv[] ){
    std::cerr<<"###starting###"<<std::endl;
    if( argc != 1  ){
        return -1;
    }
    std::vector< std::string > argvStr( &argv[0], &argv[0] + argc );
    
    // generate input
    double mvis = 0.001;
    std::tuple<fourvector,fourvector,fourvector> input = generate_input( mvis );
    fourvector resvec = std::get<0>(input);
    fourvector visvec = std::get<1>(input);
    fourvector nuvec = std::get<2>(input);

    // replace by hard-coded input
    /*resvec = std::make_tuple(0, 0, 0, 80);
    visvec = std::make_tuple(0, 0, 40, 40);
    nuvec = std::make_tuple(0, 0, -40, 40);*/

    // print input
    std::cout << "input four-vectors:" << std::endl;
    std::cout << "resonance: " << to_string(resvec) << std::endl;
    std::cout << "visible: " << to_string(visvec) << std::endl;
    std::cout << "neutrino: " << to_string(nuvec) << std::endl;

    // call neutrino reconstruction
    std::tuple<bool, std::pair<double,double>> nusol;
    nusol = nuReco::pNuZCandidates( getx(visvec), gety(visvec), getz(visvec), 
				    gete(visvec), getm(visvec),
				    getx(nuvec), gety(nuvec),
				    getm(resvec) );

    // print neutrino solutions
    std::cout << "neutrino solutions (test method):" << std::endl;
    std::cout << "valid: " << std::get<0>(nusol) << std::endl;
    std::cout << "pnuz1: " << std::get<1>(nusol).first << std::endl;
    std::cout << "pnuz2: " << std::get<1>(nusol).second << std::endl;

    // call referene neutrino reconstruction
    std::tuple<bool, std::pair<double,double>> nusolref;
    nusolref = nuReco::pNuZCandidatesOld( getx(visvec), gety(visvec), getz(visvec),
                                          getx(nuvec), gety(nuvec), getm(resvec) );

    // print neutrino solutions
    std::cout << "neutrino solutions (reference method):" << std::endl;
    std::cout << "valid: " << std::get<0>(nusolref) << std::endl;
    std::cout << "pnuz1: " << std::get<1>(nusolref).first << std::endl;
    std::cout << "pnuz2: " << std::get<1>(nusolref).second << std::endl;
 
    return 0;
}
