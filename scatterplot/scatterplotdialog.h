#ifndef SCATTERPLOTDIALOG_H
#define SCATTERPLOTDIALOG_H

#include <QDialog>
#include <QtCharts/QChart>

#include "datalog/datalog.h"
#include <vector>

namespace Ui {
class ScatterPlotDialog;
}

class ScatterPlotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScatterPlotDialog(QWidget *parent = 0);
    ~ScatterPlotDialog();

    void AddTestSite(const std::vector<TestSite*>& site_list);
private:
    void ClearPlotData();
    void SetItemLabels();

private slots:
    void on_PlotButton_clicked();

    void on_SaveChartButton_clicked();

    void on_ExitButton_clicked();

    void on_comboBox_SiteName_currentIndexChanged(int index);

    void on_comboBox_XItemLable_currentIndexChanged(int index);

    void on_comboBox_YItemLabel_currentIndexChanged(int index);

private:
    Ui::ScatterPlotDialog *ui;
    std::vector<TestSite*> mSiteList;
    TestSite* mCurrentSite;
    TestItem* mCurrentXItem;
    TestItem* mCurrentYItem;
    bool chart_is_ok;
    bool site_is_exist;
    QtCharts::QChart* mChart;
};

#endif // SCATTERPLOTDIALOG_H
