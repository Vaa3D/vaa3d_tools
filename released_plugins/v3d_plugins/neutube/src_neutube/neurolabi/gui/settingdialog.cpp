#include <QtGui>

#include "settingdialog.h"

SettingDialog::SettingDialog(QWidget *parent) : QDialog(parent)
{
  setupUi(this);

  m_resolution[0] = 1.0;
  m_resolution[1] = 1.0;
  m_resolution[2] = 1.0;

  m_scales[0] = 1.0;
  m_scales[1] = 1.0;
  m_scales[2] = 1.0;

  m_traceEffort = 0;
  m_traceMasked = false;
  m_traceMinScore = 0.35;
  m_receptor = 0;
  m_useCone = false;
  m_unit = 0;
  m_distThre = 20.0;
  m_crossoverTest = false;
  m_singleTree = false;
  m_removeOvershoot = true;
  m_rootOption = 0;
  m_reconstructEffort = 0;

  resetWidgetValue();
}

void SettingDialog::resetWidgetValue()
{
  XResSpinBox->setValue(m_resolution[0]);
  YResSpinBox->setValue(m_resolution[1]);
  ZResSpinBox->setValue(m_resolution[2]);

  TraceComboBox->setCurrentIndex(m_traceEffort);
  maskCheckBox->setChecked(m_traceMasked);
  receptorComboBox->setCurrentIndex(m_receptor);
  ScoreThreSpinBox->setValue(m_traceMinScore);
  coneReceptorCheckBox->setChecked(m_useCone);
  unitComboBox->setCurrentIndex(m_unit);

  XScaleSpinBox->setValue(m_scales[0]);
  ZScaleSpinBox->setValue(m_scales[2]);
  distSpinBox->setValue(m_distThre);
  crossoverCheckBox->setChecked(m_crossoverTest);
  treeCheckBox->setChecked(m_singleTree);
  overshootCheckBox->setChecked(m_removeOvershoot);

  switch (m_rootOption) {
        case 0:
    autoRadioButton->setChecked(true);
    break;
        case 1:
    selectedRadioButton->setChecked(true);
    break;
        default:
    break;
  }

  spComboBox->setCurrentIndex(m_reconstructEffort);

  updateOverview();
}

void SettingDialog::setResolution(const double *res, int unit)
{
  m_resolution[0] = res[0];
  m_resolution[1] = res[1];
  m_resolution[2] = res[2];
  XResSpinBox->setValue(m_resolution[0]);
  YResSpinBox->setValue(m_resolution[1]);
  ZResSpinBox->setValue(m_resolution[2]);

  m_unit = unit;
  unitComboBox->setCurrentIndex(unit);
  updateOverview();
}

void SettingDialog::update()
{
  m_resolution[0] = XResSpinBox->value();
  m_resolution[1] = YResSpinBox->value();
  m_resolution[2] = ZResSpinBox->value();

  m_scales[0] = XScaleSpinBox->value();
  m_scales[1] = XScaleSpinBox->value();
  m_scales[2] = ZScaleSpinBox->value();

  m_traceEffort = TraceComboBox->currentIndex();
  m_traceMasked = maskCheckBox->isChecked();
  m_receptor = receptorComboBox->currentIndex();
  m_traceMinScore = ScoreThreSpinBox->value();
  m_useCone = coneReceptorCheckBox->isChecked();
  m_unit = unitComboBox->currentIndex();

  m_distThre = distSpinBox->value();
  m_crossoverTest = crossoverCheckBox->isChecked();
  m_singleTree = treeCheckBox->isChecked();
  m_reconstructEffort = spComboBox->currentIndex();
  m_removeOvershoot = overshootCheckBox->isChecked();

  if (autoRadioButton->isChecked()) {
    m_rootOption = 0;
  }
  if (selectedRadioButton->isChecked()) {
    m_rootOption = 1;
  }

  updateOverview();
}

void SettingDialog::updateOverview()
{
  QString overview = QString("Voxel size: %1 x %2 x %3 %4\n").
                     arg(m_resolution[0]).arg(m_resolution[1]).
                     arg(m_resolution[2]).
                     arg(m_unit==0 ? "pixel" : "um");

  overview += QString("Tracing settings: \n"
                      "    %1\n"
                      "    masked: %2\n").
      arg(TraceComboBox->itemText(TraceComboBox->currentIndex())).
      arg(m_traceMasked ? "yes" : "no");

  overview += QString("Reconstruction settings: \n"
                      "    Scales: x-y x%1; z x%2\n"
                      "    Neighbor range: %3\n"
                      "    Detect crossover: %4\n"
                      "    Remove overshoot: %5\n").
      arg(m_scales[0]).arg(m_scales[1]).arg(m_distThre).
      arg(m_crossoverTest ? "yes" : "no").arg(m_crossoverTest ? "yes" : "no");

  settingTextEdit->setPlainText(overview);
}

void SettingDialog::reject()
{
  resetWidgetValue();

  QDialog::reject();
}

void SettingDialog::accept()
{
  update();

  QDialog::accept();
}

double SettingDialog::xResolution()
{
  return m_resolution[0];
}

double SettingDialog::yResolution()
{
  return m_resolution[1];
}

double SettingDialog::zResolution()
{
  return m_resolution[2];
}

double SettingDialog::xScale()
{
  return m_scales[0];
}

double SettingDialog::zScale()
{
  return m_scales[2];
}

int SettingDialog::traceEffort()
{
  return m_traceEffort;
}

int SettingDialog::receptor()
{
  return m_receptor;
}

double SettingDialog::traceMinScore()
{
  return m_traceMinScore;
}

bool SettingDialog::useCone()
{
  return m_useCone;
}

char SettingDialog::unit()
{
  if (m_unit == 0) {
    return 'p';
  } else {
    return 'u';
  }
}

void SettingDialog::setUnit(char unit)
{
  if (unit == 'p') {
    m_unit = 0;
  } else {
    m_unit = 1;
  }
}
