#include "corrlationdialog.h"
#include "ui_corrlationdialog.h"
#include <QFileDialog>
#include <QMessageBox>

CorrlationDialog::CorrlationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CorrlationDialog)
{
    ui->setupUi(this);
    Qt::WindowFlags flags = this->windowFlags();
    flags  |= (Qt::WindowMaximizeButtonHint|Qt::WindowMinimizeButtonHint|Qt::WindowCloseButtonHint);
    this->setWindowFlags(flags);
    ui->CorrTableWidget->setDragEnabled(true);
    mCorr = NULL;
}

CorrlationDialog::~CorrlationDialog()
{
    delete ui;
}

void CorrlationDialog::on_ExitButton_clicked()
{
    close();
}
QTableWidget *CorrlationDialog::GetCorrelationTable()
{
    return ui->CorrTableWidget;
}

void CorrlationDialog::SetSitesCorr(TestSite_CORR* corr)
{
    mCorr = corr;
}

void CorrlationDialog::on_SaveButton_clicked()
{
    QFileDialog *dialog = new QFileDialog(this);
    dialog->setWindowTitle(tr("Save Correlation Result"));
    QStringList filters;
    QString csv_filter("CSV Files(*.csv)");
    QString xls_filter("XLS Files(*.xlsx)");
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
            if(mCorr->save_result_csv(filename.toLocal8Bit().data()))
                QMessageBox::information(this,tr("Save Result Success"),tr("Save Correlation Result to CSV File Success."),QMessageBox::Ok);
            else
                QMessageBox::critical(this,tr("Save Result Failure"),tr("Save Correlation Result to CSV File Falure."),QMessageBox::Ok);
        }
        else
        {
            if(file_suffux != "xlsx") filename += tr(".xlsx");
            if(mCorr->save_result_xls(filename.toLocal8Bit().data()))
                QMessageBox::information(this,tr("Save Result Success"),tr("Save Correlation Result to XLSX File Success."),QMessageBox::Ok);
            else
                QMessageBox::critical(this,tr("Save Result Failure"),tr("Save Correlation Result to XLSX File Falure."),QMessageBox::Ok);

        }
    }
	delete dialog;
}
