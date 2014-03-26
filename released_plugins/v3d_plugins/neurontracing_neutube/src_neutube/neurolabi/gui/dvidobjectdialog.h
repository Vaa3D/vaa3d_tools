#ifndef DVIDOBJECTDIALOG_H
#define DVIDOBJECTDIALOG_H

#include <QDialog>

namespace Ui {
class DvidObjectDialog;
}

class DvidObjectDialog : public QDialog
{
  Q_OBJECT

public:
  explicit DvidObjectDialog(QWidget *parent = 0);
  ~DvidObjectDialog();

  void setAddress(const QString &address);
  QString getAddress() const;
  int getBodyId() const;

private:
  Ui::DvidObjectDialog *ui;
};

#endif // DVIDOBJECTDIALOG_H
