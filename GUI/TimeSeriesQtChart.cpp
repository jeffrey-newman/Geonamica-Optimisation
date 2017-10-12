#include "TimeSeriesQtChart.h"
#include <iostream>

TimeSeriesQtChart::TimeSeriesQtChart(QGraphicsItem *parent, Qt::WindowFlags wFlags):
    QtCharts::QChart(QtCharts::QChart::ChartTypeCartesian, parent, wFlags),
    m_series(0),
    m_axis(new QtCharts::QValueAxis),
    m_step(0),
    m_x(5),
    m_y(1),
    y_min(0),
    y_max(0.1),
    x_min(0),
    x_max(10)

{
    m_series = new QtCharts::QSplineSeries(this);
    QPen green(Qt::red);
    green.setWidth(3);
    m_series->setPen(green);
    m_series->append(m_x, m_y);

    addSeries(m_series);
    createDefaultAxes();
    setAxisX(m_axis, m_series);
    m_axis->setTickCount(5);
    axisX()->setRange(x_min, x_max);
    axisY()->setRange(y_min, y_max);


}

TimeSeriesQtChart::~TimeSeriesQtChart()
{

}

void TimeSeriesQtChart::addNextMetricValue(int gen, double metric_val)
{
//    std::cout << "Adding next hypervolume metric to graph" << std::endl;
    m_x = gen;
    m_y = metric_val;

    //Check axis ranges
    if (gen > x_max)
    {
        x_max = gen * 1.30;
        axisX()->setRange(x_min, x_max);
    }

    if (metric_val > y_max)
    {
        y_max = metric_val * 1.30;
        axisY()->setRange(y_min, y_max);
    }

    if (metric_val < y_min) // as initially set to 0, this means a negative value....
    {
        y_min = metric_val * 1.30;
        axisY()->setRange(y_min, y_max);
    }

//    m_series->append(m_x, m_y);
//    qreal x = 1;
//    qreal y = (m_axis->max() - m_axis->min()) / m_axis->tickCount();
//    m_x += x;
//    m_y = qrand() % 5 - 2.5;
    m_series->append(m_x, m_y);
//    scroll(x, 0);
}
