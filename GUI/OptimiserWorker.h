//
// Created by a1091793 on 8/7/17.
//

#ifndef WDS_OPT_OPTIMISERWORKER_H
#define WDS_OPT_OPTIMISERWORKER_H

#include <QObject>
#include <QMutex>
#include <boost/shared_ptr.hpp>
#include <Population.hpp>
#include "GeonamicaPolicyParametersQtMetaTyping.hpp"
#include "../GeonamicaPolicyOptimiser.hpp"
#include "../GeonamicaNSGAII.hpp"

class OptimiserWorker : public QObject
{
    Q_OBJECT

public:
    OptimiserWorker();


public slots:
    void initialise(ZonalPolicyParameters params);
    void optimise();
    void step();
    void test();

signals:
    void nextHypervolumeMetric(int gen, double hypervolume_val);
    void nextFront(int gen, QVector<std::pair<double, double> > new_front);
    void nextPopulation(int gen, QVector<std::pair<double, double> > next_pop);
    void finishedOptimisation();
    void error(QString what);

private:
    QMutex mutex;
    ZonalPolicyParameters params;
    boost::shared_ptr<GeonamicaOptimiser> geon_eval;
    boost::shared_ptr<NSGAIIObjs> nsgaii_obs;
    PopulationSPtr pop;
    double count;
    bool is_initialised;

//private slots:
//    void relayNextHypervolumeMetric(int gen, double hypervolume_val);
//    void relayNextFront(int gen, QVector<std::pair<double, double> > new_front);
//    void relayNextPopulation(int gen, QVector<std::pair<double, double> > next_pop);

};


#endif //WDS_OPT_OPTIMISERWORKER_H
