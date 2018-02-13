/*
 * testing code for kinematic functions
 */

#include "../interface/kinematicTools.h"

//include c++ library classes
#include <vector>
#include <iostream>

//include ROOT classes
#include "TLorentzVector.h"


int main(){
    std::vector< std::vector < TLorentzVector > > dataset =  {
        {TLorentzVector(-2.64524, -51.6449, 71.1255, 87.9377),
        TLorentzVector(34.8851, 20.6965, 38.215, 55.7289),
        TLorentzVector(-28.57, 8.69936, -51.4941, 59.5278)},

        {TLorentzVector(-25.4431, 49.4119, -303.633, 308.678),
        TLorentzVector(12.5272, -27.1958, -114.727, 118.57),
        TLorentzVector(10.5973, -16.2084, -23.6206, 30.5444)},

        {TLorentzVector(4.62291, 56.0011, -121.041, 133.449),
        TLorentzVector(-5.15206, -35.4052, -42.05, 55.2113),
        TLorentzVector(-9.03261, 15.3078, 46.71, 49.9774)},

        {TLorentzVector(-5.92606, -32.7774, -113.086, 117.889),
        TLorentzVector(4.2551, 31.2552, -16.0162, 35.3767),
        TLorentzVector(2.04932, 7.02273, -5.35294, 9.06551)},

        {TLorentzVector(-25.7928, 9.42197, -12.5662, 30.1987),
        TLorentzVector(12.8743, -1.2025, -5.60615, 14.0934),
        TLorentzVector(-11.7851, 4.10623, -5.38374, 13.5921)}
    };
    std::vector<unsigned> indices = {0, 1 , 2};
    for(unsigned i = 0; i < dataset.size(); ++i){
        std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
        std::cout << "kinematics::minMass(dataset[" << i << "], indices) = " << kinematics::minMass(&dataset[i][0], indices) << std::endl;
        std::cout << "kinematics::maxMass(dataset[" << i << "], indices) = " << kinematics::maxMass(&dataset[i][0], indices) << std::endl;

        std::cout << "kinematics::minDeltaPhi(dataset[" << i << "], indices) = " << kinematics::minDeltaPhi(&dataset[i][0], indices) << std::endl;
        std::cout << "kinematics::maxDeltaPhi(dataset[" << i << "], indices) = " << kinematics::maxDeltaPhi(&dataset[i][0], indices) << std::endl;

        std::cout << "kinematics::minDeltaR(dataset[" << i << "], indices) = " << kinematics::minDeltaR(&dataset[i][0], indices) << std::endl;
        std::cout << "kinematics::maxDeltaR(dataset[" << i << "], indices) = " << kinematics::maxDeltaR(&dataset[i][0], indices) << std::endl;

        std::cout << "kinematics::minMT(dataset[" << i << "], indices) = " << kinematics::minMT(&dataset[i][0], indices) << std::endl;
        std::cout << "kinematics::maxMT(dataset[" << i << "], indices) = " << kinematics::maxMT(&dataset[i][0], indices) << std::endl;

        std::cout << "kinematics::minPT(dataset[" << i << "], indices) = " << kinematics::minPT(&dataset[i][0], indices) << std::endl;
        std::cout << "kinematics::maxPT(dataset[" << i << "], indices) = " << kinematics::maxPT(&dataset[i][0], indices) << std::endl;
    }



}
