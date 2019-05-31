#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "statistic/statistics.h"
#include "grr/showgrrdialog.h"
#include "statistic/statisticdialog.h"
#include "correlation/corrlationdialog.h"
#include "widgets/chartsettingdialog.h"
#include "widgets/calsettingdialog.h"
#include "widgets/convertfiledialog.h"
#include "stdfreader//stdfdialog.h"
#include "wafermap/wafermapdialog.h"
#include "scatterplot/scatterplotdialog.h"

#include <iostream>
#include <QList>
#include <QIcon>
#include <QInputDialog>

#include <QUrl>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileDialog>
#include <QtCharts>

QT_CHARTS_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":images/TestDataAnalysis.png"));

    ui->ChartPlotWidget->setAcceptDrops(false);
    setAcceptDrops(true);

    mDefaultHighLimit = 9999.9;
    mDefaultLowLimit = -9999.9;
    mCorrelationSpec = 0.1;
    mChartType = NoneChart;
    mItemSelectedIndex = 0;
    mTabPageIndex = 0;
    mSiteSelectedIndex = -1;
    mMaxHistoGroupCount = 99;
    mMinHistoGroupCount = 13;
    mGRR_Mode = 0;

    mGrr = new TestSite_GRR();
    mCorr = new TestSite_CORR();
    mChart = new QChart(nullptr);


    ui->ChartPlotWidget->setRenderHint(QPainter::Antialiasing);
    ui->ChartPlotWidget->setRubberBand(QChartView::RectangleRubberBand);
    ui->ChartPlotWidget->setChart(mChart);

    xy_lable.setParent(ui->ChartPlotWidget);
    xy_lable.hide();
}

MainWindow::~MainWindow()
{
    ClearDataLog();
    delete ui;
    delete mGrr;
    delete mCorr;
    delete mChart;
}

void MainWindow::on_OpenDataLogButton_clicked()
{
    OpenDataLogFiles();
    InitAfterOpenFiles();
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if(event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if(urls.isEmpty()) return;
    foreach(QUrl url, urls)
    {
        QString filename = url.toLocalFile();
        OpenDataLogFile(filename);
    }
    GetSelectedSites();
    InitAfterOpenFiles();
}

void MainWindow::PlotChartOrShowData()
{
    switch(mTabPageIndex)
    {
    case 0:
        PlotChart(mChartType);
        break;
    case 1:
        ShowItemStatisticToTable();
        break;
    default: break;
    }
}

void MainWindow::InitAfterOpenFiles()
{
    int count = ui->TestItemListWidget->count();
    if(count <= 0) return;
    SetNewItemIndex(count);
    if(mChartType == NoneChart) mChartType = SPC_Chart;
    GetSelectedItems();
    PlotChartOrShowData();
}

void MainWindow::on_ClearDataLogButton_clicked()
{
    ui->TestItemListWidget->clear();
    ui->TestSiteListWidget->clear();

    ClearDataLog();
    ClearChartWidget();
    ui->DataTableWidget->clear();
}

void MainWindow::on_LineButton_clicked()
{
    mChartType = LineChart;
    PlotChart(LineChart);
}

void MainWindow::on_HistoButton_clicked()
{
    mChartType = HistoChart;
    PlotChart(HistoChart);
}

void MainWindow::on_SPCButton_clicked()
{
    mChartType = SPC_Chart;
    PlotChart(SPC_Chart);
}

void MainWindow::on_StatisticalBoxButton_clicked()
{
    mChartType = StatisticalBoxChart;
    PlotChart(StatisticalBoxChart);
}

void MainWindow::SetNewItemIndex(int max_count)
{
    if(mItemSelectedIndex < max_count)
    {
         ui->TestItemListWidget->setCurrentRow(mItemSelectedIndex);
    }
    else
    {
         mItemSelectedIndex = 0;
         ui->TestItemListWidget->setCurrentRow(mItemSelectedIndex);
    }
}


void MainWindow::on_TestSiteListWidget_itemChanged(QListWidgetItem *item)
{
    Q_UNUSED(item);
    unsigned int count = GetSelectedSites();
    if(count == 0)
    {
        ClearChartWidget();
        ui->TestItemListWidget->clear();
        return;
    }
    SetNewItemIndex(count);
}

void MainWindow::on_TestSiteListWidget_clicked(const QModelIndex &index)
{
    mSiteSelectedIndex = index.row();
}

void MainWindow::on_TestItemListWidget_clicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    GetSelectedItems();
    PlotChartOrShowData();
}

void MainWindow::on_TestItemListWidget_itemSelectionChanged()
{
    GetSelectedItems();
    PlotChartOrShowData();
}

void MainWindow::on_SaveChartButton_clicked()
{
    int nItems = mSeclectItemInfoList.size();
    if(nItems == 0)
    {
        QMessageBox::information(this,tr("No Plot to Save"), tr("No Data in Plot, No Plot to Save."),QMessageBox::Ok);
        return;
    }

    QFileDialog dialog(this);
    dialog.setWindowTitle(tr("Save PNG File"));
    dialog.setNameFilter(tr("PNG Files(*.png)"));
    dialog.setDefaultSuffix("png");
    dialog.setAcceptMode(QFileDialog::AcceptSave);

    if(dialog.exec() == QDialog::Accepted)
    {
        QString filename = dialog.selectedFiles()[0];
        QPixmap pix = ui->ChartPlotWidget->grab();
        if(pix.save(filename))
        {
            QMessageBox::information(this,tr("Save Chart Success"), tr("Save Chart to PNG File Success."),QMessageBox::Ok);
        }
        else
        {
            QMessageBox::critical(this,tr("Save Chart Failure"), tr("Save Chart to PNG File Failure."),QMessageBox::Ok);
        }
    }
}

void MainWindow::on_ChartSetButton_clicked()
{
    ChartSettingDialog dialog(this);
    dialog.SetDefaultHighLimit(mDefaultHighLimit);
    dialog.SetDefaultLowLimit(mDefaultLowLimit);
    dialog.SetMaxHistoCount(mMaxHistoGroupCount);
    dialog.SetMinHistoCount(mMinHistoGroupCount);
    int ret = dialog.exec();
    if(ret == QDialog::Accepted)
    {
        mDefaultHighLimit = dialog.GetDefaultHighLimit();
        mDefaultLowLimit = dialog.GetDefaultLowLimit();
        mMaxHistoGroupCount = dialog.GetMaxHistoCount();
        mMinHistoGroupCount = dialog.GetMinHistoCount();
        PlotChart(mChartType);
    }
}

void MainWindow::on_DataSetButton_clicked()
{
    CalSettingDialog dialog(this);
    dialog.SetDefaultHighLimit(mDefaultHighLimit);
    dialog.SetDefaultLowLimit(mDefaultLowLimit);
    dialog.SetEVMode(mGRR_Mode);
    dialog.SetCorrelationMode(mCorr_Mode);
    dialog.SetCorrelationSpec(mCorrelationSpec*100.0);
    int ret = dialog.exec();
    if(ret == QDialog::Accepted)
    {
        mDefaultHighLimit = dialog.GetDefaultHighLimit();
        mDefaultLowLimit = dialog.GetDefaultLowLimit();
        mCorrelationSpec = dialog.GetCorrelationSpecValue();
        mGRR_Mode = dialog.GetEVMode();
        mCorr_Mode = dialog.GetCorrelationMode();
        ShowItemStatisticToTable();
    }
}

void MainWindow::on_MainTabWidget_currentChanged(int index)
{
    mTabPageIndex = index;
    PlotChartOrShowData();
}

void MainWindow::on_GRRButton_clicked()
{
    if(CalculateGRR())
    {
        ShowGRRDialog dialog(this);
        QTableWidget *table = dialog.GetGRRTable();
        ShowGRRDataToTable(table, mGrr);
        dialog.SetItemToList(mItemLabelList);
        dialog.SetSitesGRR(mGrr);
        dialog.exec();
    }
}

void MainWindow::on_CorrelationButton_clicked()
{
    if(CalculateCorrelation())
    {
        CorrlationDialog dialog(this);
        QTableWidget *table = dialog.GetCorrelationTable();
        ShowCorrDataToTable(table, mCorr);
        dialog.SetSitesCorr(mCorr);
        dialog.exec();
    }
}

