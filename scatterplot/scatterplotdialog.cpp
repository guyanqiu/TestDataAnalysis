#include "scatterplotdialog.h"
#include "ui_scatterplotdialog.h"
#include <math.h>

#include <QtCharts>

QT_CHARTS_USE_NAMESPACE

static double correlation_coefficient(const QVector<double>&x, const QVector<double>& y)
{
    double sum_of_x = 0.0, sum_of_y = 0.0;
    double sum_of_xx = 0.0, sum_of_yy = 0.0, sum_of_xy = 0.0;
    int x_size = x.size();
    int y_size = y.size();
    if(x_size != y_size) return 0;
    if(x_size == 0) return 0;
    for(int i = 0; i < x_size; i++)
    {
        sum_of_x += x[i];
        sum_of_y += y[i];
        sum_of_xx += x[i]*x[i];
        sum_of_yy += y[i]*y[i];
        sum_of_xy += x[i]*y[i];
    }

    double Sxx = sum_of_xx - sum_of_x*sum_of_x/x_size;
    double Syy = sum_of_yy - sum_of_y*sum_of_y/x_size;
    double Sxy = sum_of_xy - sum_of_x*sum_of_y/x_size;

    if(Sxx == 0) return 0.0;
    if(Syy == 0) return 0.0;

    double r = Sxy / sqrt(Sxx*Syy);
    return r;
}

ScatterPlotDialog::ScatterPlotDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScatterPlotDialog)
{
    ui->setupUi(this);
    mSiteList.clear();
    chart_is_ok = false;
    site_is_exist = false;
    mCurrentSite = nullptr;
    mCurrentXItem = nullptr;
    mCurrentYItem = nullptr;

    mChart = new QChart(nullptr);
    ui->ChartPlotWidget->setChart(mChart);
    ui->ChartPlotWidget->setRenderHint(QPainter::Antialiasing);
}

void ScatterPlotDialog::ClearPlotData()
{
    QList<QAbstractAxis*> axisList = mChart->axes();
    foreach (QAbstractAxis *axis, axisList)
    {
          mChart->removeAxis(axis);
    }

    mChart->removeAllSeries();
    mChart->setTitle("");

    chart_is_ok = false;
}

ScatterPlotDialog::~ScatterPlotDialog()
{
    delete ui;
    delete mChart;
}

void ScatterPlotDialog::on_PlotButton_clicked()
{
    ClearPlotData();
    if(site_is_exist && mCurrentXItem && mCurrentYItem)
    {
        QVector<double> x, y;

        unsigned int x_count = mCurrentXItem->get_result_count();
        unsigned int y_count = mCurrentYItem->get_result_count();
        if(x_count != y_count)
        {
            QMessageBox::critical(this, tr("X and Y data count not Same"), tr("X and Y data count must Same."),QMessageBox::Ok);
            return;
        }

        QScatterSeries *series = new QScatterSeries();
        for(unsigned int i = 0; i < x_count; i++)
        {
            x.push_back(mCurrentXItem->get_result(i).get_value());
            y.push_back(mCurrentYItem->get_result(i).get_value());
            series->append(mCurrentXItem->get_result(i).get_value(), mCurrentYItem->get_result(i).get_value());
        }
        QString xlabel = QString::fromLocal8Bit(mCurrentXItem->get_label());
        if(mCurrentXItem->get_unit())
        {
            xlabel += tr("(");
            xlabel += QString::fromLocal8Bit(mCurrentXItem->get_unit());
            xlabel += tr(")");
        }
        QString ylabel = QString::fromLocal8Bit(mCurrentYItem->get_label());
        if(mCurrentYItem->get_unit())
        {
            ylabel += tr("(");
            ylabel += QString::fromLocal8Bit(mCurrentYItem->get_unit());
            ylabel += tr(")");
        }

        double minx = mCurrentXItem->get_min();
        double maxx = mCurrentXItem->get_max();
        double miny = mCurrentYItem->get_min();
        double maxy = mCurrentYItem->get_max();

        QValueAxis *axisX = new QValueAxis;
        axisX->setLabelFormat("%.4f");
        axisX->setTitleText(xlabel);
        axisX->setGridLineVisible(true);
        axisX->setRange(minx, maxx);

        QValueAxis *axisY = new QValueAxis;
        axisY->setTitleText(ylabel);
        axisY->setLabelFormat("%.4f");
        axisY->setGridLineVisible(true);
        axisY->setRange(miny, maxy);

        mChart->setAxisX(axisX, series);
        mChart->setAxisY(axisY, series);
        mChart->legend()->show();

        double r = correlation_coefficient(x, y);
        QString text = QString::asprintf("Correlation Coefficient: R = %.2f", r);
        mChart->setTitle(text);
        QPen pen;
        pen.setColor(Qt::blue);

        series->setName("X Y Scatter");
        series->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
        series->setMarkerSize(6.0);
        series->setPen(pen);
        mChart->addSeries(series);

        chart_is_ok = true;
    }
    else
    {
        chart_is_ok = false;
    }
}

