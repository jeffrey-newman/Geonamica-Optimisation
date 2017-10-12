//
// Created by a1091793 on 24/6/17.
//

#ifndef WDS_OPT_FRONTQTSIGNAL_H
#define WDS_OPT_FRONTQTSIGNAL_H

#include <QObject>
#include <QVector>
#include "Population.hpp"
#include "Checkpoint.hpp"


class FrontQtSignal : public QObject , public CheckpointBase
{
    Q_OBJECT

    int num_gens;

public:
    FrontQtSignal();

    bool
    operator()(PopulationSPtr pop);

signals:
    void nextFront(int gen, QVector<std::pair<double, double> > next_front);
};

//Q_DECLARE_METATYPE(QVector<std::pair<double, double> >)


#endif //WDS_OPT_FRONTQTSIGNAL_H
