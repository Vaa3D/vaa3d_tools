/**@file cannyedgedialog.h
 * @brief Canny edge parameter dialog
 * @author Ting Zhao
 */
#ifndef CANNYEDGEDIALOG_H
#define CANNYEDGEDIALOG_H

#include <QDialog>

namespace Ui {
    class CannyEdgeDialog;
}

class CannyEdgeDialog : public QDialog {
    Q_OBJECT
public:
  CannyEdgeDialog(QWidget *parent = 0);
  ~CannyEdgeDialog();

public:
  double variance();
  double lowerThreshold();
  double upperThreshold();

protected:
  void changeEvent(QEvent *e);

private:
  Ui::CannyEdgeDialog *m_ui;
};

#endif // CANNYEDGEDIALOG_H