void MainWindow::on_StatisticButton_clicked()
{
    unsigned int site_count = mSelectedSiteList.size();
    if(site_count == 0) return;

    StatisticDialog dialog(this);
    dialog.SetSiteList(mSelectedSiteList);
    dialog.SetDefaultLimit(mDefaultLowLimit, mDefaultHighLimit);
    QTableWidget* table = dialog.GetStatisticTable();

    table->clear();
    table->setRowCount(0);
    table->setColumnCount(10);

    int start_row = 0;
    for(unsigned int i = 0; i < site_count; i++)
    {
        TestSite* site = mSelectedSiteList[i];
        ShowSiteStatisticToTable(table,site, start_row);
    }
    dialog.exec();
}

void MainWindow::on_actionSave_DataLog_to_CSV_triggered()
{
    ConvertFileDialog dialog(this);
    dialog.SetDataLogs(mDataLogFileNames);
    int ret = dialog.exec();
    if(ret == QDialog::Accepted)
    {
        int index = dialog.GetCurrentIndex();
        if(index >= 0)
        {
            DataLog* datalog = mDataLogList[index];
            QFileDialog file_dialog(this);
            file_dialog.setWindowTitle(tr("Save PowerTECH CSV File"));
            file_dialog.setNameFilter(tr("CSV Files(*.csv)"));
            file_dialog.setDefaultSuffix("csv");
            file_dialog.setAcceptMode(QFileDialog::AcceptSave);

            if(file_dialog.exec() == QDialog::Accepted)
            {
                QString filename = file_dialog.selectedFiles()[0];
                DataLogError err = datalog->write_data_to_csv(filename.toLocal8Bit().data());
                if(err == DATALOG_OPERATE_OK)
                    QMessageBox::information(this,tr("Save File Success"), tr("Save Test Datas to CSV File Success."),QMessageBox::Ok);
                else
                    ShowDataLogErrorMessage(err);
            }
        }
    }
}

void MainWindow::on_actionCheck_STDF_File_triggered()
{
    StdfDialog dialog(this);
    dialog.exec();
}

void MainWindow::ClearDataLog()
{
    int datalog_count = mDataLogList.size();
    for(int i = 0; i < datalog_count; i++)
    {
        delete mDataLogList[i];
    }
    mDataLogList.clear();
    mTestSiteList.clear();
    mSelectedSiteList.clear();
    mSeclectItemInfoList.clear();
    mDataLogFileNames.clear();
}

void MainWindow::OpenDataLogFile(const QString& filename)
{
	DataLog* datalog = new DataLog();
	DataLogError err = datalog->read_datalog(filename.toLocal8Bit().data());
    if(err == DATALOG_OPERATE_OK)
	{
		mDataLogList.push_back(datalog);
		QString datalog_name = QString::fromLocal8Bit(datalog->get_name());
        mDataLogFileNames.push_back(datalog_name);
		
		unsigned int site_count = datalog->get_site_count();
        for(unsigned int i = 0; i < site_count; i++)
        {
            TestSite* current_site = datalog->get_site(i);
            mTestSiteList.push_back(current_site);
			
			QString site_name = QString::fromLocal8Bit(current_site->get_name());
            QListWidgetItem *list_item = new QListWidgetItem(site_name);
            list_item->setCheckState(Qt::Checked);
            ui->TestSiteListWidget->addItem(list_item);
            mSelectedSiteList.push_back(current_site);
        }
    }
    else
    {
        delete datalog;
        ShowDataLogErrorMessage(err);
    }
}

void MainWindow::OpenDataLogFiles()
{
    QString title = tr("Open DataLog File");
    QString default_dir = tr("");

    //QString filter = tr("STDF V4 File (*.stdf *.std*);;ETS LOG File (*.log);;PowTech CSV File(*.csv"));
    QStringList filters;
    filters << tr("STDF V4 Files (*.stdf *.std)")
            << tr("ETS LOG Files (*.log)")
            << tr("PowTech CSV Files (*.csv)")
            << tr("PowTech TXT Files (*.txt)");

    QFileDialog *file_dialog = new QFileDialog(this, title, default_dir);
    file_dialog->setNameFilters(filters);
    file_dialog->setFileMode(QFileDialog::ExistingFiles);
    if(file_dialog->exec() == QDialog::Accepted)
    {
         QStringList filenames_list = file_dialog->selectedFiles();
         int file_count =  filenames_list.size();
         for(int i = 0; i < file_count; i++)
         {
             QString filename = filenames_list[i];
             OpenDataLogFile(filename);
         }
    }
    GetSelectedSites();
}


unsigned int MainWindow::GetSelectedSites()
{
    mSelectedSiteList.clear();
    unsigned int list_item_count = ui->TestSiteListWidget->count();
    unsigned int site_count = mTestSiteList.size();
    unsigned int count = 0;

    if(list_item_count != site_count)
    {
        QMessageBox::critical(this, tr("Site Count Error"),
                              tr("Read Site Count != Show Site Count. Clear the Files and Reload."), QMessageBox::Ok);
        return count;
    }
    for(unsigned int n = 0; n < list_item_count; n++)
    {
          QListWidgetItem* list_item = ui->TestSiteListWidget->item(n);
          if(list_item->checkState() == Qt::Checked)
          {
              mSelectedSiteList.push_back(mTestSiteList[n]);
              count++;
          }
    }

    if(count > 0)
    {
        ui->TestItemListWidget->clear();
        TestSite* first_site = mSelectedSiteList[0];
        unsigned int item_count = first_site->get_item_count();
        for(unsigned int i = 0; i < item_count; i++)
        {
            TestItem* test_item = first_site->get_item(i);
            QString item_label = QString::fromLocal8Bit(test_item->get_label());
            ui->TestItemListWidget->addItem(item_label);
        }
    }
    return count;
}

unsigned int MainWindow::GetSelectedItems()
{
    unsigned int count = 0;
    QListWidgetItem* row_item = ui->TestItemListWidget->currentItem();
    if(row_item == NULL) return count;

    QString item_label = row_item->text();

    mSeclectItemInfoList.clear();
    unsigned int site_count = mSelectedSiteList.size();
    if(site_count == 0) return count;
    for(unsigned int s = 0; s < site_count; s++)
    {
        TestSite* site = mSelectedSiteList[s];
        TestItem* item = site->get_item_by_label(item_label.toLocal8Bit().data());
        if(item)
        {
            TestItemInfo item_info;
            item_info.item = item;
            item_info.site_index = s;
            mSeclectItemInfoList.push_back(item_info);
            count++;
        }
    }
    return count;
}


void MainWindow::ClearChartWidget()
{
    QList<QAbstractAxis*> axisList = mChart->axes();
    foreach (QAbstractAxis *axis, axisList)
    {
          mChart->removeAxis(axis);
    }

    mChart->removeAllSeries();
    mChart->setTitle("");
}

void MainWindow::PlotChart(ChartType mode)
{
    switch(mode)
    {
    case LineChart:
        PlotLineChart();
        break;
    case SPC_Chart:
        PlotSPCChart();
        break;

    case HistoChart:
        PlotHistoChart(mMinHistoGroupCount, mMaxHistoGroupCount);
        break;
    case StatisticalBoxChart:
        PlotStatisticalBoxChart();
        break;
    default:
        break;
    }
}


