#include "dvidobjectdialog.h"
#include "ui_dvidobjectdialog.h"

DvidObjectDialog::DvidObjectDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DvidObjectDialog)
{
  ui->setupUi(this);
}

DvidObjectDialog::~DvidObjectDialog()
{
  delete ui;
}

void DvidObjectDialog::setAddress(const QString &address)
{
  ui->addressLineEdit->setText(address);
}

QString DvidObjectDialog::getAddress() const
{
  return ui->addressLineEdit->text();
}

int DvidObjectDialog::getBodyId() const
{
  return ui->bodySpinBox->value();
}
