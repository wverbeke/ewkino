
//include class to test 
#include "../../objects/interface/PhysicsObject.h"

//include c++ library classes
#include <iostream>
#include <chrono>
#include <random>
#include <utility>

//include ROOT TLorentzVector for performance comparison
#include "TLorentzVector.h"


int main(){
    PhysicsObject a = PhysicsObject(0, 0, 0, 0);
    a.pt();
    return 0;
}
