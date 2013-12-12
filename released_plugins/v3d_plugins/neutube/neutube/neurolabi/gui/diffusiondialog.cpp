#include "diffusiondialog.h"
#include "ui_diffusiondialog.h"

DiffusionDialog::DiffusionDialog(QWidget *parent) :
    QDialog(parent), m_ui(new Ui::DiffusionDialog)
{
  m_ui->setupUi(this);
  connect(m_ui->methodComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(resetParameters(int)));
}

DiffusionDialog::~DiffusionDialog()
{
  delete m_ui;
}

int DiffusionDialog::numberOfIteration()
{
  return m_ui->niterSpinBox->value();
}

double DiffusionDialog::timeStep()
{
  return m_ui->timeStepSpinBox->value();
}

double DiffusionDialog::conductance()
{
  return m_ui->conductanceSpinBox->value();
}

int DiffusionDialog::method()
{
  return m_ui->methodComboBox->currentIndex();
}

void DiffusionDialog::resetParameters(int method)
{
  switch (method) {
  case 0:
    m_ui->niterSpinBox->setValue(5);
    m_ui->timeStepSpinBox->setValue(0.125);
    m_ui->conductanceSpinBox->setValue(20.0);
    break;
  case 1:
    m_ui->niterSpinBox->setValue(10);
    m_ui->timeStepSpinBox->setValue(0.0625);
    m_ui->conductanceSpinBox->setValue(10.0);
    break;
  case 2:
    m_ui->niterSpinBox->setValue(10);
    m_ui->timeStepSpinBox->setValue(0.0625);
    m_ui->conductanceSpinBox->setValue(1.0);
    break;
  }
}
