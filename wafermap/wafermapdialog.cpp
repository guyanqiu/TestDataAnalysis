#include "wafermapdialog.h"
#include "ui_wafermapdialog.h"

WaferMapDialog::WaferMapDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WaferMapDialog)
{
    ui->setupUi(this);
    update_flag = false;
}

WaferMapDialog::~WaferMapDialog()
{
    delete ui;
}

bool WaferMapDialog::SetDataLog(DataLog* datalog)
{
    update_flag = ui->widget->SetDataLog(datalog);
    if(update_flag)
    {
        ui->tableWidget->clearContents();
        unsigned int bin_count = ui->widget->GetBinCount();
        ui->tableWidget->setRowCount(bin_count);
        ui->tableWidget->setColumnWidth(0, 75);
        ui->tableWidget->setColumnWidth(1, 75);
        ui->tableWidget->setColumnWidth(2, 200);
        for(unsigned int i = 0; i < bin_count; i++)
        {
            unsigned int bin_number = ui->widget->GetHBinNumber(i);
            QTableWidgetItem* bin_item = new QTableWidgetItem(QString("BIN%1").arg(bin_number));
            ui->tableWidget->setItem(i, 0, bin_item);

            QColor color = ui->widget->GetBinColor(bin_number);
            QTableWidgetItem* color_item = new QTableWidgetItem();
            if(ui->widget->show_bin_count())
            {
                unsigned int bin_count = ui->widget->GetHbinDeviceCount(bin_number);
                color_item->setText(QString("%1").arg(bin_count));
            }
            color_item->setBackgroundColor(color);
            ui->tableWidget->setItem(i, 1, color_item);

            QString bin_name = ui->widget->GetSBinName(bin_number);
            QTableWidgetItem* bin_name_item = new QTableWidgetItem(bin_name);
            ui->tableWidget->setItem(i, 2, bin_name_item);
        }
    }
    return update_flag;
}

