#ifndef SWCSKELETONTRANSFORMDIALOG_H
#define SWCSKELETONTRANSFORMDIALOG_H

#include <QDialog>

namespace Ui {
class SwcSkeletonTransformDialog;
}

class SwcSkeletonTransformDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit SwcSkeletonTransformDialog(QWidget *parent = 0);
  ~SwcSkeletonTransformDialog();
  
  enum EDimension{
    X, Y, Z
  };

  double getTranslateValue(EDimension dim);
  double getScaleValue(EDimension dim);
  bool isTranslateFirst();

  void setTranslateValue(double dx, double dy, double dz);
  void setScaleValue(double sx, double sy, double sz);

private:
  Ui::SwcSkeletonTransformDialog *ui;
};

#endif // SWCSKELETONTRANSFORMDIALOG_H