unsigned int MainWindow::PlotLineChart()
{
    ClearChartWidget();
    unsigned int site_count = mSelectedSiteList.size();
    if(site_count == 0) return 0;
    unsigned int item_count = mSeclectItemInfoList.size();
    if(item_count == 0) return 0;

    unsigned int max_device_count = 0;
    for(unsigned int s = 0; s < site_count; s++)
    {
        TestSite* site = mSelectedSiteList[s];
        unsigned int device_count = site->get_device_count();
        if(device_count > max_device_count)
            max_device_count = device_count;
    }
    double min_result = 0.0, max_result = 0.0;
    QString item_label;

    QValueAxis *axisX = new QValueAxis;
    axisX->setRange(1, max_device_count);
    axisX->setLabelFormat("%.0f");
    axisX->setTitleText(tr("#Index"));
    axisX->setGridLineVisible(true);

    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText(item_label);
    axisY->setLabelFormat("%.6f");
    axisY->setGridLineVisible(true);

    for(unsigned int i = 0; i < item_count; i++)
    {
        TestItemInfo item_info = mSeclectItemInfoList[i];
        TestItem* item = item_info.item;
        TestSite* site = mSelectedSiteList[item_info.site_index];

        double min_temp = item->get_min();
        double max_temp = item->get_max();
        unsigned int result_count = item->get_result_count();

        if(i == 0)
        {
            min_result = min_temp;
            max_result = max_temp;
            item_label = QString::fromLocal8Bit(item->get_label());
            const char* unit = item->get_unit();
            if(unit) item_label += QString(tr("(Unit: %1)").arg(unit));
            mChart->setTitle(item_label);

        }
        else
        {
            if(min_result > min_temp) min_result = min_temp;
            if(max_result < max_temp) max_result = max_temp;
        }

        QLineSeries *series=new QLineSeries;
        connect(series, &QLineSeries::hovered, this, &on_slotPointHoverd);

        for(unsigned int n = 0; n < result_count; n++)
        {
            TestResult result = item->get_result(n);
            series->append(result.get_index(), result.get_value());
        }
        QString site_name = QString::fromLocal8Bit(site->get_name());
        series->setName(site_name);
        mChart->addSeries(series);

        QPen pen;
        pen.setWidth(3);
        int rand1 = i % 16 + 1;
        int rand2 = i % 4 + 1;
        int rand3 = i % 2 + 1;
        QColor color(15 * rand1 , 63 * rand2 , 127 * rand3);
        pen.setColor(color);
        series->setPen(pen);
        mChart->setAxisX(axisX, series);
        mChart->setAxisY(axisY, series);
    }

    axisY->setRange(min_result, max_result);
    mChart->legend()->show();

    return max_device_count;
}

void MainWindow::on_slotPointHoverd(const QPointF &point, bool state)
{
    if (state)
    {
        xy_lable.setText(QString::asprintf("(%f,%f)", point.x(),point.y()));
        QPoint curPos = mapFromGlobal(QCursor::pos());
        xy_lable.move(curPos.x()-xy_lable.width()*1.9 , curPos.y()-xy_lable.height()*2.9);
        xy_lable.show();
    }
    else
        xy_lable.hide();
}

void MainWindow::PlotHistoChart(unsigned int min_step, unsigned int max_step)
{
    ClearChartWidget();
    unsigned int site_count = mSelectedSiteList.size();
    if(site_count == 0) return ;
    unsigned int item_count = mSeclectItemInfoList.size();
    if(item_count == 0) return ;

    MyHistogram histogram(min_step, max_step);
    double max_x_value, min_x_value;
    for(unsigned int i = 0; i < item_count; i++)
    {
        TestItemInfo item_info = mSeclectItemInfoList[i];
        TestItem* item = item_info.item;

        double temp_mean = item->get_average();
        double temp_stdev = item->get_stdev();
        double temp_max = item->get_max();
        double temp_min = item->get_min();
        double temp_upl = temp_mean + 3.0*temp_stdev;
        double temp_lpl = temp_mean - 3.0*temp_stdev;

        if(temp_max < temp_upl) temp_max = temp_upl;
        if(temp_min > temp_lpl) temp_min = temp_lpl;

        if(i == 0)
        {
            max_x_value = temp_max;
            min_x_value = temp_min;
        }
        else
        {
            if(max_x_value < temp_max)   max_x_value = temp_max;
            if(min_x_value > temp_min)   min_x_value = temp_min;
        }

        histogram.AddItem(item);
    }

    unsigned int group_count = histogram.GetHistogramGroupCount();
    double min_value = histogram.GetMinValue();
    double max_value = histogram.GetMaxValue();

    if(max_x_value > max_value) max_value = max_x_value;
    if(min_x_value < min_value) min_value = min_x_value;

    double max_range = max_value - min_value;
    if(max_range == 0) max_range = 1.0;

    double step_value = max_range / (group_count-1);

    QBarSeries* bars = new QBarSeries();
    bars->setBarWidth(0.9);

    QValueAxis *axisX = new QValueAxis;
    axisX->setRange(min_value, max_value);
    axisX->setLabelsAngle(90);
    axisX->setLabelFormat("%.6f");

    QValueAxis *axisY = new QValueAxis;
    axisY->setLabelFormat("%.0f");

    QString XTestLable;
    double max_count_total = 0;
    for(unsigned int i = 0; i < item_count; i++)
    {
        unsigned int MaxCount = 0;
        TestItemInfo item_info = mSeclectItemInfoList[i];
        TestItem* item = item_info.item;
        TestSite* site = mSelectedSiteList[item_info.site_index];

        QBarSet *set = new QBarSet(QString::fromLocal8Bit(site->get_name()));
        QPen pen;
        pen.setWidthF(2.0);
        int rand1 = i % 16 + 1;
        int rand2 = i % 4 + 1;
        int rand3 = i % 2 + 1;
        QColor color(15 * rand1 , 63 * rand2 , 127 * rand3);
        pen.setColor(color);
        QColor new_color = color;
        new_color.setAlpha(127);
        set->setPen(pen);
        set->setBrush(new_color);

        if(i == 0)
        {
            XTestLable = QString::fromLocal8Bit(item->get_label());
            axisY->setTitleText(XTestLable);
        }

        for(unsigned int m = 0; m < group_count; m++)
        {
            double low_threshold = min_value + (m - 0.5) * step_value;
            double high_threshold = min_value + (m + 0.5) * step_value;
            unsigned int count = MyHistogram::GetIntervalCount(item, low_threshold, high_threshold);

            if(count > MaxCount) MaxCount = count;
            set->append(count);
        }

        bars->append(set);

        double avearge_value = item->get_average();
        double mean_x = avearge_value;
        int line_width = 3;

        QPen pen_mean;
        pen_mean.setStyle(Qt::DotLine);
        pen_mean.setWidth(line_width);
        pen_mean.setColor(color.darker());

        QLineSeries *series_mean=new QLineSeries;
        //series_mean->setName("Avg");
        series_mean->setPen(pen_mean);
        series_mean->append(mean_x, 0);
        series_mean->append(mean_x, MaxCount*1.05);
        mChart->addSeries(series_mean);
        mChart->setAxisX(axisX, series_mean);
        mChart->setAxisY(axisY, series_mean);

        QLineSeries *series_stdev=new QLineSeries;
        //series_stdev->setName("Std");
        double stdev_value = item->get_stdev();
        if(stdev_value != 0 )
        {
            double Xsigma[13] = {-3.0,-2.5,-2.0,-1.5,-1.0,-0.5,0.0,0.5,1.0,1.5,2.0,2.5,3.0};
            double Ysigma[13] = {0.00240274,0.00924471,0.02783468,0.06559062,0.12097758,0.17466632,
                 0.19741265,0.17466632,0.12097758,0.06559062,0.02783468,0.00924471,0.00240274}; // +/-3Sigma Percent.
            double Peak = 0.19741265;
            for(int s = 0; s < 13; s++)
            {
                double x_sigma = mean_x + Xsigma[s] * stdev_value;
                double y_max_count = MaxCount * Ysigma[s] / Peak;
                series_stdev->append(x_sigma, y_max_count);
                if(y_max_count > max_count_total) max_count_total = y_max_count;
                if(max_count_total < MaxCount) max_count_total = MaxCount;
            }
            QPen pen_stdev;
            pen_stdev.setStyle(Qt::DashLine);
            pen_stdev.setWidth(line_width);
            pen_stdev.setColor(color.dark());

            series_stdev->setPen(pen_stdev);
            mChart->addSeries(series_stdev);
            mChart->setAxisX(axisX, series_stdev);
            mChart->setAxisY(axisY, series_stdev);
        }
    }

    mChart->setAxisX(axisX, bars);
    mChart->setAxisY(axisY, bars);
    mChart->addSeries(bars);
    mChart->axisY()->setMax(max_count_total*1.05);
    mChart->legend()->show();
}

