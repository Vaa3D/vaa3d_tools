/**@file connectedthresholddialog.h
 * @brief Connected thresholding parameter dialog
 * @author Ting Zhao
 */
#ifndef CONNECTEDTHRESHOLDDIALOG_H
#define CONNECTEDTHRESHOLDDIALOG_H

#include <QDialog>

namespace Ui {
class ConnectedThresholdDialog;
}

class ConnectedThresholdDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ConnectedThresholdDialog(QWidget *parent = 0);
  ~ConnectedThresholdDialog();

  int lowerThreshold();
  int upperThreshold();

protected:
  void changeEvent(QEvent *e);

private:
  Ui::ConnectedThresholdDialog *ui;
};

#endif // CONNECTEDTHRESHOLDDIALOG_H
