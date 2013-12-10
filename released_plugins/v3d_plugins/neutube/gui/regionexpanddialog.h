#ifndef REGIONEXPANDDIALOG_H
#define REGIONEXPANDDIALOG_H

#include <QDialog>

namespace Ui {
class RegionExpandDialog;
}

class RegionExpandDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit RegionExpandDialog(QWidget *parent = 0);
  ~RegionExpandDialog();
  
  int getRadius();

private:
  Ui::RegionExpandDialog *ui;
};

#endif // REGIONEXPANDDIALOG_H
