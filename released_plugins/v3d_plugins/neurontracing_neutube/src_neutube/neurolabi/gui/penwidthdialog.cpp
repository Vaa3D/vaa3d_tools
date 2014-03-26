#include "penwidthdialog.h"
#include "ui_penwidthdialog.h"

PenWidthDialog::PenWidthDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::PenWidthDialog)
{
  ui->setupUi(this);
}

PenWidthDialog::~PenWidthDialog()
{
  delete ui;
}

void PenWidthDialog::setPenWidth(double width)
{
  ui->widthDoubleSpinBox->setValue(width);
}

double PenWidthDialog::getPenWidth()
{
  return ui->widthDoubleSpinBox->value();
}
