#include "flyemdataexportdialog.h"
#include "ui_flyemdataexportdialog.h"

FlyEmDataExportDialog::FlyEmDataExportDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmDataExportDialog)
{
  ui->setupUi(this);
}

FlyEmDataExportDialog::~FlyEmDataExportDialog()
{
  delete ui;
}
