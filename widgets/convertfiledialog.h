#ifndef CONVERTFILEDIALOG_H
#define CONVERTFILEDIALOG_H

#include <QDialog>

namespace Ui {
class ConvertFileDialog;
}

class ConvertFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConvertFileDialog(QWidget *parent = 0);
    ~ConvertFileDialog();
    int GetCurrentIndex();
    void SetDataLogs(QStringList& name_list);

private slots:
    void on_buttonBox_accepted();


    void on_FilenameComboBox_currentIndexChanged(int index);

private:
    Ui::ConvertFileDialog *ui;
    int mCurrentIndex;
};

#endif // CONVERTFILEDIALOG_H
