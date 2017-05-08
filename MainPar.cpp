//
//  main.cpp
//  MetronamicaCalibrator
//
//  Created by a1091793 on 9/05/2016.
//  Copyright © 2016 University of Adelaide and Bushfire and Natural Hazards CRC. All rights reserved.
//

#include <iostream>
#include <string>
#include <random>
#include <chrono>
#include <boost/mpi.hpp>
#include <boost/timer/timer.hpp>
#include "ParallelEvaluator.hpp"
#include "NSGAII.hpp"
#include "GeonamicaPolicyUtility.hpp"
#include "GeonamicaPolicyOptimiser.hpp"

int main(int argc, char * argv[]) {
    boost::mpi::environment env(argc, argv);
    boost::mpi::communicator world;
    ZonalPolicyParameters params;
    //Sleep the threads so that they do not all try and create the same working directory at once, which could potentially cause havoc. This creation usually occurs in the evaluatior constructor but could also be placed in the command line option parser.
    std::this_thread::sleep_for(std::chrono::seconds(world.rank()));
    int ret = processOptions(argc, argv, params);
    if (ret == 1)
    {
        return 1;
    }

    params.evaluator_id = world.rank();
    GeonamicaOptimiser zonal_eval(params);
;
    
    
    if (world.rank() == 0)
    {

        boost::shared_ptr<boost::timer::auto_cpu_timer> timer(nullptr);
        boost::filesystem::path timing_fname = params.save_dir.second / "ZonalOptTiming.txt";
        std::ofstream timer_fs(timing_fname.c_str());
        if (timer_fs.is_open())
        {
            timer.reset(new boost::timer::auto_cpu_timer(timer_fs, 3));
        }
        else
        {
            std::cerr << "Error: Could not open file for writing time elapsed for search, using std::cout";
            timer.reset(new boost::timer::auto_cpu_timer(3));
        }

        //create evaluator server
        boost::filesystem::path eval_log = params.save_dir.second / "evaluation_timing.log";
        std::ofstream eval_strm(eval_log.c_str());
        ParallelEvaluatePopServerNonBlocking eval_server(env, world, zonal_eval.getProblemDefinitions());
        if (eval_strm.is_open())
        {
            eval_server.log(ParallelEvaluatorBase::LVL1, eval_strm);
        }

        // The random number generator
        typedef std::mt19937 RNG;
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        RNG rng(seed);

        // The optimiser
        NSGAII<RNG> optimiser(rng, eval_server);
        if (eval_strm.is_open())
        {
            optimiser.log(eval_strm, eval_log, NSGAII<RNG>::LVL1 );
        }
        
        // Add the checkpoints
        createCheckpoints(optimiser, params);
        

        PopulationSPtr pop(new Population);
        if (params.restart_pop_file.first == "no_seed")
        {
            pop = intialisePopulationRandomDVAssignment(params.pop_size, zonal_eval.getProblemDefinitions(), rng);
        }
        else
        {
            pop = restore_population(params.restart_pop_file.second);
        }

        optimiser(pop);


        //Postprocess the results
        postProcessResults(zonal_eval, pop, params);


        timer.reset((boost::timer::auto_cpu_timer *) nullptr);
        if (timer_fs.is_open())
        {
            timer_fs.close();
        }

    }
    else
    {
        std::string log_file_name = "worker_" + std::to_string(world.rank()) + "_timing.log";
        boost::filesystem::path eval_log = params.save_dir.second / log_file_name;
        std::ofstream eval_strm(eval_log.c_str());
        ParallelEvaluatePopClientNonBlocking eval_client(env, world, zonal_eval.getProblemDefinitions(), zonal_eval);
        if (eval_strm.is_open())
        {
            eval_client.log(ParallelEvaluatorBase::LVL1, eval_strm);
        }

        //logging eval_client
//        std::string log_filename = "evaluation_timing_worker" + std::to_string(world.rank()) + ".log";
//        boost::filesystem::path eval_log = working_dir.second / log_filename;
//        std::ofstream eval_strm(eval_log.c_str());
//        if (eval_strm.is_open())
//        {
//            eval_client.log(ParallelEvaluatorBase::LVL1, eval_strm);
//        }
        eval_client();
    }

    //Cleanup
//    cleanup(params);
}