void MainWindow::PlotSPCChart()
{
    unsigned int max_device_count = PlotLineChart();
    if(max_device_count == 0) return;

    QVector<double> low_limit_list, high_limit_list, index_list;
    double low_limit, high_limit;
    TestItemInfo item_info = mSeclectItemInfoList[0];
    TestItem* item = item_info.item;

    if(item->highlimit_valid()) high_limit = item->get_highlimit();
    else high_limit = mDefaultHighLimit;

    if(item->lowlimit_valid()) low_limit = item->get_lowlimit();
    else low_limit = mDefaultLowLimit;

    low_limit_list.resize(max_device_count);
    high_limit_list.resize(max_device_count);
    index_list.resize(max_device_count);

    QLineSeries *series_low_limit=new QLineSeries;
    QLineSeries *series_high_limit=new QLineSeries;

    for(unsigned int i = 0; i < max_device_count; i++)
    {
        series_low_limit->append(i+1, low_limit);
        series_high_limit->append(i+1, high_limit);
    }

    QPen pen_lsl;
    pen_lsl.setWidth(3);
    pen_lsl.setColor(Qt::red);
    pen_lsl.setStyle(Qt::DashLine);

    QPen pen_usl;
    pen_usl.setWidth(3);
    pen_usl.setColor(Qt::red);
    pen_usl.setStyle(Qt::DotLine);

    series_low_limit->setName("Lower Limit");
    series_low_limit->setPen(pen_lsl);

    series_high_limit->setName("Upper Limit");
    series_high_limit->setPen(pen_usl);

    double space_vale = fabs(high_limit - low_limit) * 0.1;
    mChart->axisY()->setRange(low_limit - space_vale, high_limit + space_vale);
    mChart->addSeries(series_low_limit);
    mChart->addSeries(series_high_limit);
    auto axisX = mChart->axisX();
    auto axisY = mChart->axisY();
    mChart->setAxisX(axisX, series_low_limit);
    mChart->setAxisY(axisY, series_low_limit);

    mChart->setAxisX(axisX, series_high_limit);
    mChart->setAxisY(axisY, series_high_limit);
}

static void get_statistocal_box_values(const TestItem* item, double& minimum, double& lower_quart, double& median, double& upper_quart, double& maximum )
{
    unsigned int result_count = item->get_result_count();
    if(result_count < 4 ) return;
    std::vector<double> result(result_count, 0.0);
    for(unsigned int i = 0; i < result_count; i++)
        result[i]=item->get_result(i).get_value();
    std::sort(std::begin(result), std::end(result));
    minimum = result[0];
    lower_quart = result[result_count/4];
    median = result[result_count/2];
    upper_quart = result[result_count*3/4];
    maximum = result[result_count-1];
}

void MainWindow::PlotStatisticalBoxChart()
{
   ClearChartWidget();
   unsigned int site_count = mSelectedSiteList.size();
   if(site_count == 0) return ;
   unsigned int item_count = mSeclectItemInfoList.size();
   if(item_count == 0) return ;

   int plot_box_count = 0;
   QBoxPlotSeries *box_series = new QBoxPlotSeries();
   double ymin = 0.0, ymax = 0.0;
   QString item_lable;
    for(unsigned int i = 0; i < item_count; i++)
    {
        TestItemInfo item_info = mSeclectItemInfoList[i];
        TestItem* item = item_info.item;
        TestSite* site = mSelectedSiteList[item_info.site_index];
        int device_count = item->get_result_count();
        if(device_count < 4) continue;

        double minimum=0.0, lower_quart=0.0, median=0.0, upper_quart=0.0, maximum=0.0;
        get_statistocal_box_values(item, minimum, lower_quart, median, upper_quart, maximum);
        if(plot_box_count == 0)
        {
            ymin = minimum;
            ymax = maximum;
            item_lable = QString::fromLocal8Bit(item->get_label());
            const char* unit = item->get_unit();
            if(unit) item_lable += QString(tr("(Unit: %1)").arg(unit));
        }
        else
        {
            if(ymin >  minimum) ymin =  minimum;
            if(ymax < maximum) ymax = maximum;
        }

        QString box_name = QString::fromLocal8Bit(site->get_name());
        QBoxSet *box = new QBoxSet(box_name);
        box->setValue(QBoxSet::LowerExtreme, minimum);
        box->setValue(QBoxSet::UpperExtreme, maximum);
        box->setValue(QBoxSet::Median, median);
        box->setValue(QBoxSet::LowerQuartile, lower_quart);
        box->setValue(QBoxSet::UpperQuartile, upper_quart);

        box_series->append(box);
        plot_box_count++;

    }
    if(plot_box_count == 0 )
    {
        delete box_series;
        return;
    }

    mChart->setTitle(item_lable);
    mChart->addSeries(box_series);
    mChart->createDefaultAxes();
    mChart->axisY()->setMin(ymin - 0.01*(ymax-ymin));
    mChart->axisY()->setMax(ymax + 0.01*(ymax-ymin));
    mChart->legend()->setVisible(false);
    mChart->axisX()->setLabelsAngle(90);
}

void MainWindow::ShowDataLogErrorMessage(DataLogError err)
{
    switch(err)
    {
    case DATALOG_OPERATE_OK:
        QMessageBox::information(this,tr("Read DataLog Success"), tr("Read Test Data From DataLog File Success."),QMessageBox::Ok);
        break;
    case READ_FILE_ERROR:
        QMessageBox::critical(this, tr("Read File Error"), tr("Read Test Data From DataLog File Failure."),QMessageBox::Ok);
        break;
    case WRITE_FILE_ERROR:
        QMessageBox::critical(this, tr("Write File Error"), tr("Write Test Data To File Failure."),QMessageBox::Ok);
        break;
    case FILE_FORMATE_ERROR:
        QMessageBox::critical(this, tr("Error Message"), tr("File Format not Support or File is broken."),QMessageBox::Ok);
        break;
    case DATALOG_STDF_VERSION_NOT_SUPPORT:
        QMessageBox::critical(this, tr("Error Message"), tr("This Program Only Support STDF V4."),QMessageBox::Ok);
        break;
    case DATDALOG_STDF_CPU_TYPE_NOT_SUPPORT:
        QMessageBox::critical(this, tr("Error Message"), tr("This Program Only Support STDF CPU Type = 2."),QMessageBox::Ok);
        break;
    case NO_DATA_TO_WRIET:
        QMessageBox::critical(this, tr("Error Message"), tr("No Data in Site Need to Write.."),QMessageBox::Ok);
        break;
    case NO_DATA_TO_READ:
        QMessageBox::critical(this, tr("Error Message"), tr("No Data in File Need to Read. May File Format is Wrong."),QMessageBox::Ok);
        break;
    default:
        QMessageBox::critical(this, tr("Unknown Error"), tr("An Unknown Error Occurs."),QMessageBox::Ok);
        break;
    }
}


