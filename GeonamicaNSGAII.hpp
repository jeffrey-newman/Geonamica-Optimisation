//
// Created by a1091793 on 18/8/17.
//

#ifndef GEON_OPT_GEONAMICANSGAII_HPP
#define GEON_OPT_GEONAMICANSGAII_HPP

#include "NSGAII.hpp"
#include "GeonamicaPolicyOptimiser.hpp"
#include <random>
#include <boost/shared_ptr.hpp>
#include "ParallelEvaluator.hpp"

class NSGAIIObjs
{
public:
    typedef  std::mt19937 RNG;
    unsigned seed;
    RNG rng;
    NSGAII<RNG> optimiser;
    NSGAIIObjs(GeonamicaOptimiser & geon_eval);
};

class NSGAIIObjsParallel
{
public:
    typedef  std::mt19937 RNG;
    unsigned seed;
    RNG rng;
    NSGAII<RNG> optimiser;
    NSGAIIObjsParallel(ParallelEvaluatePopServerNonBlocking & geon_eval);
};

boost::shared_ptr<NSGAIIObjs>
getNSGAIIForGeon(GeonamicaOptimiser & geon_eval);

boost::shared_ptr<NSGAIIObjsParallel>
getNSGAIIForGeonParallel(ParallelEvaluatePopServerNonBlocking & eval_server);

#endif //GEON_OPT_GEONAMICANSGAII_HPP
