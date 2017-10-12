//
// Created by a1091793 on 24/6/17.
//

#include "QtDialogueCheckpoint.h"
#include <QMessageBox>

QtDialogueCheckpoint::QtDialogueCheckpoint() :
num_gens(0)
{

}

bool
QtDialogueCheckpoint::operator()(PopulationSPtr pop)
{
    ++num_gens;
    QMessageBox msgBox;
    msgBox.setText(QStringLiteral("End of generation %1").arg(num_gens));
//    msgBox.exec();
    return true;
}