void MainWindow::ShowItemStatisticToTable()
{
	ui->DataTableWidget->clear();
	unsigned int site_count = mSelectedSiteList.size();
    if(site_count == 0) return;
    unsigned int item_count = mSeclectItemInfoList.size();
    if(item_count == 0) return;

	QStringList column_labels_list;
	column_labels_list.push_back(tr("Test Label"));
	column_labels_list.push_back(tr("Test Unit"));
	column_labels_list.push_back(tr("Low Limit"));
	column_labels_list.push_back(tr("High Limit"));
	column_labels_list.push_back(tr("Average"));
	column_labels_list.push_back(tr("Device Count"));
	column_labels_list.push_back(tr("Min"));
	column_labels_list.push_back(tr("Max"));
	column_labels_list.push_back(tr("Range"));
	column_labels_list.push_back(tr("Stdev"));
	column_labels_list.push_back(tr("Cp"));
	column_labels_list.push_back(tr("Cpk"));
	
	ui->DataTableWidget->setColumnCount(12);
	ui->DataTableWidget->setRowCount(site_count);
	ui->DataTableWidget->setHorizontalHeaderLabels(column_labels_list);
	
	QStringList row_labels_list;
    for(unsigned int n = 0; n < item_count; n++)
    {
        TestItemInfo item_info = mSeclectItemInfoList[n];
        TestItem* item = item_info.item;
        TestSite* site = mSelectedSiteList[item_info.site_index];
		
		QString site_name = QString::fromLocal8Bit(site->get_name());
		row_labels_list.push_back(site_name);

        QString test_label = QString::fromLocal8Bit(item->get_label());
        QString test_unit = QString::fromLocal8Bit(item->get_unit());
		
        QString str_low_limit, str_high_limit;
        if(item->highlimit_valid())
            str_high_limit.sprintf("%f", item->get_highlimit());
        else str_high_limit.sprintf("N/A");

        if(item->lowlimit_valid())
            str_low_limit.sprintf("%f", item->get_lowlimit());
        else str_low_limit.sprintf("N/A");


        QString str_cp, str_cpk;
        MySPC spc(item);
        spc.SetDefaltSpec(mDefaultLowLimit, mDefaultHighLimit);
        double cp_value = spc.GetCp();
        if(cp_value < 0) str_cp.sprintf("N/A");
        else str_cp.sprintf("%.3f", cp_value);

        double cpk_value = spc.GetCpk();
        if(cpk_value < 0) str_cpk.sprintf("N/A");
        else str_cpk.sprintf("%.3f", cpk_value);

		int colume_number = 0;
        ui->DataTableWidget->setItem(n, colume_number++, new QTableWidgetItem(test_label));
        ui->DataTableWidget->setItem(n, colume_number++, new QTableWidgetItem(test_unit));
        ui->DataTableWidget->setItem(n, colume_number++, new QTableWidgetItem(str_low_limit));
        ui->DataTableWidget->setItem(n, colume_number++, new QTableWidgetItem(str_high_limit));
        ui->DataTableWidget->setItem(n, colume_number++, new QTableWidgetItem(QString("%1").arg(item->get_average())));
        ui->DataTableWidget->setItem(n, colume_number++, new QTableWidgetItem(QString("%1").arg(item->get_result_count())));
        ui->DataTableWidget->setItem(n, colume_number++, new QTableWidgetItem(QString("%1").arg(item->get_min())));
        ui->DataTableWidget->setItem(n, colume_number++, new QTableWidgetItem(QString("%1").arg(item->get_max())));
        ui->DataTableWidget->setItem(n, colume_number++, new QTableWidgetItem(QString("%1").arg(item->get_range())));
        ui->DataTableWidget->setItem(n, colume_number++, new QTableWidgetItem(QString("%1").arg(item->get_stdev())));
        ui->DataTableWidget->setItem(n, colume_number++, new QTableWidgetItem(str_cp));

        QTableWidgetItem *cpk_item = new QTableWidgetItem(str_cpk);
        if(cpk_value >= 1.67) cpk_item->setBackgroundColor(Qt::green);
        else if(cpk_value < 1.33) cpk_item->setBackgroundColor(Qt::red);
        else cpk_item->setBackgroundColor(Qt::yellow);

        ui->DataTableWidget->setItem(n, colume_number, cpk_item );
	}
	ui->DataTableWidget->setVerticalHeaderLabels(row_labels_list);
	
}


QMessageBox::StandardButton MainWindow::ShowGRRErrorMessage(GRR_CALCULATE_ERROR err)
{
    QMessageBox::StandardButton ret;
    switch(err)
    {
    case GRR_RESULT_IS_OK:
        ret = QMessageBox::information(this,tr("GRR Calculate Success"), tr("Calculate GRR Success."),QMessageBox::Ok);
        break;
    case GRR_LABEL_NOT_SAME:
        ret = QMessageBox::question(this, tr("Test Label Not Same"), tr("Some TestItems TestLabel are Not Same. Continue?"));
        break;
    case GRR_NUMBER_NOT_SAME:
        ret = QMessageBox::question(this, tr("Test Number Not Same"), tr("Some TestItems TestNumber are Not Same. Continue?"));
        break;
    case GRR_LABEL_AND_NUMBER_NOT_SAME:
        ret = QMessageBox::question(this, tr("TestNumber and TestLabel Not Same"), tr("Some TestItems TestNumber and TestLabel are Not Same. Continue?"));
        break;
    case GRR_DATACOUNT_NOT_SAME:
        ret = QMessageBox::critical(this, tr("Test Data Count Not Same"), tr("Some TestItems Test Times or Devices Count Not Same."),QMessageBox::Ok);
        break;
    case GRR_ITEM_NOT_EXIST:
        ret = QMessageBox::critical(this, tr("TestItem Not Exist"), tr("TestItem not in Some TestSites."),QMessageBox::Ok);
        break;
    case GRR_DATACOUNT_TOO_FEW:
        ret = QMessageBox::critical(this, tr("Test Data Count Too Few"), tr("TestItem Test Times Need More Than Two."),QMessageBox::Ok);
        break;
    case GRR_DATA_NOT_ALL_PASS:
        ret = QMessageBox::critical(this, tr("Test Data Not All Pass"), tr("Some TestItems not All Pass."),QMessageBox::Ok);
        break;
    case GRR_SITE_NOT_EXIST:
        ret = QMessageBox::critical(this, tr("TestSite Has No Data"), tr("Some TestSite Has No Data."),QMessageBox::Ok);
        break;
    case GRR_ITEMCOUNT_NOT_SAME:
        ret = QMessageBox::critical(this, tr("TestItem Count Not Same"), tr("Some TestSite TestItem Count Not Same."),QMessageBox::Ok);
        break;
    case GRR_SITECOUNT_TOO_FEW:
        ret = QMessageBox::critical(this, tr("TestSite Count Too Few"), tr("GRR Calculate Need More Than Two."),QMessageBox::Ok);
        break;
    case GRR_NOT_ALL_ITEM_TESTED:
        ret = QMessageBox::critical(this, tr("TestItem Not All Tested"), tr("TestItem Exist, But Not Test."),QMessageBox::Ok);
        break;
    default:
        ret = QMessageBox::critical(this, tr("Unknown Error"), tr("An Unkonwn Error Occured."),QMessageBox::Ok);
        break;
    }
    return ret;
}

bool MainWindow::CalculateGRR()
{
    unsigned int site_count = mSelectedSiteList.size();
    if(site_count < 2)
    {
        QMessageBox::warning(this,tr("Test Site too Less"),tr("GRR calculating need more than two sites."),QMessageBox::Ok);
        return false;
    }

    mGrr->clear();
    mGrr->set_default_limit(mDefaultLowLimit, mDefaultHighLimit);
    mGrr->set_calculate_mode(mGRR_Mode);
    bool continue_flag = false;
    for(unsigned int i = 0; i < site_count; i++)
    {
        GRR_CALCULATE_ERROR err = mGrr->add_site(mSelectedSiteList[i]);
        if(err < 0)
        {
            ShowGRRErrorMessage(err);
            return false;
        }
        if(err > 0 && !continue_flag)
        {
            if(ShowGRRErrorMessage(err) == QMessageBox::Yes)
                continue_flag = true;
            else
                return false;
        }
    }

    GRR_CALCULATE_ERROR err = mGrr->calculate();
    if(err != 0)
    {
        ShowGRRErrorMessage(err);
        return false;
    }

    return true;
}

void MainWindow::SetGridCellValue(QTableWidget *table, int row, int column, const QString& value)
{
    QTableWidgetItem* item = new QTableWidgetItem(value);
    table->setItem(row, column, item);
}

void MainWindow::SetGridCellColor(QTableWidget *table,int row, int column, const QColor& color)
{
    QTableWidgetItem* item = table->item(row, column);
    if(!item)
    {
        item = new QTableWidgetItem();
        table->setItem(row, column, item);
    }
    item->setBackgroundColor(color);
}


