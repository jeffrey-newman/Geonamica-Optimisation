//
// Created by a1091793 on 24/6/17.
//

#ifndef WDS_OPT_QTDIALOGUECHECKPOINT_H
#define WDS_OPT_QTDIALOGUECHECKPOINT_H

#include "Checkpoint.hpp"

class QtDialogueCheckpoint : public CheckpointBase
{
    int num_gens;

public:

    QtDialogueCheckpoint();

    bool
    operator()(PopulationSPtr pop);

};


#endif //WDS_OPT_QTDIALOGUECHECKPOINT_H
