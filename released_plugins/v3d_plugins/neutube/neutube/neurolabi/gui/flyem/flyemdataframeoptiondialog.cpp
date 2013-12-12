#include "flyemdataframeoptiondialog.h"
#include "ui_flyemdataframeoptiondialog.h"

FlyEmDataFrameOptionDialog::FlyEmDataFrameOptionDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmDataFrameOptionDialog)
{
  ui->setupUi(this);
}

FlyEmDataFrameOptionDialog::~FlyEmDataFrameOptionDialog()
{
  delete ui;
}

void FlyEmDataFrameOptionDialog::setResampleStep(double step)
{
  ui->resampleStepSpinBox->setValue(step);
}

void FlyEmDataFrameOptionDialog::setTrunkStep(double step)
{
  ui->trunkStepSpinBox->setValue(step);
}

void FlyEmDataFrameOptionDialog::setLayerScale(double scale)
{
  ui->layerScaleSpinBox->setValue(scale);
}


void FlyEmDataFrameOptionDialog::setLayerMargin(double margin)
{
  ui->layerMarginSpinBox->setValue(margin);
}

double FlyEmDataFrameOptionDialog::getTrunkStep()
{
  return ui->trunkStepSpinBox->value();
}

double FlyEmDataFrameOptionDialog::getLayerScale()
{
  return ui->layerScaleSpinBox->value();
}

double FlyEmDataFrameOptionDialog:: getLayerMargin()
{
  return ui->layerMarginSpinBox->value();
}
