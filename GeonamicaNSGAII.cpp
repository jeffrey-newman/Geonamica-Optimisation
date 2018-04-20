//
// Created by a1091793 on 18/8/17.
//

#include "GeonamicaNSGAII.hpp"

NSGAIIObjs::NSGAIIObjs(boost::shared_ptr<RNG> _rng, boost::shared_ptr<NSGAIIBase<RNG> > _optimiser):
    rng(_rng),
    optimiser(_optimiser)
//    optimiser(rng, geon_eval)
{

}

//NSGAIICEObjsParallel::NSGAIICEObjsParallel(boost::mpi::environment & _mpi_env,
//                                       boost::mpi::communicator & _world,
//                                       ProblemDefinitionsSPtr _problem_defs) :
//    seed(std::chrono::system_clock::now().time_since_epoch().count()),
//    rng(seed),
//    optimiser(rng,  _mpi_env, _world, _problem_defs)
//{
//
//}

boost::shared_ptr<NSGAIIObjs>
getNSGAIIForGeon(GeonamicaOptimiser & geon_eval, int rng_seed)
{
    typedef  std::mt19937 RNG;
    boost::shared_ptr<RNG> rng(new RNG(rng_seed));
    boost::shared_ptr<NSGAIIBase<RNG> > optimiser(new NSGAII<RNG>(*rng, geon_eval));
    boost::shared_ptr<NSGAIIObjs> nsgaii_objs(new NSGAIIObjs(rng, optimiser));
    return nsgaii_objs;
}

boost::shared_ptr<NSGAIIObjs>
getNSGAIICEForGeonParallel(boost::mpi::environment & _mpi_env,
                         boost::mpi::communicator & _world,
                         ProblemDefinitionsSPtr _problem_defs,
                           int rng_seed)
{
    typedef  std::mt19937 RNG;
    boost::shared_ptr<RNG> rng(new RNG(rng_seed));
    boost::shared_ptr<NSGAIIBase<RNG> > optimiser(new NSGAIICE<RNG>(*rng, _mpi_env, _world, _problem_defs));
    boost::shared_ptr<NSGAIIObjs> nsgaii_objs(new NSGAIIObjs(rng, optimiser));
    return nsgaii_objs;
}

boost::shared_ptr<NSGAIIObjs>
getNSGAIIForGeonParallel(ParallelEvaluatePopServerNonBlocking & eval_server, int rng_seed)
{
    typedef  std::mt19937 RNG;
    boost::shared_ptr<RNG> rng(new RNG(rng_seed));
    boost::shared_ptr<NSGAIIBase<RNG> > optimiser(new NSGAII<RNG>(*rng, eval_server));
    boost::shared_ptr<NSGAIIObjs> nsgaii_objs(new NSGAIIObjs(rng, optimiser));
    return nsgaii_objs;
}