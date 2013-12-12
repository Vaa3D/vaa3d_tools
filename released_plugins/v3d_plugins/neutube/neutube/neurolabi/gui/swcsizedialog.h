#ifndef SWCSIZEDIALOG_H
#define SWCSIZEDIALOG_H

#include <QDialog>

namespace Ui {
class SwcSizeDialog;
}

class SwcSizeDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit SwcSizeDialog(QWidget *parent = 0);
  ~SwcSizeDialog();
  
  double getAddValue();
  double getMulValue();

private:
  Ui::SwcSizeDialog *ui;
};

#endif // SWCSIZEDIALOG_H
