/*
//
// Created by a1091793 on 8/7/17.
//
 */


#include "OptimiserWorker.h"
#include <iostream>
#include "GeonamicaOptGUICheckpoints.h"
#include "../GeonamicaPolicyPostProcess.hpp"
#include <stdexcept>
#include <QVector>
#include <QMessageBox>

template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}

OptimiserWorker::OptimiserWorker():
is_initialised(false)
{


}

void OptimiserWorker::initialise(ZonalPolicyParameters _params)
{
    count = 0;
    params = _params;

    try
    {
        geon_eval = boost::shared_ptr<GeonamicaOptimiser>(new GeonamicaOptimiser(params));
        //Make the NSGAII
        nsgaii_obs = getNSGAIIForGeon(*geon_eval);

        // Add the checkpoints
        createCheckpointsQtGUI(nsgaii_obs->optimiser, params, this);

        // Initialise population
        pop = PopulationSPtr(new Population);
        if (params.restart_pop_file.first == "no_seed")
        {
            pop = intialisePopulationRandomDVAssignment(params.pop_size, geon_eval->getProblemDefinitions(),
                                                        nsgaii_obs->rng);
        }
        else
        {
            pop = restore_population(params.restart_pop_file.second);
        }
        nsgaii_obs->optimiser.getIntMutationOperator().setMutationInverseDVSize(pop->at(0));

        nsgaii_obs->optimiser.initialisePop(pop);
    }
//    nsgaii_obs->optimiser.log();
    catch (std::runtime_error err)
    {
        emit error(QString(err.what()));
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
        nsgaii_obs->optimiser.run();

        //Postprocess the results
        if(nsgaii_obs->optimiser.isFinished()) postProcessResults(*geon_eval, pop, params);

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
        nsgaii_obs->optimiser.step();

        //Postprocess the results
        if(nsgaii_obs->optimiser.isFinished()) postProcessResults(*geon_eval, pop, params);

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



