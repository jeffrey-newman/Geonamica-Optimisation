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

    if (using_mpi == false)
    {
        try
        {
            geon_eval = boost::shared_ptr<GeonamicaOptimiser>(new GeonamicaOptimiser(params));

            //Make the NSGAII
            nsgaii_objs = getNSGAIIForGeon(*geon_eval);

            // Add the checkpoints
            createCheckpointsQtGUI(nsgaii_objs->optimiser, params, this);

            // Initialise population
            pop = PopulationSPtr(new Population);
            if (params.restart_pop_file.first == "no_seed")
            {
                pop = intialisePopulationRandomDVAssignment(params.pop_size, geon_eval->getProblemDefinitions(),
                                                            nsgaii_objs->rng);
            }
            else
            {
                pop = restore_population(params.restart_pop_file.second);
            }
            nsgaii_objs->optimiser.getIntMutationOperator().setMutationInverseDVSize(pop->at(0));

            nsgaii_objs->optimiser.initialiseWithPop(pop);
        }
//    nsgaii_objs->optimiser.log();
        catch (std::runtime_error err)
        {
            emit error(QString(err.what()));
            return;
        }
        is_initialised = true;
    }
    else
    {
        try
        {
            if (params.wine_prefix_path.first.substr(0, 4) != "copy")
            {
                emit error(QString(" Using parallel evaluation (mpi) but not making a 'copy' of the prefix for each evaluator. Are you sure?"));
            }

//            std::cout << "Broadcasting params\n";
            boost::mpi::broadcast(world, _params, 0);

            geon_eval = boost::shared_ptr<GeonamicaOptimiser>(new GeonamicaOptimiser(params));
            eval_server = boost::shared_ptr<ParallelEvaluatePopServerNonBlocking>
                                               (new ParallelEvaluatePopServerNonBlocking(env, world, geon_eval->getProblemDefinitions()));
            //Make the NSGAII
            nsgaii_objs_pll = getNSGAIIForGeonParallel(*eval_server);

            // Add the checkpoints
            createCheckpointsQtGUI(nsgaii_objs_pll->optimiser, params, this);

            // Initialise population
            pop = PopulationSPtr(new Population);
            if (params.restart_pop_file.first == "no_seed")
            {
                pop = intialisePopulationRandomDVAssignment(params.pop_size, geon_eval->getProblemDefinitions(),
                                                            nsgaii_objs_pll->rng);
            }
            else
            {
                pop = restore_population(params.restart_pop_file.second);
            }
            nsgaii_objs_pll->optimiser.getIntMutationOperator().setMutationInverseDVSize(pop->at(0));

            nsgaii_objs_pll->optimiser.initialiseWithPop(pop);
        }
//    nsgaii_objs->optimiser.log();
        catch (std::runtime_error err)
        {
            emit error(QString(err.what()));
            return;
        }
        is_initialised = true;

    }
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

        if (using_mpi == false)
        {
            do
            {
                nsgaii_objs->optimiser.step();
                mutex.lock();
                if (this->do_terminate) do_continue = false;
                mutex.unlock();
                if (nsgaii_objs->optimiser.isFinished()) do_continue = false;
            }
            while (do_continue);

//        nsgaii_objs->optimiser.run();

            //Postprocess the results
            if (nsgaii_objs->optimiser.isFinished()) postProcessResults(*geon_eval, pop, params);
        }
        else
        {
            do
            {
                nsgaii_objs_pll->optimiser.step();
                mutex.lock();
                if (this->do_terminate) do_continue = false;
                mutex.unlock();
                if (nsgaii_objs_pll->optimiser.isFinished()) do_continue = false;
            }
            while (do_continue);

//        nsgaii_objs->optimiser.run();

            //Postprocess the results
            if (nsgaii_objs_pll->optimiser.isFinished()) postProcessResults(*geon_eval, pop, params);
        }

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

        if (using_mpi == false)
        {
            // Run the optimisation
            nsgaii_objs->optimiser.step();

            //Postprocess the results
            if (nsgaii_objs->optimiser.isFinished()) postProcessResults(*geon_eval, pop, params);
        }
        else
        {
            // Run the optimisation
            nsgaii_objs_pll->optimiser.step();

            //Postprocess the results
            if (nsgaii_objs_pll->optimiser.isFinished()) postProcessResults(*geon_eval, pop, params);
        }https://support.google.com/a/answer/178723?hl=en

        std::cout << "Finished running optimisation" << std::endl;
    }
    catch (std::runtime_error err)
    {
        emit error(QString(err.what()));
    }

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

    std::vector<double> objectives;
    std::vector<double> constraints;

    boost::filesystem::path save_min_dv_dir = params_copy.test_dir.second / "min-dvs";
    if (!boost::filesystem::exists(save_min_dv_dir)) boost::filesystem::create_directories(save_min_dv_dir);
    std::tie(objectives, constraints) = testing_geon_eval(min_dvs->getRealDVVector(), min_dvs->getIntDVVector(), save_min_dv_dir);
    min_dvs->setObjectives(objectives);
    min_dvs->setConstraints(constraints);
    std::cout << "All dvs min value: " << std::endl;
    std::cout << *min_dvs << std::endl;

    boost::filesystem::path save_max_dv_dir = params_copy.test_dir.second / "max-dvs";
    if (!boost::filesystem::exists(save_max_dv_dir)) boost::filesystem::create_directories(save_max_dv_dir);
    std::tie(objectives, constraints) = testing_geon_eval(max_dvs->getRealDVVector(), max_dvs->getIntDVVector(), save_max_dv_dir);
    max_dvs->setObjectives(objectives);
    max_dvs->setConstraints(constraints);
    std::cout << "All dvs max value: " << std::endl;
    std::cout << *max_dvs << std::endl;

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 rng(seed);
    boost::filesystem::path save_rand_dv_dir = params_copy.test_dir.second / "random-dvs";
    if (!boost::filesystem::exists(save_max_dv_dir)) boost::filesystem::create_directories(save_rand_dv_dir);
    PopulationSPtr pop(new Population);
    pop =
        intialisePopulationRandomDVAssignment(params_copy.pop_size, testing_geon_eval.getProblemDefinitions(), rng);
    //Postprocess the results
    postProcessResults(testing_geon_eval, pop, params_copy, save_rand_dv_dir, false);
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



