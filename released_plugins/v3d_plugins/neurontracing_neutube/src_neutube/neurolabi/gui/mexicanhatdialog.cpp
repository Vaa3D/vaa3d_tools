#include "mexicanhatdialog.h"
#include "ui_mexicanhatdialog.h"

MexicanHatDialog::MexicanHatDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::MexicanHatDialog)
{
  ui->setupUi(this);
}

MexicanHatDialog::~MexicanHatDialog()
{
  delete ui;
}

double MexicanHatDialog::sigma()
{
  return ui->sigmaSpinBox->value();
}
