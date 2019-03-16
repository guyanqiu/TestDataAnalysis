#include "statisticdialog.h"
#include "ui_statisticdialog.h"
#include <QFileDialog>
#include <QMessageBox>

#include "xlsxdocument.h"
#include "xlsxdocument.h"
#include "xlsxabstractsheet.h"

//static std::wstring convert(const char* str)
//{
//    QString temp = QString::fromLocal8Bit(str);
//    return std::wstring(reinterpret_cast<const wchar_t *>(temp.utf16()));
//}

StatisticDialog::StatisticDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StatisticDialog)
{
    ui->setupUi(this);
    Qt::WindowFlags flags = this->windowFlags();
    flags  |= (Qt::WindowMaximizeButtonHint|Qt::WindowMinimizeButtonHint|Qt::WindowCloseButtonHint);
    this->setWindowFlags(flags);

    mDefaultLowLimit = -9999.9;
    mDefaultHighLimit = 9999.9;
}

StatisticDialog::~StatisticDialog()
{
    ui->tableWidget->clear();
    delete ui;
}

void StatisticDialog::SetDefaultLimit(double default_lowlimit, double default_highlimit)
{
    mDefaultLowLimit = default_lowlimit;
    mDefaultHighLimit = default_highlimit;
}

void StatisticDialog::on_pushButton_Ok_clicked()
{
    close();
}

QTableWidget *StatisticDialog::GetStatisticTable()
{
    return ui->tableWidget;
}

void StatisticDialog::SetSiteList(std::vector<TestSite*>& site_list)
{
    mSitesList = site_list;
}

void StatisticDialog::on_SaveButton_clicked()
{
    QFileDialog *dialog = new QFileDialog(this);
    dialog->setWindowTitle(tr("Save Statistic Result"));
    QStringList filters;
    QString csv_filter("CSV Files(*.csv)");
    QString xls_filter("XLSX Files(*.xlsx)");
    filters << csv_filter << xls_filter;
    dialog->setNameFilters(filters);
    dialog->setAcceptMode(QFileDialog::AcceptSave);

    if(dialog->exec() == QDialog::Accepted)
    {
        QString filename = dialog->selectedFiles()[0];
        QString file_suffux = QFileInfo(filename).suffix();
        QString current_filter = dialog->selectedNameFilter();

        if(current_filter == csv_filter)
        {
            if(file_suffux != "csv") filename += tr(".csv");
            if(SaveStatisticToCSVFile(filename.toLocal8Bit().data()))
                QMessageBox::information(this,tr("Save Result Success"),tr("Save Statistic Result to CSV File Success."),QMessageBox::Ok);
            else
                QMessageBox::critical(this,tr("Save Result Failure"),tr("Save Statistic Result to CSV File Falure."),QMessageBox::Ok);
        }
        else
        {
            if(file_suffux != "xlsx") filename += tr(".xlsx");
            if(SaveStatisticToXLSFile(filename.toLocal8Bit().data()))
                QMessageBox::information(this,tr("Save Result Success"),tr("Save Statistic Result to XLSX File Success."),QMessageBox::Ok);
            else
                QMessageBox::critical(this,tr("Save Result Failure"),tr("Save Statistic Result to XLSX File Falure."),QMessageBox::Ok);

        }
    }
    delete dialog;
}

bool StatisticDialog::SaveStatisticToCSVFile(const char* filename)
{
    unsigned int site_count = mSitesList.size();
    if(site_count == 0) return false;

    FILE *csv_file = fopen(filename, "wt");
    if(!csv_file) return false;


    for(unsigned int n = 0; n < site_count; n++)
    {
        TestSite* site = mSitesList.at(n);

        fprintf(csv_file, "%s Total Test %u Devices.\n",site->get_name(),site->get_device_count());
        fflush(csv_file);
        fprintf(csv_file, "TestNumber,TestLabel,LowLimit,HighLimit,TestUnit,Min,Max,Average,Range,Stdev,Cpl,Cpu,Cp,Cpk\n");
        fflush(csv_file);

        unsigned int item_count = site->get_item_count();

        for(unsigned int n = 0; n < item_count; n++)
        {
            TestItem* item = site->get_item(n);
            MySPC spc(item);
            spc.SetDefaltSpec(mDefaultLowLimit, mDefaultHighLimit);
            fprintf(csv_file, "%u,", item->get_number());
            fprintf(csv_file, "\"%s\",", item->get_label());
            fprintf(csv_file, "%f,", item->get_lowlimit());
            fprintf(csv_file, "%f,", item->get_highlimit());
            fprintf(csv_file, "%s,", item->get_unit());
            fprintf(csv_file, "%f,", item->get_min());
            fprintf(csv_file, "%f,", item->get_max());
            fprintf(csv_file, "%f,", item->get_average());
            fprintf(csv_file, "%f,", item->get_range());
            fprintf(csv_file, "%f,",item->get_stdev());

            double cpl = spc.GetCpl();
            double cpu = spc.GetCpu();
            double cpk = spc.GetCpk();
            double cp  = spc.GetCp();

            if(cpl > 0) fprintf(csv_file, "%f,", cpl);
            else fprintf(csv_file, "N/A,");

            if(cpu > 0) fprintf(csv_file, "%f,", cpu);
            else fprintf(csv_file, "N/A,");

            if(cp > 0) fprintf(csv_file, "%f,", cp);
            else fprintf(csv_file, "N/A,");

            if(cpk > 0) fprintf(csv_file, "%f,", cpk);
            else fprintf(csv_file, "N/A,");

            fprintf(csv_file, "\n");
            fflush(csv_file);
        }

        fprintf(csv_file, "\n");
        fflush(csv_file);

    }
    fclose(csv_file);
    return true;
}

