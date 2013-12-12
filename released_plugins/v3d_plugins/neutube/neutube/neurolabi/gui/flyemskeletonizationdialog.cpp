#include "flyemskeletonizationdialog.h"
#include "ui_flyemskeletonizationdialog.h"

FlyEmSkeletonizationDialog::FlyEmSkeletonizationDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmSkeletonizationDialog)
{
  ui->setupUi(this);
  connect(ui->connectCheckBox, SIGNAL(stateChanged(int)),
          this, SLOT(enableDistanceThreshold()));
  connect(ui->objSizeCheckBox, SIGNAL(stateChanged(int)),
          this, SLOT(enableSizeThreshold()));
}

FlyEmSkeletonizationDialog::~FlyEmSkeletonizationDialog()
{
  delete ui;
}

int FlyEmSkeletonizationDialog::lengthThreshold()
{
  return ui->lengthSpinBox->value();
}

int FlyEmSkeletonizationDialog::distanceThreshold()
{
  return ui->distThreSpinBox->value();
}

int FlyEmSkeletonizationDialog::sizeThreshold()
{
  return ui->objSizeSpinBox->value();
}

bool FlyEmSkeletonizationDialog::isConnectingAll()
{
  return ui->connectCheckBox->isChecked();
}

bool FlyEmSkeletonizationDialog::isExcludingSmallObj()
{
  return ui->objSizeCheckBox->isChecked();
}

bool FlyEmSkeletonizationDialog::isKeepingShortObject()
{
  return ui->shortObjectCheckBox->isChecked();
}

void FlyEmSkeletonizationDialog::enableDistanceThreshold()
{
  ui->distThreSpinBox->setEnabled(!ui->connectCheckBox->isChecked());
}

void FlyEmSkeletonizationDialog::enableSizeThreshold()
{
  ui->objSizeSpinBox->setEnabled(ui->objSizeCheckBox->isChecked());
}

bool FlyEmSkeletonizationDialog::isLevelChecked()
{
  return ui->greyToBinaryCheckBox->isChecked();
}

int FlyEmSkeletonizationDialog::level()
{
  return ui->levelSpinBox->value();
}
