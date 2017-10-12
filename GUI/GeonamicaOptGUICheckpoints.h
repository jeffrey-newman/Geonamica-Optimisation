//
// Created by a1091793 on 12/6/17.
//

#ifndef WDS_OPT_WDSOPTCHECKPOINTS_H
#define WDS_OPT_WDSOPTCHECKPOINTS_H

#include "GeonamicaPolicyParametersQtMetaTyping.hpp"
#include "NSGAII.hpp"
//#include "optimiserthread.h"
#include "OptimiserWorker.h"

template<typename RNG>
void
createCheckpointsQtGUI(NSGAII<RNG> & optimiser, ZonalPolicyParameters & params, OptimiserWorker * thread);

#endif //WDS_OPT_WDSOPTCHECKPOINTS_H
