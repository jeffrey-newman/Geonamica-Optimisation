 
//
//  main.cpp
//  MetronamicaCalibrator
//
//  Created by a1091793 on 9/05/2016.
//  Copyright © 2016 University of Adelaide and Bushfire and Natural Hazards CRC. All rights reserved.
//


#include "NSGAII.hpp"
#include "ZonalPolicyUtility.hpp"


int main(int argc, char * argv[]) {

    ZonalPolicyParameters params;
    processOptions(argc, argv, params);
    ZonalOptimiser zonal_eval(params);
    
    // The random number generator
    typedef std::mt19937 RNG;
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    RNG rng(seed);
    
    // The optimiser
    NSGAII<RNG> optimiser(rng, zonal_eval);
    
    // Add the checkpoints
    createCheckpoints(optimiser, params);
    
    
    // Initialise population
    PopulationSPtr pop = intialisePopulationRandomDVAssignment(params.pop_size, zonal_eval.getProblemDefinitions(), rng);
    optimiser.getIntMutationOperator().setMutationInverseDVSize(pop->at(0));
    
    // Run the optimisation
    optimiser(pop);
    
    //Postprocess the results
    postProcessResults(zonal_eval, pop, params);

    //Cleanup
    cleanup(params);

}
