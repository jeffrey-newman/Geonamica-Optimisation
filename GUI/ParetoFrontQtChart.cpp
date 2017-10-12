//
// Created by a1091793 on 24/6/17.
//

#include "ParetoFrontQtChart.h"

ParetoFrontQtChart::ParetoFrontQtChart(QGraphicsItem *parent, Qt::WindowFlags wFlags):
    QtCharts::QChart(QtCharts::QChart::ChartTypeCartesian, parent, wFlags),
//    m_axis(new QtCharts::QValueAxis),
    first_time(true),
    y_min(std::numeric_limits<qreal>::max()),
    y_max(std::numeric_limits<qreal>::min()),
    x_min(std::numeric_limits<qreal>::max()),
    x_max(std::numeric_limits<qreal>::min()),
    front(new QtCharts::QScatterSeries(this))
{
//    QtCharts::QScatterSeries dummy_front(this);
//    fronts.insert(0, dummy_front);
//    addSeries(&dummy_front);
//    createDefaultAxes();
//    setAxisX(m_axis, &dummy_front);
//    axisX()->setRange(x_min, x_max);
//    axisY()->setRange(y_min, y_max);
}


ParetoFrontQtChart::~ParetoFrontQtChart()
{

}

void ParetoFrontQtChart::addNewFront(int gen, QVector<std::pair<double, double> > new_front)
{

//    if (gen % 50 == 0)
//    {
//
//
//        QtCharts::QScatterSeries *new_front_series(new QtCharts::QScatterSeries(this));
//
//
//        std::for_each(new_front.begin(), new_front.end(),
//                      [&new_front_series](std::pair<double, double> &ind) {
//                          new_front_series->append(ind.first, ind.second);
//                      }
//        );
//
//        new_front_series->setName(QString::number(gen));
////        new_front_series->setParent(this);
//        this->addSeries(new_front_series);
//    }

//    if (!first_time)
//    {
//        this->removeSeries(front);
//        delete front;
//        front = new QtCharts::QScatterSeries();
//    }

    bool bounds_changed = false;
    front->clear();
//    front->setName(QString::number(gen));
    std::for_each(new_front.begin(), new_front.end(),
                  [this, &bounds_changed](std::pair<double, double> &ind) {
                      front->append(ind.first, ind.second);
                      if (ind.first > x_max) {x_max = ind.first; bounds_changed = true;}
                      if (ind.first < x_min) {x_min = ind.first; bounds_changed = true;}
                      if (ind.second > y_max) {y_max = ind.second; bounds_changed = true;}
                      if (ind.second < y_min) {y_min = ind.second; bounds_changed = true;}
                  });



    if (first_time)
    {
        this->addSeries(front);
        this->createDefaultAxes();
//        this->setAxisX(m_axis, front);
        first_time = false;
    }

    if(bounds_changed)
    {
        this->axisX()->setRange(0, x_max);
        this->axisY()->setRange(0, y_max);
    }




}

