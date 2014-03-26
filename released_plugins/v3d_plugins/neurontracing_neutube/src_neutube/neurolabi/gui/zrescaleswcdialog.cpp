#include "zrescaleswcdialog.h"
#include "ui_zrescaleswcdialog.h"

ZRescaleSwcDialog::ZRescaleSwcDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZRescaleSwcDialog)
{
  ui->setupUi(this);

  connect(ui->TranslateSomaCheckBox, SIGNAL(stateChanged(int)), this, SLOT(translateSomaCheckBoxChanged(int)));
  connect(ui->ScaleUseResolutionRadioButton, SIGNAL(clicked()), this, SLOT(setScaleMethod()));
  connect(ui->ScaleManuallyRadioButton, SIGNAL(clicked()), this, SLOT(setScaleMethod()));
  connect(ui->RescaleBranchRadiusCheckBox, SIGNAL(stateChanged(int)), this, SLOT(rescaleBranchRadiusCheckBoxChanged(int)));
  connect(ui->RescaleSomaRadiusCheckBox, SIGNAL(stateChanged(int)), this, SLOT(rescaleSomaRadiusCheckBoxChanged(int)));
  connect(ui->ReduceNSwcNodesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(reduceNSwcNodesCheckBoxChanged(int)));
}

ZRescaleSwcDialog::~ZRescaleSwcDialog()
{
  delete ui;
}

ZRescaleSwcSetting ZRescaleSwcDialog::getRescaleSetting()
{
  ZRescaleSwcSetting setting;
  setting.bTranslateSoma = ui->TranslateSomaCheckBox->isChecked();
  setting.translateDstX = ui->TranslateXDoubleSpinBox->value();
  setting.translateDstY = ui->TranslateYDoubleSpinBox->value();
  setting.translateDstZ = ui->TranslateZDoubleSpinBox->value();
  if (ui->ScaleUseResolutionRadioButton->isChecked()) {
    setting.scaleX = ui->TargetResolutionXYDoubleSpinBox->value()/ui->CurrentSwcResolutionXYDoubleSpinBox->value();
    setting.scaleY = ui->TargetResolutionXYDoubleSpinBox->value()/ui->CurrentSwcResolutionXYDoubleSpinBox->value();
    setting.scaleZ = ui->TargetResolutionZDoubleSpinBox->value()/ui->CurrentSwcResolutionZDoubleSpinBox->value();
  } else if (ui->ScaleManuallyRadioButton->isChecked()) {
    setting.scaleX = ui->ScaleXDoubleSpinBox->value();
    setting.scaleY = ui->ScaleYDoubleSpinBox->value();
    setting.scaleZ = ui->SacleZDoubleSpinBox->value();
  }
  setting.bRescaleBranchRadius = ui->RescaleBranchRadiusCheckBox->isChecked();
  setting.bRescaleSomaRadius = ui->RescaleSomaRadiusCheckBox->isChecked();
  setting.rescaleBranchRadiusScale = ui->RescaleBranchRadiusDoubleSpinBox->value();
  setting.rescaleSomaRadiusScale = ui->RescaleSomaRadiusDoubleSpinBox->value();
  setting.bReduceSwcNodes = ui->ReduceNSwcNodesCheckBox->isChecked();
  setting.reduceSwcNodesLengthThre = ui->ReduceSwcNodesDoubleSpinBox->value();
  setting.somaCutLevel = ui->SomaCutLevelSpinBox->value();
  return setting;
}

void ZRescaleSwcDialog::translateSomaCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    ui->TranslateXDoubleSpinBox->setEnabled(true);
    ui->TranslateYDoubleSpinBox->setEnabled(true);
    ui->TranslateZDoubleSpinBox->setEnabled(true);
  } else {
    ui->TranslateXDoubleSpinBox->setEnabled(false);
    ui->TranslateYDoubleSpinBox->setEnabled(false);
    ui->TranslateZDoubleSpinBox->setEnabled(false);
  }
}

void ZRescaleSwcDialog::setScaleMethod()
{
  if (ui->ScaleUseResolutionRadioButton->isChecked()) {
    ui->CurrentSwcResolutionXYDoubleSpinBox->setEnabled(true);
    ui->CurrentSwcResolutionZDoubleSpinBox->setEnabled(true);
    ui->TargetResolutionXYDoubleSpinBox->setEnabled(true);
    ui->TargetResolutionZDoubleSpinBox->setEnabled(true);
    ui->ScaleXDoubleSpinBox->setEnabled(false);
    ui->ScaleYDoubleSpinBox->setEnabled(false);
    ui->SacleZDoubleSpinBox->setEnabled(false);
  } else if (ui->ScaleManuallyRadioButton->isChecked()) {
    ui->ScaleXDoubleSpinBox->setEnabled(true);
    ui->ScaleYDoubleSpinBox->setEnabled(true);
    ui->SacleZDoubleSpinBox->setEnabled(true);
    ui->CurrentSwcResolutionXYDoubleSpinBox->setEnabled(false);
    ui->CurrentSwcResolutionZDoubleSpinBox->setEnabled(false);
    ui->TargetResolutionXYDoubleSpinBox->setEnabled(false);
    ui->TargetResolutionZDoubleSpinBox->setEnabled(false);
  }
}

void ZRescaleSwcDialog::rescaleBranchRadiusCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    ui->RescaleBranchRadiusDoubleSpinBox->setEnabled(true);
    ui->SomaCutLevelSpinBox->setEnabled(true);
  } else {
    ui->RescaleBranchRadiusDoubleSpinBox->setEnabled(false);
    if (!ui->RescaleSomaRadiusCheckBox->isChecked()) {
      ui->SomaCutLevelSpinBox->setEnabled(false);
    }
  }
}

void ZRescaleSwcDialog::rescaleSomaRadiusCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    ui->RescaleSomaRadiusDoubleSpinBox->setEnabled(true);
    ui->SomaCutLevelSpinBox->setEnabled(true);
  } else {
    ui->RescaleSomaRadiusDoubleSpinBox->setEnabled(false);
    if (!ui->RescaleBranchRadiusCheckBox->isChecked()) {
      ui->SomaCutLevelSpinBox->setEnabled(false);
    }
  }
}

void ZRescaleSwcDialog::reduceNSwcNodesCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    ui->ReduceSwcNodesDoubleSpinBox->setEnabled(true);
  } else {
    ui->ReduceSwcNodesDoubleSpinBox->setEnabled(false);
  }
}

