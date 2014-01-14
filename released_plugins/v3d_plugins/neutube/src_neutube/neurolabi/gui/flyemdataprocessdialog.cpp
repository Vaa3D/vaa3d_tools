#include "flyemdataprocessdialog.h"
#include "ui_flyemdataprocessdialog.h"

FlyEmDataProcessDialog::FlyEmDataProcessDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmDataProcessDialog)
{
  ui->setupUi(this);
}

FlyEmDataProcessDialog::~FlyEmDataProcessDialog()
{
  delete ui;
}

QString FlyEmDataProcessDialog::getSourceString()
{
  return ui->sourceLineEdit->text();
}

QString FlyEmDataProcessDialog::getSourceType()
{
  return ui->sourceComboBox->currentText();
}

QString FlyEmDataProcessDialog::getAction()
{
  return ui->actionComboBox->currentText();
}