bool MainWindow::ShowGRRDataToTable(QTableWidget* table, TestSite_GRR* grr)
{
    if(!table) return false;

    unsigned int site_count = grr->get_site_count();
    unsigned int item_count = grr->get_item_count();

    int row_count = 16 + site_count * 6;
    int col_count = item_count;

    table->clear();
    table->setRowCount(row_count);
    table->setColumnCount(col_count);

    QStringList row_labels, col_labels;
    unsigned int index = 1;
    int row = 0, column = 0;

    row_labels.push_back(QString("Test Number"));
    row_labels.push_back(QString("Unit"));
    row_labels.push_back(QString("High Limit"));
    row_labels.push_back(QString("Low Limit"));
    row_labels.push_back(QString("Max Average"));
    row_labels.push_back(QString("Min Average"));
    row_labels.push_back(QString("RxDiff"));
    row_labels.push_back(QString("Pooled Stdev"));
    row_labels.push_back(QString("EV(Repeatability)"));
    row_labels.push_back(QString("AV(Reproducibility)"));
    row_labels.push_back(QString("R&R"));
    row_labels.push_back(QString("GR&R"));
    row_labels.push_back(QString("----"));
    row_labels.push_back(QString("TCS-Error"));
    row_labels.push_back(QString("MGB"));
    row_labels.push_back(QString("TCS"));

    mItemLabelList.clear();
    for(unsigned int i = 0; i < item_count; i++)
    {
        TestItem_GRR item_grr = grr->get_grr(i);
        QString test_label = QString::fromLocal8Bit(item_grr.get_label());
        col_labels.push_back(test_label);
        mItemLabelList.push_back(test_label);

        QString test_unit = QString::fromLocal8Bit(item_grr.get_unit());
        QString str_grr;
        double grr_value = 100.0 * item_grr.get_gr_r();
        if(grr_value > 100.0) grr_value = 100.0;
        str_grr.sprintf("%.4f%%",grr_value);


        row = 0;
        SetGridCellValue(table, row++, column, QString("%1").arg(item_grr.get_number()));
        SetGridCellValue(table, row++, column, test_unit);
        SetGridCellValue(table, row++, column, QString("%1").arg(item_grr.get_highspec()));
        SetGridCellValue(table, row++, column, QString("%1").arg(item_grr.get_lowspec()));
        SetGridCellValue(table, row++, column, QString("%1").arg(item_grr.get_max_average()));
        SetGridCellValue(table, row++, column, QString("%1").arg(item_grr.get_min_average()));
        SetGridCellValue(table, row++, column, QString("%1").arg(item_grr.get_average_diff()));
        SetGridCellValue(table, row++, column, QString("%1").arg(item_grr.get_stdev_bar()));
        SetGridCellValue(table, row++, column, QString("%1").arg(item_grr.get_repeatability()));
        SetGridCellValue(table, row++, column, QString("%1").arg(item_grr.get_reproducibility()));
        SetGridCellValue(table, row++, column, QString("%1").arg(item_grr.get_r_r()));
        SetGridCellValue(table, row, column, str_grr);

        if(grr_value <= 10) SetGridCellColor(table, row, column, Qt::green);
        else if(grr_value > 30) SetGridCellColor(table,row, column, Qt::red);
        else  SetGridCellColor(table,row, column, Qt::yellow);
        row++;
        row++;

        SetGridCellValue(table, row++, column, QString("%1").arg(item_grr.get_tcs_error()));
        SetGridCellValue(table, row++, column, QString("%1").arg(item_grr.get_measure_guard_band()));

        double tcs_value = 100.0 * item_grr.get_tcs();
        QString str_tcs;
        str_tcs.sprintf("%.4f%%", tcs_value);
        SetGridCellValue(table, row, column, str_tcs);

        if(tcs_value <= 10) SetGridCellColor(table, row, column, Qt::green);
        else if(tcs_value > 30) SetGridCellColor(table,row, column, Qt::red);
        else  SetGridCellColor(table,row, column, Qt::yellow);

        row++;
        index++;
        column++;
    }

    for(unsigned int i = 0; i < site_count; i++)
    {
        const TestSite *site = grr->get_site(i);
        QString site_name = QString::fromLocal8Bit(site->get_name());

        row_labels.push_back(site_name);
        row_labels.push_back(QString("Max"));
        row_labels.push_back(QString("Min"));
        row_labels.push_back(QString("Range"));
        row_labels.push_back(QString("Average"));
        row_labels.push_back(QString("Stdev"));

        column = 0;

        QTableWidgetItem *site_name_item = new QTableWidgetItem();
        site_name_item->setToolTip(site_name);
        table->setVerticalHeaderItem(row, site_name_item);

        int start_row = row + 1;

        for(unsigned int n = 0; n < item_count; n++)
        {
            TestItem* item = site->get_item(n);
            row = start_row;
            SetGridCellValue(table, row, column, QString("%1").arg(item->get_max()));
            SetGridCellColor(table, row++, column, QColor(147, 197, 251));

            SetGridCellValue(table, row, column, QString("%1").arg(item->get_min()));
            SetGridCellColor(table, row++, column, QColor(218, 254, 251));

            SetGridCellValue(table, row, column, QString("%1").arg(item->get_range()));
            SetGridCellColor(table, row++, column, QColor(249, 187, 248));

            SetGridCellValue(table, row, column, QString("%1").arg(item->get_average()));
            SetGridCellColor(table, row++, column, QColor(249, 249, 183));

            SetGridCellValue(table, row, column, QString("%1").arg(item->get_stdev()));
            SetGridCellColor(table, row++, column, QColor(203, 207, 214));

            column++;
        }
    }

    table->setHorizontalHeaderLabels(col_labels);
    table->setVerticalHeaderLabels(row_labels);

    QHeaderView *row_header = table->verticalHeader();
    if(row_header)
    {
        row_header->setFixedWidth(120);
    }
    return true;
}

bool MainWindow::ShowSiteStatisticToTable(QTableWidget *table, TestSite* site, int& start_row)
{
    if(start_row == 0)
    {
        QStringList ColumnLabels;
        ColumnLabels<<tr("Low Spec");
        ColumnLabels<<tr("High Spec");
        ColumnLabels<<tr("Unit");
        ColumnLabels<<tr("Min.");
        ColumnLabels<<tr("Max.");
        ColumnLabels<<tr("Average");
        ColumnLabels<<tr("Range");
        ColumnLabels<<tr("Stedev");
        ColumnLabels<<tr("Cp");
        ColumnLabels<<tr("Cpk");
        table->setHorizontalHeaderLabels(ColumnLabels);
    }

    if(!site) return false;
    unsigned int item_count = site->get_item_count();
    if(item_count == 0) return false;

    table->insertRow(start_row);
    QString site_name = QString::fromLocal8Bit(site->get_name());
    table->setVerticalHeaderItem(start_row, new QTableWidgetItem("Site Name"));
    SetGridCellValue(table, start_row++, 0, site_name);
    table->insertRow(start_row);

    for(unsigned int i = 0; i < item_count; i++)
    {
        int column  = 0;
        TestItem* item = site->get_item(i);
        MySPC spc(item);
        spc.SetDefaltSpec(mDefaultLowLimit, mDefaultHighLimit);

        QString test_lable  = QString::fromLocal8Bit(item->get_label());
        table->setVerticalHeaderItem(start_row, new QTableWidgetItem(test_lable));

        if(item->lowlimit_valid())
            SetGridCellValue(table, start_row, column++, QString("%1").arg(item->get_lowlimit()));
        else
            SetGridCellValue(table, start_row, column++, QString("N/A"));

        if(item->highlimit_valid())
            SetGridCellValue(table, start_row, column++, QString("%1").arg(item->get_highlimit()));
        else
            SetGridCellValue(table, start_row, column++, QString("N/A"));

        QString test_unit = QString::fromLocal8Bit(item->get_unit());
        SetGridCellValue(table, start_row, column++, test_unit);

        SetGridCellValue(table, start_row, column++, QString("%1").arg(item->get_min()));
        SetGridCellValue(table, start_row, column++, QString("%1").arg(item->get_max()));
        SetGridCellValue(table, start_row, column++, QString("%1").arg(item->get_average()));
        SetGridCellValue(table, start_row, column++, QString("%1").arg(item->get_range()));
        SetGridCellValue(table, start_row, column++, QString("%1").arg(item->get_stdev()));

        QString str_cp, str_cpk;
        double cp_value = spc.GetCp();
        double cpk_value =spc.GetCpk();
        str_cpk.sprintf("%.4f", cpk_value);
        str_cp.sprintf("%.4f", cp_value);

        QTableWidgetItem *cp_item = new QTableWidgetItem(str_cp);
        if(cp_value > 1.67) cp_item->setBackgroundColor(Qt::green);
        else if(cp_value < 1.33) cp_item->setBackgroundColor(Qt::red);
        else cp_item->setBackgroundColor(Qt::yellow);
        table->setItem(start_row , column++, cp_item );

        QTableWidgetItem *cpk_item = new QTableWidgetItem(str_cpk);
        if(cpk_value > 1.67) cpk_item->setBackgroundColor(Qt::green);
        else if(cpk_value < 1.33) cpk_item->setBackgroundColor(Qt::red);
        else cpk_item->setBackgroundColor(Qt::yellow);
        table->setItem(start_row , column++, cpk_item );

        start_row++;
        table->insertRow(start_row);
    }
    QTableWidgetItem *row_lable_item = new QTableWidgetItem("======");
    row_lable_item->setBackgroundColor(Qt::red);
    table->setVerticalHeaderItem(start_row, row_lable_item);
    start_row++;
    return true;
}

