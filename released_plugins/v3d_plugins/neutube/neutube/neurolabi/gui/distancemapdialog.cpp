#include "distancemapdialog.h"
#include "ui_distancemapdialog.h"

DistanceMapDialog::DistanceMapDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DistanceMapDialog)
{
  ui->setupUi(this);
}

DistanceMapDialog::~DistanceMapDialog()
{
  delete ui;
}

bool DistanceMapDialog::isSliceWise()
{
  return ui->checkBoxSlice->isChecked();
}

bool DistanceMapDialog::isForegroundTransform()
{
  return !(ui->checkBoxBackground->isChecked());
}

bool DistanceMapDialog::isSquared()
{
  return ui->checkBoxSquare->isChecked();
}
