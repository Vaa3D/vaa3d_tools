#include <QtGui>

#include "frameinfodialog.h"
#include "zcurve.h"
#include "plotter.h"
#include "neutubeconfig.h"

FrameInfoDialog::FrameInfoDialog(QWidget *parent) : QDialog(parent)
{
  setupUi(this);

  m_curveWidget = new Plotter(curveFrame);
  m_curveWidget->setPlotSettings(m_plotSettings);
  curveComboBox->addItem(tr("correlation"));
  curveComboBox->addItem(tr("corrcoef"));
  curveComboBox->addItem(tr("inner signal"));
  curveComboBox->addItem(tr("outer signal"));
  curveComboBox->setCurrentIndex(1);
  connect(curveComboBox, SIGNAL(currentIndexChanged(int)),
          this, SIGNAL(newCurveSelected(int)));

  if (NeutubeConfig::getInstance().getApplication() == "Biocytin") {
    m_curveWidget->setVisible(false);
    curveComboBox->setVisible(false);
    curveTypeLabel->setVisible(false);
  }
}

FrameInfoDialog::~FrameInfoDialog()
{
  delete m_curveWidget;
}

void FrameInfoDialog::setText(const QString &info)
{
  infoBrowser->setText(info);
}

int FrameInfoDialog::curveOption()
{
  return curveComboBox->currentIndex();
}

void FrameInfoDialog::setCurve(const ZCurve &curve)
{
  m_curveWidget->setCurveData(0, curve.data());
}

void FrameInfoDialog::updatePlotSettings()
{
  m_curveWidget->setPlotSettings(m_plotSettings);
}
