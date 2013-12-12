#ifndef DISTANCEMAPDIALOG_H
#define DISTANCEMAPDIALOG_H

#include <QDialog>

namespace Ui {
class DistanceMapDialog;
}

class DistanceMapDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit DistanceMapDialog(QWidget *parent = 0);
  ~DistanceMapDialog();
  
public:
  bool isSliceWise();
  bool isForegroundTransform();
  bool isSquared();

private:
  Ui::DistanceMapDialog *ui;
};

#endif // DISTANCEMAPDIALOG_H
