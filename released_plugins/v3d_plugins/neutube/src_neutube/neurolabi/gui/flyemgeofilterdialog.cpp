#include "flyemgeofilterdialog.h"
#include "ui_flyemgeofilterdialog.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QCheckBox>
#include <QFileInfo>

#include "flyem/zflyemneuron.h"
#include "flyem/zflyemneuronfilter.h"

FlyEmGeoFilterDialog::FlyEmGeoFilterDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmGeoFilterDialog)
{
  ui->setupUi(this);
  loadFilter();
  connect(ui->filterComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(loadFilter()));
}

FlyEmGeoFilterDialog::~FlyEmGeoFilterDialog()
{
  delete ui;
}

void FlyEmGeoFilterDialog::loadFilter()
{
  QString filterPath;

  switch (ui->filterComboBox->currentIndex()) {
  case 0:
    filterPath = ":/Resources/config/layer_filter.json";
    break;
  case 1:
    filterPath = ":/Resources/config/distance_filter.json";
    break;
  case 2:
    filterPath = ":/Resources/config/angle_filter.json";
    break;
  case 3:
    filterPath = ":/Resources/config/composite_filter.json";
    break;
  }

  if (!filterPath.isEmpty()) {
    QFile file(filterPath);
    file.open(QIODevice::ReadOnly);

    QTextStream stream(&file);
    QString str;
    str.append(stream.readAll());
    ui->filterTextEdit->setText(str);
  }
}

ZFlyEmNeuronFilter* FlyEmGeoFilterDialog::getFilter()
{
  QString text = ui->filterTextEdit->toPlainText();
  ZJsonValue jsonValue;
  jsonValue.decodeString(text.toStdString().c_str());

  if (jsonValue.isEmpty()) {
    QMessageBox::warning(
          this, "Parsing Error", jsonValue.getErrorString().c_str());
  } else {
    ZJsonObject config(jsonValue.getData(), false);

    if (!config.isEmpty()) {
      ZFlyEmNeuronFilter *filter = m_filterFactory.createFilter(config);
      return filter;
    } else {
      QMessageBox::warning(this, "Parsing Error", "Invalid filter configuration");
    }
  }

  return NULL;
}

void FlyEmGeoFilterDialog::setDataBundleWidget(
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

bool FlyEmGeoFilterDialog::isBundleSelected(int index) const
{
  if (index >= 0 && index < m_bundleCheckBox.size()) {
    return m_bundleCheckBox[index]->isChecked();
  }

  return false;
}