int MainWindow::GetSelectedCheckedIndex()
{
    int index = -1;
    unsigned int list_item_count = ui->TestSiteListWidget->count();
    for(unsigned int n = 0; n < list_item_count; n++)
    {
          QListWidgetItem* list_item = ui->TestSiteListWidget->item(n);
          if(list_item->checkState() == Qt::Checked)
          {
              index++;
              if(list_item->isSelected()) break;
          }
    }
    return index;
}


bool MainWindow::CalculateCorrelation()
{
    TestSite* reference_site = nullptr;
    if(mSiteSelectedIndex == -1)
    {
        QMessageBox::warning(this,tr("No Reference Site Select"), tr("Must Select A Reference Site First."), QMessageBox::Ok);
        return false;
    }

    QListWidgetItem *list_item = ui->TestSiteListWidget->item(mSiteSelectedIndex);
    if(list_item->checkState() == Qt::Checked)
    {
        reference_site = mTestSiteList[mSiteSelectedIndex];
    }
    else
    {
        QMessageBox::warning(this,tr("No Reference Site Select"), tr("Must Select A Reference Site First. Check it."), QMessageBox::Ok);
        return false;
    }

    unsigned int site_count = mSelectedSiteList.size();
    if(site_count < 2)
    {
        QMessageBox::warning(this,tr("Test Site too Less"),tr("Correlation calculating need more than two sites."), QMessageBox::Ok);
        return false;
    }

    mCorr->clear();
    mCorr->set_ref_site(reference_site, mDefaultLowLimit, mDefaultHighLimit);
    mCorr->set_corr_spec(mCorrelationSpec);
    mCorr->set_calcualte_mode(mCorr_Mode);

    bool continue_flag = false;
    unsigned int checked_index = (unsigned int)(GetSelectedCheckedIndex());
    for(unsigned int i = 0; i < site_count; i++)
    {
        if(i == checked_index) continue;

        CORR_CALCULATE_ERROR err = mCorr->add_site(mSelectedSiteList[i]);
        if(err < 0)
        {
            ShowCorrErrorMessage(err);
            return false;
        }
        if(err > 0 && !continue_flag)
        {
            if(ShowCorrErrorMessage(err) == QMessageBox::Yes)
                continue_flag = true;
            else
                return false;
        }
    }

    CORR_CALCULATE_ERROR err = mCorr->calculate();
    if(err != 0)
    {
        ShowCorrErrorMessage(err);
        return false;
    }

    return true;
}

QMessageBox::StandardButton MainWindow::ShowCorrErrorMessage(CORR_CALCULATE_ERROR err)
{
    QMessageBox::StandardButton ret;
    switch(err)
    {
    case CORR_RESULT_IS_OK:
        ret = QMessageBox::information(this,tr("Correlation Calculate Success"), tr("Calculate Correlation Success."),QMessageBox::Ok);
        break;
    case CORR_LABEL_NOT_SAME:
        ret = QMessageBox::question(this, tr("Test Label Not Same"), tr("Some TestItems TestLabel are Not Same. Continue?"));
        break;
    case CORR_NUMBER_NOT_SAME:
        ret = QMessageBox::question(this, tr("Test Number Not Same"), tr("Some TestItems TestNumber are Not Same. Continue?"));
        break;
    case CORR_LABEL_AND_NUMBER_NOT_SAME:
        ret = QMessageBox::question(this, tr("Test Number And Label Not Same"), tr("Some TestItems TestNumber & TestLabel are Not Same. Continue?"));
        break;
    case CORR_DATA_NOT_ALL_PASS:
        ret = QMessageBox::question(this, tr("Test Result Not All Pass"), tr("Some TestItems Results are Fail. Continue?"));
        break;
    case CORR_DATACOUNT_NOT_SAME:
        ret = QMessageBox::question(this, tr("Test Result Count Not Same"), tr("Some TestItems TestTimes or Device Count Not Same. Continue?"));
        break;
    case CORR_ITEM_NOT_EXIST:
        ret = QMessageBox::critical(this, tr("TestItem Not Exist"), tr("There is No TestItem in TestSite."),QMessageBox::Ok);
        break;
    case CORR_DATACOUNT_TOO_FEW:
        ret = QMessageBox::critical(this, tr("Device Count is Zero"), tr("Device Count or TestTimes Must More Than One."),QMessageBox::Ok);
        break;
    case CORR_REF_SITE_NOT_SET:
        ret = QMessageBox::critical(this, tr("No reference Site"), tr("Set Reference Site First."),QMessageBox::Ok);
        break;
    case CORR_SITE_NOT_EXIST:
        ret = QMessageBox::critical(this, tr("TestSite is NULL"), tr("TestSite is NULL. Need Check The Program."),QMessageBox::Ok);
        break;
    case CORR_ITEMCOUNT_NOT_SAME:
        ret = QMessageBox::critical(this, tr("TestItem Count Not Same"), tr("Some TestSite TestItem Count Not Same."),QMessageBox::Ok);
        break;
    case CORR_SITECOUNT_TOO_FEW:
        ret = QMessageBox::critical(this, tr("Sites Too Few"), tr("Calculate Correlation Need More Than Two Sites."),QMessageBox::Ok);
        break;
    case CORR_NOT_ALL_ITEM_TESTED:
        ret = QMessageBox::critical(this, tr("Not All TestItem Tested"), tr("Some TestItems are Not Tested."),QMessageBox::Ok);
        break;
    default:
        ret = QMessageBox::critical(this, tr("Unknown Error"), tr("An Unkonwn Error Occured."),QMessageBox::Ok);
        break;
    }
    return ret;
}

bool MainWindow::ShowCorrDataToTable(QTableWidget* table, TestSite_CORR* corr)
{
    if(!table) return false;
    unsigned int site_count = corr->get_site_count();
    unsigned int item_count = corr->get_item_count();
    int last_col_count = 6;

    table->clear();
    table->setRowCount(item_count);
    table->setColumnCount(last_col_count + 4 * site_count);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QStringList col_labels;

    col_labels.push_back(QString("Test Number"));
    col_labels.push_back(QString("High Limit"));
    col_labels.push_back(QString("Low Limit"));
    col_labels.push_back(QString("Tolerance"));
    col_labels.push_back(QString("Unit"));
    col_labels.push_back(QString("Ref Average"));
    col_labels.push_back(QString("||"));

    for(unsigned int i = 0; i < site_count; i++)
    {
        const TestSite* site = corr->get_site(i);
        QString site_name = QString::fromLocal8Bit(site->get_name());
        site_name += tr(" Average");
        col_labels.push_back(site_name);
        col_labels.push_back(tr("Delta Value"));
        col_labels.push_back(tr("Correlation Value"));
        col_labels.push_back("||");
    }

    int row = 0;
    int col = 0;
    for(unsigned int i = 0; i < item_count; i++)
    {
        col = 0;
        TestItem_CORR* item_corr = corr->get_corr(i);
        QString test_label = QString::fromLocal8Bit(item_corr->get_label());
        QString test_unit = QString::fromLocal8Bit(item_corr->get_unit());

        table->setVerticalHeaderItem(row, new QTableWidgetItem(test_label));
        SetGridCellValue(table, row, col++, QString("%1").arg(item_corr->get_number()));
        SetGridCellValue(table, row, col++, QString("%1").arg(item_corr->get_highspec()));
        SetGridCellValue(table, row, col++, QString("%1").arg(item_corr->get_lowspec()));
        SetGridCellValue(table, row, col++, QString("%1").arg(item_corr->get_tolerance()));
        SetGridCellValue(table, row, col++, test_unit);
        SetGridCellValue(table, row, col++, QString("%1").arg(item_corr->get_ref_average()));

        col++;
        for(unsigned int n = 0; n < site_count; n++)
        {
            QString str_corr;
            double corr_value = item_corr->get_corr_value(n);
            str_corr.sprintf("%.2f%%", corr_value * 100.0);
            SetGridCellValue(table, row, col++, QString("%1").arg(item_corr->get_corr_average(n)));
            SetGridCellValue(table, row, col++, QString("%1").arg(item_corr->get_delta_value(n)));
            SetGridCellValue(table, row, col, str_corr);
            if(fabs(corr_value) <= mCorrelationSpec)
                SetGridCellColor(table, row, col, Qt::green);
            else
                SetGridCellColor(table, row, col, Qt::red);
            col++;
            SetGridCellColor(table, row, col++, Qt::gray);
        }
        row++;
    }
    table->setHorizontalHeaderLabels(col_labels);

    QHeaderView *row_header = table->verticalHeader();
    if(row_header)
    {
        row_header->setFixedWidth(120);
    }
    return true;
}

