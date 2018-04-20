/*
//
// Created by a1091793 on 8/7/17.
//
 */



#include <iostream>
#include "GeonamicaOptGUICheckpoints.h"
#include "../GeonamicaPolicyPostProcess.hpp"
#include <stdexcept>
#include <QVector>
#include <QMessageBox>
#include "RandomVector.hpp"

//template<typename Iter, typename RandomGenerator>
//Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
//    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
//    std::advance(start, dis(g));
//    return start;
//}
//
//template<typename Iter>
//Iter select_randomly(Iter start, Iter end) {
//    static std::random_device rd;
//    static std::mt19937 gen(rd());
//    return select_randomly(start, end, gen);
//}

OptimiserWorker::OptimiserWorker():
is_initialised(false)
{
    using_mpi = false;
//    std::cout << "world size: " << world.size() << std::endl;
    if (world.size() > 1) using_mpi = true;
}

void OptimiserWorker::initialise(GeonamicaPolicyParameters _params)
{
    count = 0;
    params = _params;

    try
    {
        if (using_mpi == true)
        {
            if (params.wine_prefix_path.first.substr(0, 4) != "copy")
            {
                emit error(QString(
                    " Using parallel evaluation (mpi) but not making a 'copy' of the prefix for each evaluator. Are you sure?"));
            }

//            std::cout << "Broadcasting params\n";
            boost::mpi::broadcast(world, _params, 0);
        }

        geon_eval = boost::shared_ptr<GeonamicaOptimiser>(new GeonamicaOptimiser(params));

//        boost::filesystem::path eval_log = params.save_dir.second / "evaluation_timing.log";
//        std::ofstream eval_strm(eval_log.c_str());

        if (params.algorithm == "NSGAII Proper")
        {
            if (using_mpi == true)
            {
                eval_server = boost::shared_ptr<ParallelEvaluatePopServerNonBlocking>
                    (new ParallelEvaluatePopServerNonBlocking(env, world, geon_eval->getProblemDefinitions()));
                if (params.is_logging)
                {
                    eval_server->log(params.save_dir.second, ParallelEvaluatorBase::LVL1);
                }
                nsgaii_objs = getNSGAIIForGeonParallel(*eval_server);
            }
            else
                {
                    nsgaii_objs = getNSGAIIForGeon(*geon_eval);
            }

        }
        else if (params.algorithm == "NSGAII - continuous evolution")
        {
            nsgaii_objs = getNSGAIICEForGeonParallel(env, world, geon_eval->getProblemDefinitions());
        }

        if (params.is_logging)
        {
            typedef  std::mt19937 RNG;
            nsgaii_objs->optimiser->log(params.save_dir.second, NSGAIIBase<RNG>::LVL1);
        }

        // Add the checkpoints
        createCheckpointsQtGUI(*nsgaii_objs->optimiser, params, this);

        // Initialise population
        pop = PopulationSPtr(new Population);
        if (params.restart_pop_file.first == "no_seed" || params.restart_pop_file.first.empty())
        {
            pop = intialisePopulationRandomDVAssignment(params.pop_size, geon_eval->getProblemDefinitions(),
                                                        *nsgaii_objs->rng);
        }
        else
        {
            pop = restore_population(params.restart_pop_file.second, geon_eval->getProblemDefinitions());
        }
        nsgaii_objs->optimiser->getIntMutationOperator().setMutationInverseDVSize(pop->at(0));

        nsgaii_objs->optimiser->initialiseWithPop(pop, params.save_dir.second);
//        nsgaii_objs->optimiser->savePop(pop, params.save_dir.second, "initial_pop");
    }
//    nsgaii_objs->optimiser.log();
    catch (std::runtime_error err)
    {
        emit error(QString(err.what()));
        return;
    }



    is_initialised = true;


}

