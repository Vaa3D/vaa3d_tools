#ifndef FLYEMDATAPROCESSDIALOG_H
#define FLYEMDATAPROCESSDIALOG_H

#include <QDialog>

namespace Ui {
class FlyEmDataProcessDialog;
}

class FlyEmDataProcessDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlyEmDataProcessDialog(QWidget *parent = 0);
  ~FlyEmDataProcessDialog();

  QString getSourceType();
  QString getSourceString();
  QString getAction();

private:
  Ui::FlyEmDataProcessDialog *ui;
};

#endif // FLYEMDATAPROCESSDIALOG_H
