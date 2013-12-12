#include "flyemgeosearchdialog.h"
#include <QCheckBox>
#include <QString>
#include <QFileInfo>
#include "ui_flyemgeosearchdialog.h"

FlyEmGeoSearchDialog::FlyEmGeoSearchDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmGeoSearchDialog)
{
  ui->setupUi(this);
  init();

  connect(ui->minLayerSpinBox, SIGNAL(valueChanged(int)),
          this, SLOT(updateMaxLayer(int)));
  connect(ui->maxLayerSpinBox, SIGNAL(valueChanged(int)),
          this, SLOT(updateMinLayer(int)));
}

FlyEmGeoSearchDialog::~FlyEmGeoSearchDialog()
{
  delete ui;
}

void FlyEmGeoSearchDialog::init()
{
  ui->maxLengthCheckBox->setChecked(false);
  ui->maxLengthDoubleSpinBox->setVisible(false);
  ui->minLengthCheckBox->setChecked(false);
  ui->minLengthDoubleSpinBox->setVisible(false);
  ui->minLayerSpinBox->setValue(1);
  ui->maxLayerSpinBox->setValue(10);
}

void FlyEmGeoSearchDialog::updateMinLayer(int upperBound)
{
  if (ui->minLayerSpinBox->value() > upperBound) {
    ui->minLayerSpinBox->setValue(upperBound);
  }
}

void FlyEmGeoSearchDialog::updateMaxLayer(int lowerBound)
{
  if (ui->maxLayerSpinBox->value() < lowerBound) {
    ui->maxLayerSpinBox->setValue(lowerBound);
  }
}

void FlyEmGeoSearchDialog::setDataBundleWidget(
    const QVector<ZFlyEmDataBundle*> &bundleArray)
{
  foreach (QCheckBox *box, m_bundleCheckBox) {
    delete box;
  }

  m_bundleCheckBox.clear();

  int index = 1;
  foreach (ZFlyEmDataBundle *bundle, bundleArray) {
    QString name = bundle->getSource().c_str();
    QFileInfo fileInfo(name);
    name = QString("Bundle %1: ").arg(index) + fileInfo.baseName() +
        QString(" %1 neurons").arg(bundle->getNeuronArray().size());
    QCheckBox *box = new QCheckBox(name);
    m_bundleCheckBox.append(box);
    box->setChecked(index == 1);
    ui->verticalLayout_2->addWidget(box);
    ++index;
  }
}

bool FlyEmGeoSearchDialog::isBundleSelected(int index) const
{
  if (index >= 0 && index < m_bundleCheckBox.size()) {
    return m_bundleCheckBox[index]->isChecked();
  }

  return false;
}

double FlyEmGeoSearchDialog::getMaxLength() const
{
  if (ui->maxLengthCheckBox->isChecked()) {
    return ui->maxLengthDoubleSpinBox->value();
  }

  return Infinity;
}

double FlyEmGeoSearchDialog::getMinLength() const
{
  if (ui->minLengthCheckBox->isChecked()) {
    return ui->minLengthDoubleSpinBox->value();
  }

  return 0.0;
}

double FlyEmGeoSearchDialog::getMinDist() const
{
  return ui->minDistDoubleSpinBox->value();
}

int FlyEmGeoSearchDialog::getMinLayer() const
{
  return ui->minLayerSpinBox->value();
}

int FlyEmGeoSearchDialog::getMaxLayer() const
{
  return ui->maxLayerSpinBox->value();
}

bool FlyEmGeoSearchDialog::isLayerExclusive() const
{
  return ui->exclusiveCheckBox->isChecked();
}
