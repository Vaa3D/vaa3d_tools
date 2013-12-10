#ifndef FLYEMGEOFILTERDIALOG_H
#define FLYEMGEOFILTERDIALOG_H

#include <QDialog>
#include <QVector>
#include "flyem/zflyemdatabundle.h"

#include "flyem/zflyemneuronfilterfactory.h"


class QCheckBox;

class ZFlyEmNeuron;

namespace Ui {
class FlyEmGeoFilterDialog;
}

class FlyEmGeoFilterDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlyEmGeoFilterDialog(QWidget *parent = 0);
  ~FlyEmGeoFilterDialog();

  ZFlyEmNeuronFilter* getFilter();

  void setDataBundleWidget(const QVector<ZFlyEmDataBundle*> &bundleArray);

  bool isBundleSelected(int index) const;

private slots:
  void loadFilter();

private:
  Ui::FlyEmGeoFilterDialog *ui;

  ZFlyEmNeuronFilterFactory m_filterFactory;
  QVector<QCheckBox*> m_bundleCheckBox;
};

#endif // FLYEMGEOFILTERDIALOG_H
