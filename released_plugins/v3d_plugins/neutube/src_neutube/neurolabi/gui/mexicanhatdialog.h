#ifndef MEXICANHATDIALOG_H
#define MEXICANHATDIALOG_H

#include <QDialog>

namespace Ui {
class MexicanHatDialog;
}

class MexicanHatDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit MexicanHatDialog(QWidget *parent = 0);
  ~MexicanHatDialog();
  
  double sigma();

private:
  Ui::MexicanHatDialog *ui;
};

#endif // MEXICANHATDIALOG_H
