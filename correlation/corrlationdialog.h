#ifndef CORRLATIONDIALOG_H
#define CORRLATIONDIALOG_H

#include <QDialog>
#include "correlation.h"

class QTableWidget;
namespace Ui {
class CorrlationDialog;
}

class CorrlationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CorrlationDialog(QWidget *parent = 0);
    ~CorrlationDialog();
    void SetSitesCorr(TestSite_CORR* corr);
    QTableWidget *GetCorrelationTable();

private slots:
    void on_ExitButton_clicked();

    void on_SaveButton_clicked();

private:
    Ui::CorrlationDialog *ui;
    TestSite_CORR* mCorr;
};

#endif // CORRLATIONDIALOG_H
