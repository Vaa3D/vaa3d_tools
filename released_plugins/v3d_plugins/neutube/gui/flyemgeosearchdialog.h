#ifndef FLYEMGEOSEARCHDIALOG_H
#define FLYEMGEOSEARCHDIALOG_H

#include <QDialog>
#include <QVector>
#include "flyem/zflyemdatabundle.h"

class QCheckBox;

namespace Ui {
class FlyEmGeoSearchDialog;
}

class FlyEmGeoSearchDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlyEmGeoSearchDialog(QWidget *parent = 0);
  ~FlyEmGeoSearchDialog();

  void init();

  void setDataBundleWidget(const QVector<ZFlyEmDataBundle*> &bundleArray);

  bool isBundleSelected(int index) const;

  double getMaxLength() const;
  double getMinLength() const;
  double getMinDist() const;

  int getMinLayer() const;
  int getMaxLayer() const;

  bool isLayerExclusive() const;

private slots:
  void updateMinLayer(int upperBound);
  void updateMaxLayer(int lowerBound);

private:
  Ui::FlyEmGeoSearchDialog *ui;
  QVector<QCheckBox*> m_bundleCheckBox;
};

#endif // FLYEMGEOSEARCHDIALOG_H