bool StatisticDialog::SaveStatisticToXLSFile(const char* filename)
{
    unsigned int site_count = mSitesList.size();
    if(site_count == 0) return false;

    QXlsx::Document xlsx;
    xlsx.addSheet("Statistic Result");
    QString qfilename = QString::fromLocal8Bit(filename);


    unsigned int row_number  = 1;
    for(unsigned int n = 0; n < site_count; n++)
    {
        TestSite* site = mSitesList.at(n);
        QString wsite_name = QString::fromLocal8Bit(site->get_name());

        unsigned int col = 1;
        xlsx.write(row_number, col, wsite_name);
        row_number++;
        xlsx.write(row_number, col++, "TestNumber");
        xlsx.write(row_number, col++, "TestLabel");
        xlsx.write(row_number, col++, "LowLimit");
        xlsx.write(row_number, col++, "HighLimit");
        xlsx.write(row_number, col++, "TestUnit");
        xlsx.write(row_number, col++, "Min");
        xlsx.write(row_number, col++, "Max");
        xlsx.write(row_number, col++, "Average");
        xlsx.write(row_number, col++, "Range");
        xlsx.write(row_number, col++, "Stdev");
        xlsx.write(row_number, col++, "Cpl");
        xlsx.write(row_number, col++, "Cpu");
        xlsx.write(row_number, col++, "Cp");
        xlsx.write(row_number, col++, "Cpk");

        row_number++;
        unsigned int item_count = site->get_item_count();

        for(unsigned int n = 0; n < item_count; n++)
        {
            TestItem* item = site->get_item(n);
            MySPC spc(item);
            spc.SetDefaltSpec(mDefaultLowLimit, mDefaultHighLimit);

            QString witem_name = QString::fromLocal8Bit(item->get_label());
            QString wunit = QString::fromLocal8Bit(item->get_unit());

            unsigned int col_number = 1;

            xlsx.write(row_number, col_number++, item->get_number());
            xlsx.write(row_number, col_number++, witem_name);
            xlsx.write(row_number, col_number++, item->get_lowlimit());
            xlsx.write(row_number, col_number++, item->get_highlimit());
            xlsx.write(row_number, col_number++, wunit);
            xlsx.write(row_number, col_number++, item->get_min());
            xlsx.write(row_number, col_number++, item->get_max());
            xlsx.write(row_number, col_number++, item->get_average());
            xlsx.write(row_number, col_number++, item->get_range());
            xlsx.write(row_number, col_number++, item->get_stdev());

            double cpl = spc.GetCpl();
            double cpu = spc.GetCpu();
            double cpk = spc.GetCpk();
            double cp  = spc.GetCp();


            if(cpl > 0)  xlsx.write(row_number, col_number++, cpl);
            else  xlsx.write(row_number, col_number++, "N/A");
            if(cpu > 0)  xlsx.write(row_number, col_number++, cpu);
            else   xlsx.write(row_number, col_number++, "N/A");

            QXlsx::Format format_red;
            format_red.setPatternBackgroundColor(QColor(Qt::red));

            QXlsx::Format format_green;
            format_green.setPatternBackgroundColor(QColor(Qt::green));

            QXlsx::Format format_yellow;
            format_yellow.setPatternBackgroundColor(QColor(Qt::yellow));

            if(cp > 0 && cp < 1.33)   xlsx.write(row_number, col_number++, cp, format_red);
            else if(cp >= 1.33 && cp < 1.67 ) xlsx.write(row_number, col_number++, cp, format_yellow);
            else if(cp >= 1.67 ) xlsx.write(row_number, col_number++, cp, format_green);
            else  xlsx.write(row_number, col_number++, "N/A");

            if(cpk > 0 && cpk < 1.33)   xlsx.write(row_number, col_number++, cpk, format_red);
            else if(cpk >= 1.33 && cpk < 1.67 ) xlsx.write(row_number, col_number++, cpk, format_yellow);
            else if(cpk >= 1.67 ) xlsx.write(row_number, col_number++, cpk, format_green);
            else  xlsx.write(row_number, col_number++, "N/A");

            row_number++;
        }
        row_number++;
    }
    bool result = xlsx.saveAs(qfilename);
    return result;
}
