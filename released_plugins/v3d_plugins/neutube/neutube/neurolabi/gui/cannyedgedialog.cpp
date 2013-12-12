#include "cannyedgedialog.h"
#include "ui_cannyedgedialog.h"

CannyEdgeDialog::CannyEdgeDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::CannyEdgeDialog)
{
    m_ui->setupUi(this);
}

CannyEdgeDialog::~CannyEdgeDialog()
{
    delete m_ui;
}

void CannyEdgeDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

double CannyEdgeDialog::variance()
{
  return m_ui->varianceSpinBox->value();
}

double CannyEdgeDialog::lowerThreshold()
{
  return m_ui->lowThreSpinBox->value();
}

double CannyEdgeDialog::upperThreshold()
{
  return m_ui->highThreSpinBox->value();
}
