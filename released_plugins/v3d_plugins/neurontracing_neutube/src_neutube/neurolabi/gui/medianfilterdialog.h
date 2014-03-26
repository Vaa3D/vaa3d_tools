/**@file medianfilterdialog.h
 * @brief Median filter parameter setup
 * @author Ting Zhao
 */
#ifndef MEDIANFILTERDIALOG_H
#define MEDIANFILTERDIALOG_H

#include <QDialog>

namespace Ui {
  class MedianFilterDialog;
}

class MedianFilterDialog : public QDialog {
  Q_OBJECT
public:
  MedianFilterDialog(QWidget *parent = 0);
  ~MedianFilterDialog();

public:
  int radius();

protected:
  void changeEvent(QEvent *e);

private:
  Ui::MedianFilterDialog *m_ui;
};

#endif // MEDIANFILTERDIALOG_H
