#ifndef OPTIMISERTHREAD_H
#define OPTIMISERTHREAD_H

#include <QMutex>
#include <QThread>
#include <boost/scoped_ptr.hpp>
#include <Population.hpp>
#include "../WDSOptParameters.hpp"


class OptimiserThread : public QThread
{
    Q_OBJECT
public:
    OptimiserThread();

    void initialise(int argc, char *argv[]);
    void optimise(double _val);

signals:
    void nextHypervolumeMetric(int gen, double hypervolume_vaOl);
    void nextFront(int gen, QVector<std::pair<double, double> > new_front);


protected:
    void run() override;

private:
    QMutex mutex;
    boost::scoped_ptr<WDSOptParameters> params;
    double count;

private slots:
    void relayNextHypervolumeMetric(int gen, double hypervolume_val);
    void relayNextFront(int gen, QVector<std::pair<double, double> > new_front);

public slots:
    void runOptimisation(double _val);

};

#endif // OPTIMISERTHREAD_H
