//
// Created by a1091793 on 18/8/17.
//

#include "GeonamicaNSGAII.hpp"

NSGAIIObjs::NSGAIIObjs(GeonamicaOptimiser &geon_eval):
    seed(std::chrono::system_clock::now().time_since_epoch().count()),
    rng(seed),
    optimiser(rng, geon_eval)
{

}

NSGAIIObjsParallel::NSGAIIObjsParallel(ParallelEvaluatePopServerNonBlocking &eval_server):
    seed(std::chrono::system_clock::now().time_since_epoch().count()),
    rng(seed),
    optimiser(rng, eval_server)
{

}

boost::shared_ptr<NSGAIIObjs>
getNSGAIIForGeon(GeonamicaOptimiser & geon_eval)
{
    boost::shared_ptr<NSGAIIObjs> nsgaii_objs(new NSGAIIObjs(geon_eval));
    return nsgaii_objs;
}

boost::shared_ptr<NSGAIIObjsParallel>
getNSGAIIForGeonParallel(ParallelEvaluatePopServerNonBlocking & eval_server)
{
    boost::shared_ptr<NSGAIIObjsParallel> nsgaii_objs_pll(new NSGAIIObjsParallel(eval_server));
    return nsgaii_objs_pll;
}