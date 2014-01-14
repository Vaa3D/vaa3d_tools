#include "regionexpanddialog.h"
#include "ui_regionexpanddialog.h"

RegionExpandDialog::RegionExpandDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::RegionExpandDialog)
{
  ui->setupUi(this);
}

RegionExpandDialog::~RegionExpandDialog()
{
  delete ui;
}

int RegionExpandDialog::getRadius()
{
  return ui->radiusSpinBox->value();
}