void MainWindow::on_actionConvert_DataLog_to_Summary_triggered()
{
    ConvertFileDialog dialog(this);
    dialog.setWindowTitle(tr("Convert DataLog to Summary"));
    dialog.SetDataLogs(mDataLogFileNames);
    int ret = dialog.exec();
    if(ret == QDialog::Accepted)
    {
        int index = dialog.GetCurrentIndex();
        if(index >= 0)
        {
            DataLog* datalog = mDataLogList[index];
            QFileDialog file_dialog(this);
            file_dialog.setWindowTitle(tr("Save Summary CSV File"));
            file_dialog.setNameFilter(tr("CSV Files(*.csv)"));
            file_dialog.setDefaultSuffix("csv");
            file_dialog.setAcceptMode(QFileDialog::AcceptSave);

            if(file_dialog.exec() == QDialog::Accepted)
            {
                QString filename = file_dialog.selectedFiles()[0];
                DataLogError err = datalog->write_summary_to_csv(filename.toLocal8Bit().data());
                if(err == DATALOG_OPERATE_OK)
                    QMessageBox::information(this,tr("Save File Success"), tr("Save Summary to CSV File Success."),QMessageBox::Ok);
                else
                    ShowDataLogErrorMessage(err);
            }
        }
    }
}

void MainWindow::on_actionShow_WaferMap_triggered()
{
    ConvertFileDialog dialog(this);
    dialog.SetDataLogs(mDataLogFileNames);
    dialog.setWindowTitle(tr("Select the Datalog to Show WaferMap"));
    int ret = dialog.exec();
    if(ret == QDialog::Accepted)
    {
        int index = dialog.GetCurrentIndex();
        if(index >= 0)
        {
            DataLog* datalog = mDataLogList[index];
            WaferMapDialog wafermap;
            if(wafermap.SetDataLog(datalog))
                wafermap.exec();
        }
    }
}

void MainWindow::ClearAllSiteSelected()
{
    int item_count = ui->TestSiteListWidget->count();
    for(int i = 0; i < item_count; i++)
    {
        QListWidgetItem *item = ui->TestSiteListWidget->item(i);
        item->setCheckState(Qt::Unchecked);
    }
}

void MainWindow::on_UnSelectAllSiteButton_clicked()
{
    ClearAllSiteSelected();
}

void MainWindow::SetSiteNumbers()
{
    mSiteNumbers.clear();
    unsigned int site_count = mTestSiteList.size();
    for(unsigned int i = 0; i < site_count; i++)
    {
        const TestSite* site = mTestSiteList[i];
        unsigned int site_number = site->get_number();
        mSiteNumbers[site_number] = QString("site%1").arg(site_number);
    }
}

void MainWindow::on_SelectBySiteButton_clicked()
{
    SetSiteNumbers();
    int number_count = mSiteNumbers.size();
    if(number_count == 0) return;
    QStringList site_numbers;
    std::map<unsigned int, QString>::const_iterator it = mSiteNumbers.begin();
    while(it != mSiteNumbers.end())
    {
        site_numbers<<(it->second);
        ++it;
    }

    bool ok = false;
    QString str_number = QInputDialog::getItem(this, QObject::tr("Select Site Number"),QObject::tr("Site Numbers"), site_numbers, 0, false, &ok);
    if(ok && !str_number.isEmpty())
    {
        ClearAllSiteSelected();
        int count = ui->TestSiteListWidget->count();
        for(int i = 0; i < count; i++)
        {
            QListWidgetItem *item = ui->TestSiteListWidget->item(i);
            QString value = item->text();
            if(value.contains(str_number)) item->setCheckState(Qt::Checked);
        }
    }
}

void MainWindow::on_ScatterPlotButton_clicked()
{
    ScatterPlotDialog dialog(this);
    dialog.AddTestSite(mTestSiteList);
    dialog.exec();
}



void MainWindow::on_AboutAction_triggered()
{
    QString title = QObject::tr("About");
    QString message = QObject::tr("------ Version 3.0.0.2 -------\n");
    message += QObject::tr("-- This Software is Free of Charge, \n");
    message += QObject::tr("   Unlicensed and comes with No Warranty.");
    QMessageBox msgDlg(QMessageBox::Information, title, message, QMessageBox::Ok,NULL);
    msgDlg.exec();
}

void MainWindow::on_UpdateInfoAction_triggered()
{
    QString title = QObject::tr("Update Information");
    QString message = QObject::tr("- 2016.04.11 Create New DataLog Analysis Program\n");
    message  += QObject::tr("- 2016.04.12 Refactoring Codes of STDF Part\n");
    message  += QObject::tr("- 2016.04.13 Combine with STDF V4 Reader\n");
    message  += QObject::tr("- 2016.05.09 Bug Fixed By Init Zero Record Array\n");
    message  += QObject::tr("- 2016.06.07 Fixed STDF Record Write to File Error\n");
    message  += QObject::tr("- 2016.06.12 Add Translation of Simpled Chinese\n");
    message  += QObject::tr("- 2016.07.12 Adapt to PIR-PTR-PRR and PIR-PTRs-PRR\n");
    message  += QObject::tr("- 2016.08.03 Fiexd Correlation Calculation Sites Selected Error\n");
    message  += QObject::tr("- 2016.08.08 Ignore the All Commas Line in CSV\n");
    message  += QObject::tr("- 2016.08.29 Add Correlation Spec Setting\n");
    message  += QObject::tr("- 2016.09.02 Add X/Y Coordinate When Read CSV\n");
    message  += QObject::tr("- 2016.11.06 Add Simplied WaferMap Function\n");
    message  += QObject::tr("- 2017.01.12 Fixed STDF Reader MIR miss PROC_ID\n");
    message  += QObject::tr("- 2017.04.05 Fixed Correlation On once Test\n");
    message  += QObject::tr("- 2017.04.29 Add XLS Format for Data Saving\n");
    message  += QObject::tr("- 2017.05.02 Change Sites Selected Method\n");
    message  += QObject::tr("- 2017.05.04 Add Scatter Plot for TestItems in Same Site\n");
    message  += QObject::tr("- 2017.05.05 STDF Check can Save STDF to XLS\n");
    message  += QObject::tr("- 2017.05.12 Scatter Plot Add Correlation Coefficient\n");
    message  += QObject::tr("- 2017.06.01 Replace the QCustomPlot with QtCharts\n");
    message  += QObject::tr("- 2017.06.21 Stdf Reader Add Save Table to CSV\n");
    message  += QObject::tr("- 2017.07.05 Update CSV Interface for DataLog Read\n");
    message  += QObject::tr("- 2017.07.19 Add Correlation Calculate Mode\n");
    message  += QObject::tr("- 2017.07.26 Refactoring the STDF Reader Codes\n");
    message  += QObject::tr("- 2017.07.27 Fixed STDF No SDR Site Name is Empty Error\n");
    message  += QObject::tr("- 2018.02.25 Add Statistical Box Plot\n");
    message  += QObject::tr("- 2018.03.01 Stdf Check Add Part ID for PTR Records\n");
    message  += QObject::tr("- 2018.06.29 Update the Bin Description for Wafer Map\n");
    message  += QObject::tr("- 2018.10.24 Add All Sites Summary for Log File\n");
    message  += QObject::tr("- 2018.10.30 Support the Powertech TXT file format\n");
    message  += QObject::tr("- 2018.11.27 Change XLS file to XLSX file format\n");
    message  += QObject::tr("- 2019.03.16 Show Data Value on the Line Chart\n");
	message  += QObject::tr("- 2019.05.20 When no Limits, Set Result Pass.\n");
    message  += QObject::tr("- 2019.05.25 Add the TCS(Test Capability Study). \n");
    QMessageBox msgDlg(QMessageBox::Information, title, message, QMessageBox::Ok,NULL);
    msgDlg.exec();
}


