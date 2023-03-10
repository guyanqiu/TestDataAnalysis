#ifndef STDF_WINDOW_H
#define STDF_WINDOW_H

#include "stdf_v4_file.h"
#include <vector>
#include <QDialog>

namespace Ui {
class StdfDialog;
}

class StdfDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StdfDialog(QWidget *parent = 0);
    ~StdfDialog();

private slots:
    void on_OpenButton_clicked();
    void on_ClearButton_clicked();
    void on_CloseButton_clicked();
    void on_RecordListWidget_clicked(const QModelIndex &index);
    void on_SaveButton_clicked();
    void on_SaveAsButton_clicked();
    void on_SaveXLS_clicked();
    void SaveTableToFile();

private:
    void UpdateUi();
    void ShowRecordTable(STDF_TYPE type);
    void ShowStdfFAR();
	void ShowStdfATR();
	void ShowStdfMIR();
	void ShowStdfMRR();
	void ShowStdfPCR();
	void ShowStdfHBR();
	void ShowStdfSBR();
	void ShowStdfPMR();
	void ShowStdfPGR();
	void ShowStdfPLR();
	void ShowStdfRDR();
	void ShowStdfSDR();
	void ShowStdfWIR();
	void ShowStdfWRR();
	void ShowStdfWCR();
	void ShowStdfPIR();
	void ShowStdfPRR();
	void ShowStdfTSR();
	void ShowStdfPTR();
	void ShowStdfMPR();
	void ShowStdfFTR();
	void ShowStdfBPS();
	void ShowStdfEPS();
	void ShowStdfGDR();
	void ShowStdfDTR();
    STDF_FILE_ERROR save_to_xls(const char* filename);

private:
    Ui::StdfDialog *ui;
    STDF_FILE *stdf_file;
    QString filename;
    QAction *save_action;
    std::vector<int> stdf_types;
    void table_set_value(int row, int col, unsigned int value);
    void table_set_value(int row, int col, int value);
    void table_set_value(int row, int col, unsigned short value);
    void table_set_value(int row, int col, const char* value);
    void table_set_value(int row, int col, time_t value);
    void table_set_value(int row, int col, char value);
    void table_set_value(int row, int col, unsigned char value);
    void table_set_value(int row, int col, signed char value);
    void table_set_value(int row, int col, float value);
    void table_set_value(int row, int col, short value);
    void table_set_value(int row, int col, QString value);
    void table_set_flag(int row, int col, unsigned char flag);
};


#endif // STDF_WINDOW_H
