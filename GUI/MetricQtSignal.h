//
// Created by a1091793 on 22/6/17.
//

#ifndef WDS_OPT_HYPERVOLUMECHECKPOINTQTSIGNAL_H
#define WDS_OPT_HYPERVOLUMECHECKPOINTQTSIGNAL_H

#include <QObject>
#include "Population.hpp"
#include "Checkpoint.hpp"
#include "Metrics/MetricBase.hpp"

class MetricQtSignal : public QObject, public CheckpointBase
{
    Q_OBJECT

    int num_gens;
    MetricBaseSPtr metric;

public:
    MetricQtSignal(MetricBaseSPtr _metric);

    bool
    operator ()(PopulationSPtr pop);

signals:
    void nextMetricValue(int gen, double metric_val);


};


#endif //WDS_OPT_HYPERVOLUMECHECKPOINTQTSIGNAL_H
