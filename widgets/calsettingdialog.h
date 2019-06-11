#ifndef CALSETTINGDIALOG_H
#define CALSETTINGDIALOG_H

#include <QDialog>

namespace Ui {
class CalSettingDialog;
}

class CalSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalSettingDialog(QWidget *parent = 0);
    ~CalSettingDialog();
    void SetDefaultLowLimit(double low_limit);
    void SetDefaultHighLimit(double high_limit);

    void SetGRRMode(int mode);
    int GetGRRMode();
    double GetDefaultHighLimit();
    double GetDefaultLowLimit();

    void SetCorrelationSpec(double percent_spec_value);
    double GetCorrelationSpecValue();
    void SetCorrelationMode(int mode);
    int GetCorrelationMode();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::CalSettingDialog *ui;
};

#endif // CALSETTINGDIALOG_H
