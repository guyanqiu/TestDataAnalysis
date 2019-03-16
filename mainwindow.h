#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <QListWidgetItem>
#include "datalog/datalog.h"
#include "grr/grr.h"
#include "correlation/correlation.h"
#include <QTableWidget>
#include <QMessageBox>
#include <map>
#include <QtCharts/QChart>
#include <QLabel>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum ChartType
    {
        LineChart,
        SPC_Chart,
        HistoChart,
        StatisticalBoxChart,
        NoneChart
    };
    struct TestItemInfo
    {
        TestItem* item;
        unsigned int site_index;
    };

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void OpenDataLogFile(const QString& filename);
    void OpenDataLogFiles();
    void ShowDataLogErrorMessage(DataLogError err);
    unsigned int GetSelectedSites();
    unsigned int GetSelectedItems();
    void PlotChart(ChartType mode);
    void InitAfterOpenFiles();
    void PlotChartOrShowData();
protected:
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);

private:
    void ClearDataLog(); 
    void ClearChartWidget();
    unsigned int PlotLineChart();
    void PlotSPCChart();
    void SetNewItemIndex(int max_count);
    void PlotHistoChart(unsigned int min_step, unsigned int max_step);
    void PlotStatisticalBoxChart();
    void ShowItemStatisticToTable();

    void SetGridCellValue(QTableWidget *table, int row, int column, const QString& value);
    void SetGridCellColor(QTableWidget *table,int row, int column, const QColor& color);

    bool ShowSiteStatisticToTable(QTableWidget *table, TestSite* site, int& start_row);

    bool CalculateGRR();
    QMessageBox::StandardButton ShowGRRErrorMessage(GRR_CALCULATE_ERROR err);
    bool ShowGRRDataToTable(QTableWidget* table, TestSite_GRR* grr);

    int GetSelectedCheckedIndex();
    bool CalculateCorrelation();
    QMessageBox::StandardButton ShowCorrErrorMessage(CORR_CALCULATE_ERROR err);
    bool ShowCorrDataToTable(QTableWidget* table, TestSite_CORR* corr);


private slots:
    void on_OpenDataLogButton_clicked();
    void on_LineButton_clicked();
    void on_TestSiteListWidget_itemChanged(QListWidgetItem *item);
    void on_TestItemListWidget_itemSelectionChanged();
    void on_HistoButton_clicked();
    void on_SPCButton_clicked();
    void on_ClearDataLogButton_clicked();
    void on_TestItemListWidget_clicked(const QModelIndex &index);
    void on_SaveChartButton_clicked();
    void on_MainTabWidget_currentChanged(int index);
    void on_GRRButton_clicked();
	void on_StatisticButton_clicked();
    void on_CorrelationButton_clicked();
    void on_TestSiteListWidget_clicked(const QModelIndex &index);
    void on_ChartSetButton_clicked();
    void on_DataSetButton_clicked();
    void on_AboutAction_triggered();
    void on_UpdateInfoAction_triggered();
    void on_actionSave_DataLog_to_CSV_triggered();
    void on_actionCheck_STDF_File_triggered();
    void on_actionConvert_DataLog_to_Summary_triggered();
    void on_actionShow_WaferMap_triggered();
    void on_UnSelectAllSiteButton_clicked();
    void on_SelectBySiteButton_clicked();
    void on_ScatterPlotButton_clicked();
    void on_StatisticalBoxButton_clicked();
    void on_slotPointHoverd(const QPointF &point, bool state);

private:
    ChartType mChartType;
    int mItemSelectedIndex;
    int mSiteSelectedIndex;
    int mTabPageIndex;

    unsigned int mMaxHistoGroupCount;
    unsigned int mMinHistoGroupCount;
    int mGRR_Mode ;
    int mCorr_Mode ;
    double mDefaultLowLimit;
    double mDefaultHighLimit;
    double mCorrelationSpec;

    Ui::MainWindow *ui;
    TestSite_GRR *mGrr;
    TestSite_CORR *mCorr;
    QtCharts::QChart* mChart;

    std::vector<DataLog*> mDataLogList;
    std::vector<TestSite*> mTestSiteList;
    std::vector<TestSite*> mSelectedSiteList;
    std::vector<TestItemInfo> mSeclectItemInfoList;

    QStringList mDataLogFileNames;
    QStringList mItemLabelList;
    QLabel xy_lable;

    std::map<unsigned int, QString> mSiteNumbers;

    void ClearAllSiteSelected();
    void SetSiteNumbers();
};

#endif // MAINWINDOW_H
