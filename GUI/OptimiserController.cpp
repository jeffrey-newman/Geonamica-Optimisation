//
// Created by a1091793 on 8/7/17.
//

#include "OptimiserController.h"
#include "OptimiserWorker.h"
#include "GeonamicaPolicyParametersQtMetaTyping.hpp"
#include "MainWindow.hpp"

OptimiserController::OptimiserController(MainWindow * main_window)
: hypervolume_chart(new TimeSeriesQtChart),
  front_chart(new ParetoFrontQtChart),
  hypervolume_chart_view(new QtCharts::QChartView(hypervolume_chart, main_window)),
  paretofront_chart_view(new QtCharts::QChartView(front_chart, main_window))
{
    hypervolume_chart->setTitle("Plot of hypervolume growth by generation");
    hypervolume_chart->resize(QSizeF(600,400));
    front_chart->setTitle("Plot of Pareto fronts by generation");
    front_chart->resize(QSizeF(600,400));


    worker = new OptimiserWorker;
    worker->moveToThread(&optimiser_thread);
    QObject::connect(this, SIGNAL(testOptimisation()), worker, SLOT(test()));
    QObject::connect(this, SIGNAL(intialiseOptimisation(ZonalPolicyParameters)), worker, SLOT(initialise(ZonalPolicyParameters)));
    QObject::connect(this, SIGNAL(stepOptimisation()), worker, SLOT(step()));
    QObject::connect(this, SIGNAL(runOptimisation()), worker, SLOT(optimise()));

    QObject::connect(&optimiser_thread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    QObject::connect(this, SIGNAL(runOptimisation()), worker, SLOT(optimise()));
    QObject::connect(worker, SIGNAL(nextHypervolumeMetric(int, double)), hypervolume_chart, SLOT(addNextMetricValue(int, double)));
    QObject::connect(worker, SIGNAL(nextFront(int, QVector<std::pair<double, double> >)), front_chart,SLOT(addNewFront(int, QVector<std::pair<double, double> >)));
    QObject::connect(worker, SIGNAL(error(QString)), this, SLOT(onError(QString)));

    main_window->addPlot2MDIArea(hypervolume_chart_view);
    main_window->addPlot2MDIArea(paretofront_chart_view);

    optimiser_thread.start();
}

OptimiserController::~OptimiserController()
{
    //TODO terminate optimisation.
    worker->terminate();
    optimiser_thread.quit();
    optimiser_thread.wait();
}

void OptimiserController::onError(QString what)
{
    worker->terminate();
    QMessageBox msgBox;
    msgBox.setText( what);
    msgBox.exec();

}

void OptimiserController::initialise(ZonalPolicyParameters _params)
{
    emit intialiseOptimisation(_params);
}

void OptimiserController::run()
{
    hypervolume_chart_view->show();
    paretofront_chart_view->show();
    emit runOptimisation();
}

void OptimiserController::step()
{
    hypervolume_chart_view->show();
    paretofront_chart_view->show();
    emit stepOptimisation();
}

void OptimiserController::test()
{
    emit testOptimisation();
}





