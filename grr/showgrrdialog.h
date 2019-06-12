#ifndef SHOWGRRDIALOG_H
#define SHOWGRRDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QListWidget>
#include "grr.h"

namespace Ui {
class ShowGRRDialog;
}

class ShowGRRDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShowGRRDialog(QWidget *parent = 0);
    ~ShowGRRDialog();

    QTableWidget *GetGRRTable();
    QListWidget *GetItemList();
    void SetItemToList(QStringList& strList);
    void SetSitesGRR(TestSite_GRR* grr);

private slots:
    void on_TestItemListWidget_currentRowChanged(int currentRow);

    void on_ShowAllItemsButton_clicked();

    void on_ShowFailItemButton_clicked();

    void on_SelectAllButton_clicked();

    void on_UnSelectAllButton_clicked();

    void on_TestItemListWidget_clicked(const QModelIndex &index);

    void on_SelectItemButton_clicked();

    void on_OkButton_clicked();

    void on_SaveResultButton_clicked();

    void on_ShowTCSFailButton_clicked();

private:
    Ui::ShowGRRDialog *ui;
    bool mShowTestItems;
    int mGRR_RowNumber;
    int mTCS_RowNumber;
    int mFtest_RowNumber;
    TestSite_GRR* mGrr;

};

#endif // SHOWGRRRDIALOG_H
