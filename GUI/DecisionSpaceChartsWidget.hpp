//
// Created by a1091793 on 5/8/17.
//

#ifndef WDS_OPT_DECISIONSPACECHARTSWIDGET_HPP
#define WDS_OPT_DECISIONSPACECHARTSWIDGET_HPP

#include <QtWidgets/QWidget>
#include <QtCharts/QChartGlobal>

QT_CHARTS_BEGIN_NAMESPACE
    class QChartView;
    class QChart;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class DecisionSpaceChartsWidget : public QWidget
{
Q_OBJECT

public:
    explicit DecisionSpaceChartsWidget(QObject* parent = 0);
    ~DecisionSpaceChartsWidget();


};


#endif //WDS_OPT_DECISIONSPACECHARTSWIDGET_HPP
