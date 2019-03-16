#ifndef STATISTICDIALOG_H
#define STATISTICDIALOG_H

#include <vector>
#include "statistics.h"
#include <QDialog>

class QTableWidget;
namespace Ui {
class StatisticDialog;
}

class StatisticDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatisticDialog(QWidget *parent = 0);
    ~StatisticDialog();
    QTableWidget *GetStatisticTable();
    void SetSiteList(std::vector<TestSite*>& site_list);
    void SetDefaultLimit(double default_lowlimit, double default_highlimit);

private slots:
    void on_pushButton_Ok_clicked();
    void on_SaveButton_clicked();

private:
    bool SaveStatisticToCSVFile(const char* filename);
    bool SaveStatisticToXLSFile(const char* filename);

private:
    Ui::StatisticDialog *ui;
    double mDefaultLowLimit, mDefaultHighLimit;
    std::vector<TestSite*> mSitesList;

};

#endif // STATISTICDIALOG_H
