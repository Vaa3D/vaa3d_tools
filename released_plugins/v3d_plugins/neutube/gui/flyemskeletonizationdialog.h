#ifndef FLYEMSKELETONIZATIONDIALOG_H
#define FLYEMSKELETONIZATIONDIALOG_H

#include <QDialog>

namespace Ui {
class FlyEmSkeletonizationDialog;
}

class FlyEmSkeletonizationDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit FlyEmSkeletonizationDialog(QWidget *parent = 0);
  ~FlyEmSkeletonizationDialog();
  
  int lengthThreshold();
  int distanceThreshold();
  int sizeThreshold();
  bool isConnectingAll();
  bool isKeepingShortObject();
  bool isExcludingSmallObj();
  bool isLevelChecked();
  int level();

public slots:
  void enableDistanceThreshold();
  void enableSizeThreshold();

private:
  Ui::FlyEmSkeletonizationDialog *ui;
};

#endif // FLYEMSKELETONIZATIONDIALOG_H