void
OptimiserWorker::optimise()
{
    if (!is_initialised)
    {
        emit error(QString("Optimisation is not initialised"));
        return;
    }

    try{
    //Expensive calculation here.
    std::cout << "Running optimisation in thread\n";

        // Run the optimisation
        bool do_continue = true;


            do
            {
                nsgaii_objs->optimiser->step();
                mutex.lock();
                if (this->do_terminate) do_continue = false;
                mutex.unlock();
                if (nsgaii_objs->optimiser->isFinished()) do_continue = false;
            }
            while (do_continue);

//        nsgaii_objs->optimiser.run();

            //Postprocess the results
            if (nsgaii_objs->optimiser->isFinished())
                nsgaii_objs->optimiser->savePop(pop, params.save_dir.second, "final_pop");

        std::cout << "Finished running optimisation" << std::endl;
    }
    catch (std::runtime_error err)
    {

        emit error(QString(err.what()));
    }



}

void OptimiserWorker::step()
{
    if (!is_initialised)
    {
        emit error(QString("Optimisation is not initialised"));
        return;
    }

    try{
        //Expensive calculation here.
        std::cout << "Running optimisation in thread\n";


            // Run the optimisation
            nsgaii_objs->optimiser->step();

            //Postprocess the results
            if (nsgaii_objs->optimiser->isFinished())
                nsgaii_objs->optimiser->savePop(pop, params.save_dir.second, "final_pop");


        std::cout << "Finished running optimisation" << std::endl;
    }
    catch (std::runtime_error err)
    {
        emit error(QString(err.what()));
    }

}

void
OptimiserWorker::evalReseedPop()
{
    PopulationSPtr pop2process(restore_population(params.restart_pop_file.second, geon_eval->getProblemDefinitions()));
    nsgaii_objs->optimiser->savePop(pop2process, params.save_dir.second, "reseeded_pop");
}

void OptimiserWorker::test(GeonamicaPolicyParameters params_copy)
{
//    if (!is_initialised)
//    {
//        emit error(QString("Optimisation is not initialised"));
//        return;
//    }

    params_copy.working_dir.first = params_copy.test_dir.first;
    params_copy.working_dir.second = params_copy.test_dir.second;
    params_copy.save_dir.first = params_copy.test_dir.first;
    params_copy.save_dir.second = params_copy.test_dir.second;
    params_copy.pop_size = 3;

    GeonamicaOptimiser testing_geon_eval(params_copy);

    ProblemDefinitionsSPtr problem_defs = testing_geon_eval.getProblemDefinitions();
    IndividualSPtr max_dvs(new Individual(problem_defs));
    max_dvs->setIntDVs(problem_defs->int_upperbounds);
    max_dvs->setRealDVs(problem_defs->real_upperbounds);
    IndividualSPtr min_dvs(new Individual(problem_defs));
    min_dvs->setIntDVs(problem_defs->int_lowerbounds);
    min_dvs->setRealDVs(problem_defs->real_lowerbounds);

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 rng(seed);
    PopulationSPtr pop(new Population);
    pop =
        intialisePopulationRandomDVAssignment(params_copy.pop_size, testing_geon_eval.getProblemDefinitions(), rng);
    pop->push_back(max_dvs);
    pop->push_back(min_dvs);

    //Make the NSGAII
    boost::shared_ptr<NSGAIIObjs> testing_nsgaii_objs = getNSGAIIForGeon(testing_geon_eval);
    testing_nsgaii_objs->optimiser->savePop(pop, params_copy.test_dir.second, "tested_pop");
}

void OptimiserWorker::terminate()
{
    mutex.lock();
    do_terminate = true;
    mutex.unlock();
}





//void
//OptimiserWorker::relayNextHypervolumeMetric(int gen, double hypervolume_val)
//{
//    emit nextHypervolumeMetric(gen, hypervolume_val);
//}
//
//void
//OptimiserWorker::relayNextFront(int gen, QVector<std::pair<double, double> > new_front)
//{
//    emit nextFront(gen, new_front);
//}
//
//void
//OptimiserWorker::relayNextPopulation(int gen, QVector<std::pair<double, double> > next_pop)
//{
//    emit nextPopulation(gen, next_pop);
//}



