#include "swcskeletontransformdialog.h"
#include "ui_swcskeletontransformdialog.h"

SwcSkeletonTransformDialog::SwcSkeletonTransformDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SwcSkeletonTransformDialog)
{
  ui->setupUi(this);
}

SwcSkeletonTransformDialog::~SwcSkeletonTransformDialog()
{
  delete ui;
}

double SwcSkeletonTransformDialog::getScaleValue(EDimension dim)
{
  switch (dim) {
  case X:
    return ui->scaleXSpinBox->value();
  case Y:
    return ui->scaleYSpinBox->value();
  case Z:
    return ui->scaleZSpinBox->value();
  }

  return 1.0;
}

double SwcSkeletonTransformDialog::getTranslateValue(EDimension dim)
{
  switch (dim) {
  case X:
    return ui->translateXSpinBox->value();
  case Y:
    return ui->translateYSpinBox->value();
  case Z:
    return ui->translateZSpinBox->value();
  }

  return 0.0;
}

bool SwcSkeletonTransformDialog::isTranslateFirst()
{
  return ui->translateRadioButton->isChecked();
}

void SwcSkeletonTransformDialog::setTranslateValue(
    double dx, double dy, double dz)
{
  ui->translateXSpinBox->setValue(dx);
  ui->translateYSpinBox->setValue(dy);
  ui->translateZSpinBox->setValue(dz);
}

void SwcSkeletonTransformDialog::setScaleValue(double sx, double sy, double sz)
{
  ui->scaleXSpinBox->setValue(sx);
  ui->scaleYSpinBox->setValue(sy);
  ui->scaleZSpinBox->setValue(sz);
}
