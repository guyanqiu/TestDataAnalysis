#include "convertfiledialog.h"
#include "ui_convertfiledialog.h"
#include <QMessageBox>

ConvertFileDialog::ConvertFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConvertFileDialog)
{
    ui->setupUi(this);
    mCurrentIndex = -1;
}

ConvertFileDialog::~ConvertFileDialog()
{
    delete ui;
}

void ConvertFileDialog::on_buttonBox_accepted()
{
    mCurrentIndex = ui->FilenameComboBox->currentIndex();
    if(mCurrentIndex < 0)
    {
        QMessageBox::critical(this, tr("No File"), tr("Need Open DataLog File First."));
        return;
    }
    done(QDialog::Accepted);
}


void ConvertFileDialog::SetDataLogs(QStringList& name_list)
{
    int name_count = name_list.count();
    if(name_count == 0)
    {
        ui->FilenameComboBox->clear();
        return;
    }

    ui->FilenameComboBox->clear();
    ui->FilenameComboBox->addItems(name_list);
    ui->FilenameComboBox->setCurrentIndex(0);
}

int ConvertFileDialog::GetCurrentIndex()
{
    return mCurrentIndex;
}

void ConvertFileDialog::on_FilenameComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    mCurrentIndex = ui->FilenameComboBox->currentIndex();
}
