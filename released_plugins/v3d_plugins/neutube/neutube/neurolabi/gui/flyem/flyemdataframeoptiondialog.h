#ifndef FLYEMDATAFRAMEOPTIONDIALOG_H
#define FLYEMDATAFRAMEOPTIONDIALOG_H

#include <QDialog>

namespace Ui {
class FlyEmDataFrameOptionDialog;
}

class FlyEmDataFrameOptionDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit FlyEmDataFrameOptionDialog(QWidget *parent = 0);
  ~FlyEmDataFrameOptionDialog();
  
public:
  void setResampleStep(double step);
  void setTrunkStep(double step);
  void setLayerScale(double scale);
  void setLayerMargin(double margin);
  double getResampleStep();
  double getTrunkStep();
  double getLayerScale();
  double getLayerMargin();

private:
  Ui::FlyEmDataFrameOptionDialog *ui;
};

#endif // FLYEMDATAFRAMEOPTIONDIALOG_H
