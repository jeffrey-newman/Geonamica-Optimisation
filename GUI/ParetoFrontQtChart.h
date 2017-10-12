//
// Created by a1091793 on 24/6/17.
//

#ifndef WDS_OPT_PARETOFRONTQTCHART_H
#define WDS_OPT_PARETOFRONTQTCHART_H

#include <QMap>
#include <QtCharts>
#include <QScatterSeries>
#include "Population.hpp"

class ParetoFrontQtChart : public  QtCharts::QChart
{
    Q_OBJECT

public:
    ParetoFrontQtChart(QGraphicsItem* parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~ParetoFrontQtChart();

public slots:
    void addNewFront(int gen, QVector<std::pair<double, double> > new_front);

private:
//    QMap<int, QtCharts::QScatterSeries*> historic_fronts;
    QtCharts::QScatterSeries * front;
    QStringList m_titles;
//    QtCharts::QValueAxis *m_axis;
    bool first_time;
    qreal y_min;
    qreal y_max;
    qreal x_min;
    qreal x_max;
};


#endif //WDS_OPT_PARETOFRONTQTCHART_H
