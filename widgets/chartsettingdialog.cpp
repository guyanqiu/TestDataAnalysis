#include "chartsettingdialog.h"
#include "ui_chartsettingdialog.h"
#include <QMessageBox>

ChartSettingDialog::ChartSettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChartSettingDialog)
{
    ui->setupUi(this);
}

ChartSettingDialog::~ChartSettingDialog()
{
    delete ui;
}

void ChartSettingDialog::on_buttonBox_accepted()
{
    unsigned int max_count = GetMaxHistoCount();
    unsigned int min_count = GetMinHistoCount();
    double high_limit = GetDefaultHighLimit();
    double low_limit = GetDefaultLowLimit();
    if(max_count < min_count)
    {
        QMessageBox::critical(this, tr("Max Count < Min Count"), tr("Max Count Must Greater or Equal to Min Count."));
        return;
    }
    if(high_limit < low_limit)
    {
        QMessageBox::critical(this, tr("Default HighLimit < Default LowLimit"), tr("Default HighLimit Must Greater or Equal to Default LowLimit."));
        return;
    }
    done(QDialog::Accepted);
}

void ChartSettingDialog::on_buttonBox_rejected()
{
    done(QDialog::Rejected);
}

void ChartSettingDialog::SetDefaultLowLimit(double low_limit)
{
    QString str_low = QString("%1").arg(low_limit);
    ui->LowLimitLineEdit->setText(str_low);
}

void ChartSettingDialog::SetDefaultHighLimit(double high_limit)
{
    QString str_high = QString("%1").arg(high_limit);
    ui->HighLimitLineEdit->setText(str_high);
}

void ChartSettingDialog::SetMaxHistoCount(unsigned int max_count)
{
    ui->MaxCountSpinBox->setValue(max_count);
}

void ChartSettingDialog::SetMinHistoCount(unsigned int min_count)
{
    ui->MinCountSpinBox->setValue(min_count);
}

double ChartSettingDialog::GetDefaultLowLimit()
{
    QString str_low = ui->LowLimitLineEdit->text();
    return str_low.toDouble();
}

double ChartSettingDialog::GetDefaultHighLimit()
{
    QString str_high = ui->HighLimitLineEdit->text();
    return str_high.toDouble();
}

unsigned int ChartSettingDialog::GetMaxHistoCount()
{
    return (unsigned int)(ui->MaxCountSpinBox->value());
}

unsigned int ChartSettingDialog::GetMinHistoCount()
{
    return (unsigned int)(ui->MinCountSpinBox->value());
}
