//
// Created by a1091793 on 18/8/17.
//

#ifndef GEON_OPT_GEONAMICANSGAII_HPP
#define GEON_OPT_GEONAMICANSGAII_HPP

#include "NSGAII-CE.hpp"
#include "NSGAII.hpp"
#include "GeonamicaPolicyOptimiser.hpp"
#include <random>
#include <boost/shared_ptr.hpp>
#include "ParallelEvaluator.hpp"

class NSGAIIObjs
{
public:
    typedef  std::mt19937 RNG;
    boost::shared_ptr<RNG> rng;
    boost::shared_ptr<NSGAIIBase<RNG> > optimiser;
    NSGAIIObjs(boost::shared_ptr<RNG> _rng, boost::shared_ptr<NSGAIIBase<RNG> > _optimiser);
};

//class NSGAIICEObjsParallel
//{
//public:
//    typedef  std::mt19937 RNG;
//    unsigned seed;
//    RNG rng;
//    NSGAIICE<RNG> optimiser;
//    NSGAIICEObjsParallel(boost::mpi::environment & _mpi_env,
//                       boost::mpi::communicator & _world,
//                       ProblemDefinitionsSPtr _problem_defs);
//};
//
//class NSGAIIObjsParallel
//{
//public:
//    typedef  std::mt19937 RNG;
//    unsigned seed;
//    RNG rng;
//    NSGAII<RNG> optimiser;
//    NSGAIIObjsParallel(GeonamicaOptimiser & geon_eval);
//};

boost::shared_ptr<NSGAIIObjs>
getNSGAIIForGeon(GeonamicaOptimiser & geon_eval,
                 int rng_seed = std::chrono::system_clock::now().time_since_epoch().count());

boost::shared_ptr<NSGAIIObjs>
getNSGAIICEForGeonParallel(boost::mpi::environment & _mpi_env,
                         boost::mpi::communicator & _world,
                         ProblemDefinitionsSPtr _problem_defs,
                           int rng_seed = std::chrono::system_clock::now().time_since_epoch().count());

boost::shared_ptr<NSGAIIObjs>
getNSGAIIForGeonParallel(ParallelEvaluatePopServerNonBlocking & eval_server,
                         int rng_seed = std::chrono::system_clock::now().time_since_epoch().count());

#endif //GEON_OPT_GEONAMICANSGAII_HPP
