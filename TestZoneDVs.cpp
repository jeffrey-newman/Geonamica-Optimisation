
//
//  main.cpp
//  MetronamicaCalibrator
//
//  Created by a1091793 on 9/05/2016.
//  Copyright © 2016 University of Adelaide and Bushfire and Natural Hazards CRC. All rights reserved.
//


//#include "NSGAII.hpp"
#include "GeonamicaPolicyParameters.hpp"
#include "GeonamicaPolicyOptimiser.hpp"


int main(int argc, char * argv[]) {

    ZonalPolicyParameters params;
    LoadParameters parameter_loader;
    parameter_loader.processOptions(argc, argv, params);
    GeonamicaOptimiser zonal_eval(params);


    // Initialise population
    PopulationSPtr pop(new Population);

    std::vector<int> zonal_all_restrict(zonal_eval.getProblemDefinitions()->int_lowerbounds.size(), 3);
    std::vector<int> zonal_all_permit(zonal_eval.getProblemDefinitions()->int_lowerbounds.size(), 1);
    std::vector<int> zonal_all_stimulate(zonal_eval.getProblemDefinitions()->int_lowerbounds.size(), 0);

    IndividualSPtr ind_all_restrict(new Individual(zonal_eval.getProblemDefinitions()));
    ind_all_restrict->setIntDVs(zonal_all_restrict);

    IndividualSPtr ind_all_stimulate(new Individual(zonal_eval.getProblemDefinitions()));
    ind_all_stimulate->setIntDVs(zonal_all_stimulate);

    IndividualSPtr ind_all_permit(new Individual(zonal_eval.getProblemDefinitions()));
    ind_all_permit->setIntDVs(zonal_all_permit);

    std::vector<double> objectives;
    std::vector<double> constraints;

    boost::filesystem::path save_ind_dir1 = params.save_dir.second / ("all_restrict");
    if (!boost::filesystem::exists(save_ind_dir1)) boost::filesystem::create_directory(save_ind_dir1);
    std::tie(objectives, constraints) = zonal_eval(ind_all_restrict->getRealDVVector(), ind_all_restrict->getIntDVVector(), save_ind_dir1);
    ind_all_restrict->setObjectives(objectives);
    ind_all_restrict->setConstraints(constraints);
    std::cout << "All zonal areas as restrict: " << std::endl;
    std::cout << *ind_all_restrict << std::endl;

    boost::filesystem::path save_ind_dir2 = params.save_dir.second / ("all_stimulate");
    if (!boost::filesystem::exists(save_ind_dir2)) boost::filesystem::create_directory(save_ind_dir2);
    std::tie(objectives, constraints) = zonal_eval(ind_all_stimulate->getRealDVVector(), ind_all_stimulate->getIntDVVector(), save_ind_dir2);
    ind_all_stimulate->setObjectives(objectives);
    ind_all_stimulate->setConstraints(constraints);
    std::cout << "All zonal areas as stimulate: " << std::endl;
    std::cout << *ind_all_stimulate << std::endl;

    boost::filesystem::path save_ind_dir3 = params.save_dir.second / ("all_permit");
    if (!boost::filesystem::exists(save_ind_dir3)) boost::filesystem::create_directory(save_ind_dir3);
    std::tie(objectives, constraints) = zonal_eval(ind_all_permit->getRealDVVector(), ind_all_permit->getIntDVVector(), save_ind_dir3);
    ind_all_permit->setObjectives(objectives);
    ind_all_permit->setConstraints(constraints);
    std::cout << "All zonal areas as permit: " << std::endl;
    std::cout << *ind_all_permit << std::endl;

//    //Cleanup
//    cleanup(params);

}
