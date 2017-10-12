//
// Created by a1091793 on 18/8/17.
//

#ifndef GEON_OPT_GEONAMICANSGAII_HPP
#define GEON_OPT_GEONAMICANSGAII_HPP

#include "NSGAII.hpp"
#include "GeonamicaPolicyOptimiser.hpp"
#include <random>
#include <boost/shared_ptr.hpp>

class NSGAIIObjs
{
public:
    typedef  std::mt19937 RNG;
    unsigned seed;
    RNG rng;
    NSGAII<RNG> optimiser;
    NSGAIIObjs(GeonamicaOptimiser & geon_eval);
};

boost::shared_ptr<NSGAIIObjs>
getNSGAIIForGeon(GeonamicaOptimiser & geon_eval);

#endif //GEON_OPT_GEONAMICANSGAII_HPP
