//
// Created by a1091793 on 18/8/17.
//

#ifndef GEON_OPT_GEONAMICAPOLICYPOSTPROCESS_HPP
#define GEON_OPT_GEONAMICAPOLICYPOSTPROCESS_HPP

#include "GeonamicaPolicyOptimiser.hpp"
#include "GeonamicaPolicyParameters.hpp"
#include "Population.hpp"

void
postProcessResults(GeonamicaOptimiser & zonal_eval, PopulationSPtr pop, GeonamicaPolicyParameters & params, boost::filesystem::path save_dir = "", bool process_first_front = true);

#endif //GEON_OPT_GEONAMICAPOLICYPOSTPROCESS_HPP
