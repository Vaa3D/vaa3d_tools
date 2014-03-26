#ifndef PENWIDTHDIALOG_H
#define PENWIDTHDIALOG_H

#include <QDialog>

namespace Ui {
class PenWidthDialog;
}

class PenWidthDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit PenWidthDialog(QWidget *parent = 0);
  ~PenWidthDialog();

public:
  void setPenWidth(double width);
  double getPenWidth();
  
private:
  Ui::PenWidthDialog *ui;
};

#endif // PENWIDTHDIALOG_H
