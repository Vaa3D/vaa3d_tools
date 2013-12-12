#include "zalphadialog.h"
#include "ui_zalphadialog.h"

ZAlphaDialog::ZAlphaDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZAlphaDialog)
{
  ui->setupUi(this);
}

ZAlphaDialog::~ZAlphaDialog()
{
  delete ui;
}

int ZAlphaDialog::getAlpha()
{
  return ui->alphaSpinBox->value();
}
