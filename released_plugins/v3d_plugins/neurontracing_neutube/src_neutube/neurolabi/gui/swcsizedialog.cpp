#include "swcsizedialog.h"
#include "ui_swcsizedialog.h"

SwcSizeDialog::SwcSizeDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SwcSizeDialog)
{
  ui->setupUi(this);
}

SwcSizeDialog::~SwcSizeDialog()
{
  delete ui;
}

double SwcSizeDialog::getAddValue()
{
  return ui->doubleAddSpinBox->value();
}

double SwcSizeDialog::getMulValue()
{
  return ui->doubleScaleSpinBox->value();
}
