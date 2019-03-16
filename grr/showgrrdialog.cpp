#include "showgrrdialog.h"
#include "ui_showgrrdialog.h"
#include <QListWidgetItem>
#include <QMessageBox>
#include <QFileDialog>

ShowGRRDialog::ShowGRRDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShowGRRDialog)
{
    ui->setupUi(this);
    Qt::WindowFlags flags = this->windowFlags();
    flags  |= (Qt::WindowMaximizeButtonHint|Qt::WindowMinimizeButtonHint|Qt::WindowCloseButtonHint);
    this->setWindowFlags(flags);

    mShowTestItems = false;
    mGRR_RowNumber = 11;
    ui->ItemListGroupBox->hide();
    mGrr = NULL;
}

ShowGRRDialog::~ShowGRRDialog()
{
    delete ui;
}

void ShowGRRDialog::on_OkButton_clicked()
{
    ui->GrrTableWidget->clearContents();
    ui->GrrTableWidget->clear();

    this->hide();
}

QTableWidget* ShowGRRDialog::GetGRRTable()
{
    return ui->GrrTableWidget;
}

QListWidget* ShowGRRDialog::GetItemList()
{
    return ui->TestItemListWidget;
}

void ShowGRRDialog::SetItemToList(QStringList& item_lables)
{
    ui->TestItemListWidget->clear();

    int nCol = item_lables.count();
    for(int n = 0; n < nCol; n++)
    {
        ui->TestItemListWidget->addItem(item_lables.at(n));
        QListWidgetItem *item = ui->TestItemListWidget->item(n);
        item->setCheckState(Qt::Checked);
    }
}

void ShowGRRDialog::on_TestItemListWidget_currentRowChanged(int currentRow)
{
    QListWidgetItem *item = ui->TestItemListWidget->item(currentRow);
    if(item)
    {
        if(item->checkState() == Qt::Checked)
        {
            QTableWidgetItem *item = ui->GrrTableWidget->item(11,currentRow); // GR&R Row Number=11
            if(!item) return;

            ui->GrrTableWidget->selectColumn(currentRow);
            ui->GrrTableWidget->setItemSelected(item, false);
        }
    }
}

void ShowGRRDialog::on_ShowAllItemsButton_clicked()
{
    int col_count = ui->TestItemListWidget->count();
    for(int n = 0; n < col_count; n++)
    {
        QTableWidgetItem *table_item = ui->GrrTableWidget->item(mGRR_RowNumber,n);
        QListWidgetItem *list_item = ui->TestItemListWidget->item(n);
        if(!table_item) continue;
        if(!list_item) continue;

        list_item->setCheckState(Qt::Checked);
        ui->GrrTableWidget->showColumn(n);
    }
}

void ShowGRRDialog::on_ShowFailItemButton_clicked()
{
    int col_count = ui->TestItemListWidget->count();
    for(int n = 0; n < col_count; n++)
    {
        QTableWidgetItem *table_item = ui->GrrTableWidget->item(mGRR_RowNumber,n);
        QListWidgetItem *list_item = ui->TestItemListWidget->item(n);
        if(!table_item) continue;
        if(!list_item) continue;

        if(table_item->backgroundColor() != Qt::green)
        {
            list_item->setCheckState(Qt::Checked);
            ui->GrrTableWidget->showColumn(n);
        }
        else
        {
            list_item->setCheckState(Qt::Unchecked);
            ui->GrrTableWidget->hideColumn(n);
        }
    }
}

void ShowGRRDialog::on_SelectAllButton_clicked()
{
    int col_count = ui->TestItemListWidget->count();
    for(int n = 0; n < col_count; n++)
    {
        QListWidgetItem *item = ui->TestItemListWidget->item(n);
        item->setCheckState(Qt::Checked);
        ui->GrrTableWidget->showColumn(n);
    }
}

void ShowGRRDialog::on_UnSelectAllButton_clicked()
{
    int cocol_countl = ui->TestItemListWidget->count();
    for(int n = 0; n < cocol_countl; n++)
    {
        QListWidgetItem *item = ui->TestItemListWidget->item(n);
        item->setCheckState(Qt::Unchecked);
        ui->GrrTableWidget->hideColumn(n);
    }
}

void ShowGRRDialog::on_TestItemListWidget_clicked(const QModelIndex &index)
{
    int n = index.row();
    QListWidgetItem *item = ui->TestItemListWidget->item(n);
    if(item)
    {
        if(item->checkState() == Qt::Unchecked)
        {
            ui->GrrTableWidget->hideColumn(n);
        }
        else
        {
            ui->GrrTableWidget->showColumn(n);

            QTableWidgetItem *item = ui->GrrTableWidget->item(mGRR_RowNumber,n); // GR&R Row Number=11
            if(!item) return;

            ui->GrrTableWidget->selectColumn(n);
            ui->GrrTableWidget->setItemSelected(item, false);
        }
    }
}

void ShowGRRDialog::on_SelectItemButton_clicked()
{
    if(!mShowTestItems)
    {
        ui->ItemListGroupBox->show();
        mShowTestItems = true;
        ui->SelectItemButton->setText("Hide TestItems");
    }
    else
    {
        ui->ItemListGroupBox->hide();
        mShowTestItems = false;
        ui->SelectItemButton->setText("Select TestItems");
    }
}

void ShowGRRDialog::on_SaveResultButton_clicked()
{
    if(mGrr)
    {
        QFileDialog *dialog = new QFileDialog(this);
        dialog->setWindowTitle(tr("Save GRR Result"));
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
                if(mGrr->save_result_csv(filename.toLocal8Bit().data()))
                    QMessageBox::information(this,tr("Save Result Success"),tr("Save GRR Result to CSV File Success."),QMessageBox::Ok);
                else
                    QMessageBox::critical(this,tr("Save Result Failure"),tr("Save GRR Result to CSV File Falure."),QMessageBox::Ok);

            }
            else
            {
                if(file_suffux != "xlsx") filename += tr(".xlsx");
                if(mGrr->save_result_xls(filename.toLocal8Bit().data()))
                    QMessageBox::information(this,tr("Save Result Success"),tr("Save GRR Result to XLSX File Success."),QMessageBox::Ok);
                else
                    QMessageBox::critical(this,tr("Save Result Failure"),tr("Save GRR Result to XLSX File Falure."),QMessageBox::Ok);
            }
        }
		delete dialog;
    }
}

void ShowGRRDialog::SetSitesGRR(TestSite_GRR* grr)
{
    mGrr = grr;
}
