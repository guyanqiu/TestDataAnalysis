#include "calsettingdialog.h"
#include "ui_calsettingdialog.h"
#include <QMessageBox>

CalSettingDialog::CalSettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalSettingDialog)
{
    ui->setupUi(this);
}

CalSettingDialog::~CalSettingDialog()
{
    delete ui;
}

void CalSettingDialog::on_buttonBox_accepted()
{
    double low = GetDefaultLowLimit();
    double high = GetDefaultHighLimit();
    if(high <= low)
    {
        QMessageBox::critical(this, tr("Default HighLimit <= Default LowLimit"), tr("Default HighLimit Must Greater than Default LowLimit."));
        return;
    }
    done(QDialog::Accepted);
}

void CalSettingDialog::on_buttonBox_rejected()
{
    done(QDialog::Rejected);
}

void CalSettingDialog::SetDefaultLowLimit(double low_limit)
{
    QString str_low = QString("%1").arg(low_limit);
    ui->LowLimitLineEdit->setText(str_low);
}

void CalSettingDialog::SetDefaultHighLimit(double high_limit)
{
    QString str_high = QString("%1").arg(high_limit);
    ui->HighLimitLineEdit->setText(str_high);
}

void CalSettingDialog::SetEVMode(int mode)
{
    if(mode == 0)
        ui->Mode0RadioButton->setChecked(true);
    else
        ui->Mode1RadioButton->setChecked(true);
}

void CalSettingDialog::SetCorrelationMode(int mode)
{
    if(mode == 0)
        ui->radioButton_Corr1->setChecked(true);
    else
        ui->radioButton_Corr2->setChecked(true);
}

int CalSettingDialog::GetEVMode()
{
    if(ui->Mode0RadioButton->isChecked())
        return 0;
    else
        return 1;
}

int CalSettingDialog::GetCorrelationMode()
{
    if(ui->radioButton_Corr1->isChecked())
        return 0;
    else
        return 1;
}

double CalSettingDialog::GetDefaultHighLimit()
{
    QString str_high = ui->HighLimitLineEdit->text();
    return str_high.toDouble();
}

double CalSettingDialog::GetDefaultLowLimit()
{
    QString str_low = ui->LowLimitLineEdit->text();
    return str_low.toDouble();
}

void CalSettingDialog::SetCorrelationSpec(double percent_spec_value)
{
    ui->CorrelationSpecSpinBox->setValue(percent_spec_value);
}

double CalSettingDialog::GetCorrelationSpecValue()
{
    return (ui->CorrelationSpecSpinBox->value() / 100.0);
}
