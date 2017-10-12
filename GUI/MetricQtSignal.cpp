//
// Created by a1091793 on 22/6/17.
//

#include "MetricQtSignal.h"


MetricQtSignal::MetricQtSignal(MetricBaseSPtr _metric) :
    num_gens(0),
    metric(_metric)
{

}

bool
MetricQtSignal::operator ()(PopulationSPtr pop)
{
    ++num_gens;
    emit nextMetricValue(num_gens, metric->getVal());
    return true;
}

