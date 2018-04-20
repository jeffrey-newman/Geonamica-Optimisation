//
// Created by a1091793 on 8/7/17.
//

#ifndef WDS_OPT_OPTIMISERWORKER_H
#define WDS_OPT_OPTIMISERWORKER_H

#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

#include <QObject>
#include <QMutex>
#include <boost/shared_ptr.hpp>
#include <Population.hpp>
#include "GeonamicaPolicyParametersQtMetaTyping.hpp"
#include "../GeonamicaPolicyOptimiser.hpp"
#include "../GeonamicaNSGAII.hpp"
#include "ParallelEvaluator.hpp"


class OptimiserWorker : public QObject
{
    Q_OBJECT

public:
    OptimiserWorker();


public slots:
    void initialise(GeonamicaPolicyParameters params);
    void optimise();
    void step();
    void test(GeonamicaPolicyParameters params_copy);
    void evalReseedPop();
    void terminate();

signals:
    void nextHypervolumeMetric(int gen, double hypervolume_val);
    void nextFront(int gen, QVector<std::pair<double, double> > new_front);
    void nextPopulation(int gen, QVector<std::pair<double, double> > next_pop);
    void finishedOptimisation();
    void error(QString what);

private:
    QMutex mutex;
    GeonamicaPolicyParameters params;
    boost::shared_ptr<GeonamicaOptimiser> geon_eval;
    boost::shared_ptr<ParallelEvaluatePopServerNonBlocking> eval_server;
//    boost::shared_ptr<ParallelEvaluatePopServerNonBlockingContinuousEvolution> eval_server_ce;
    boost::shared_ptr<NSGAIIObjs> nsgaii_objs;
//    boost::shared_ptr<NSGAIICEObjsParallel> nsgaii_objs_pll;
    PopulationSPtr pop;
    double count;
    bool is_initialised;
    bool do_terminate;
    boost::mpi::environment env;
    boost::mpi::communicator world;
    bool using_mpi;

//private slots:
//    void relayNextHypervolumeMetric(int gen, double hypervolume_val);
//    void relayNextFront(int gen, QVector<std::pair<double, double> > new_front);
//    void relayNextPopulation(int gen, QVector<std::pair<double, double> > next_pop);

};


#endif //WDS_OPT_OPTIMISERWORKER_H
