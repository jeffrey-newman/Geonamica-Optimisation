#ifndef HYPERVOLUMECHART_H
#define HYPERVOLUMECHART_H

#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtCharts/QSplineSeries>

class TimeSeriesQtChart : public QtCharts::QChart
{
    Q_OBJECT

public:
    TimeSeriesQtChart(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~TimeSeriesQtChart();

public slots:
    void addNextMetricValue(int gen, double hyp_val);

private:
    QtCharts::QSplineSeries *m_series;
    QStringList m_titles;
    QtCharts::QValueAxis *m_axis;
    qreal m_step;
    qreal m_x;
    qreal m_y;
    qreal y_min;
    qreal y_max;
    qreal x_min;
    qreal x_max;
};

#endif // HYPERVOLUMECHART_H
