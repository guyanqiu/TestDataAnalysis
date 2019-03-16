#ifndef WAFERMAPDIALOG_H
#define WAFERMAPDIALOG_H

#include <QDialog>
#include "../datalog/datalog.h"

namespace Ui {
class WaferMapDialog;
}

class WaferMapDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WaferMapDialog(QWidget *parent = 0);
    ~WaferMapDialog();
    bool SetDataLog(DataLog* datalog);

private:
    Ui::WaferMapDialog *ui;
    bool update_flag;
};

#endif // WAFERMAPDIALOG_H
