#ifndef ZALPHADIALOG_H
#define ZALPHADIALOG_H

#include <QDialog>

namespace Ui {
class ZAlphaDialog;
}

class ZAlphaDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit ZAlphaDialog(QWidget *parent = 0);
  ~ZAlphaDialog();
  
  int getAlpha();

private:
  Ui::ZAlphaDialog *ui;
};

#endif // ZALPHADIALOG_H
