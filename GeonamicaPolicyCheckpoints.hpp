//
// Created by a1091793 on 18/8/17.
//

#ifndef GEON_OPT_GEONAMICAPOLICYCHECKPOINTS_HPP
#define GEON_OPT_GEONAMICAPOLICYCHECKPOINTS_HPP

#include "GeonamicaPolicyParameters.hpp"
#include "NSGAII.hpp"

template<typename RNG>
void
createCheckpoints(NSGAIIBase<RNG> & optimiser, GeonamicaPolicyParameters & params);


#endif //GEON_OPT_GEONAMICAPOLICYCHECKPOINTS_HPP
