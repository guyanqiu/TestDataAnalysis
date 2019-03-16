#ifndef CHARTSETTINGDIALOG_H
#define CHARTSETTINGDIALOG_H

#include <QDialog>

namespace Ui {
class ChartSettingDialog;
}

class ChartSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChartSettingDialog(QWidget *parent = 0);
    ~ChartSettingDialog();
    void SetDefaultLowLimit(double low_limit);
    void SetDefaultHighLimit(double high_limit);
    void SetMaxHistoCount(unsigned int max_count);
    void SetMinHistoCount(unsigned int min_count);
    double GetDefaultLowLimit();
    double GetDefaultHighLimit();
    unsigned int GetMaxHistoCount();
    unsigned int GetMinHistoCount();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::ChartSettingDialog *ui;
};

#endif // CHARTSETTINGDIALOG_H
