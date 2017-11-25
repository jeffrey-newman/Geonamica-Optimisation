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

void OptimiserWorker::test()
{
    if (!is_initialised)
    {
        emit error(QString("Optimisation is not initialised"));
        return;
    }

    IndividualSPtr ind = *(select_randomly(pop->begin(), pop->end()));
    (*geon_eval)(ind->getRealDVVector(), ind->getIntDVVector());
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



