/**@file bcadjustdialog.h
 * @brief BC adjust dialog
 * @author Ting Zhao
 */
#ifndef BCADJUSTDIALOG_H
#define BCADJUSTDIALOG_H

#include <QDialog>

namespace Ui {
    class BcAdjustDialog;
}

/*
 BcAdjustDialog defines a dialog for setting parameters of
 brightness/contrast adjustment.
   Sample:
     ...
     BcAdjustDialog dlg;
     dlg.setValue(10, 100);
     dlg.setRange(0, 255);
     dlg.exec();
     ...
*/
class BcAdjustDialog : public QDialog {
  Q_OBJECT
public:
  BcAdjustDialog(QWidget *parent = 0);
  ~BcAdjustDialog();

public: // attribute access
  void setValue(int lower, int upper, int c=0);
  void setRange(int min, int max, int c=0);

  double greyScale(int c=0);
  double greyOffset(int c=0);

  void setNumOfChannel(int c=1);
  int getMaxNumOfChannel() const { return 4; }

signals:
  void valueChanged();  
  void autoAdjustTriggered();

protected:
  void changeEvent(QEvent *e);

protected slots:
  void setLowerValueLabel(int value);
  void setUpperValueLabel(int value);
  void setLowerValueLabel_2(int value);
  void setUpperValueLabel_2(int value);
  void setLowerValueLabel_3(int value);
  void setUpperValueLabel_3(int value);
  void setLowerValueLabel_4(int value);
  void setUpperValueLabel_4(int value);

private slots:
  void on_autoAdjustPushButton_clicked();

  void on_pushButton_clicked();

private:
  Ui::BcAdjustDialog *m_ui;
};

#endif // BCADJUSTDIALOG_H