void ScatterPlotDialog::SetItemLabels()
{
    if(mCurrentSite)
    {
        unsigned int item_count = mCurrentSite->get_item_count();
        if(item_count == 0) return;

        ui->comboBox_XItemLable->clear();
        ui->comboBox_YItemLabel->clear();
        for(unsigned int i = 0; i < item_count; i++)
        {
            TestItem* item = mCurrentSite->get_item(i);
            QString label = QString::fromLocal8Bit(item->get_label());
            ui->comboBox_XItemLable->addItem(label);
            ui->comboBox_YItemLabel->addItem(label);
        }
        site_is_exist = true;
        ui->comboBox_XItemLable->setCurrentIndex(0);
        ui->comboBox_YItemLabel->setCurrentIndex(0);

        mCurrentXItem = mCurrentSite->get_item(0);
        mCurrentYItem = mCurrentSite->get_item(0);
    }
    else
    {
        mCurrentXItem = nullptr;
        mCurrentYItem = nullptr;
    }
}

void ScatterPlotDialog::AddTestSite(const std::vector<TestSite*>& site_list)
{
    unsigned int site_count = site_list.size();
    if(site_count == 0) return;

    ui->comboBox_SiteName->clear();
    mSiteList.clear();
    for(unsigned int i = 0; i < site_count; i++)
    {
        TestSite* site = site_list[i];
        mSiteList.push_back(site);
        QString site_name = QString::fromLocal8Bit(site->get_name());
        ui->comboBox_SiteName->addItem(site_name);
    }
    ui->comboBox_SiteName->setCurrentIndex(0);
    mCurrentSite = mSiteList[0];
    SetItemLabels();
}

void ScatterPlotDialog::on_SaveChartButton_clicked()
{
    if(chart_is_ok)
    {
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
}

void ScatterPlotDialog::on_ExitButton_clicked()
{
    close();
}

void ScatterPlotDialog::on_comboBox_SiteName_currentIndexChanged(int index)
{
    if(index >= 0 && index < (int)mSiteList.size())
    {
        mCurrentSite = mSiteList[index];
    }
    else
    {
        mCurrentSite = nullptr;
    }
    SetItemLabels();
}


void ScatterPlotDialog::on_comboBox_XItemLable_currentIndexChanged(int index)
{
    if(!mCurrentSite) return;
    unsigned int count = mCurrentSite->get_item_count();
    if(index >= 0 && index < (int)count)
    {
        mCurrentXItem = mCurrentSite->get_item(index);
    }
    else
    {
         mCurrentXItem = nullptr;
    }
}

void ScatterPlotDialog::on_comboBox_YItemLabel_currentIndexChanged(int index)
{
    if(!mCurrentSite) return;
    unsigned int count = mCurrentSite->get_item_count();
    if(index >= 0 && index < (int)count)
    {
        mCurrentYItem = mCurrentSite->get_item(index);
    }
    else
    {
         mCurrentYItem = nullptr;
    }
}
