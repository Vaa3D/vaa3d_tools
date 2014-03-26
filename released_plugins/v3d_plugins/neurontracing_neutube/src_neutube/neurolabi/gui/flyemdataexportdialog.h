#ifndef FLYEMDATAEXPORTDIALOG_H
#define FLYEMDATAEXPORTDIALOG_H

#include <QDialog>

namespace Ui {
class FlyEmDataExportDialog;
}

class FlyEmDataExportDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlyEmDataExportDialog(QWidget *parent = 0);
  ~FlyEmDataExportDialog();

private:
  Ui::FlyEmDataExportDialog *ui;
};

#endif // FLYEMDATAEXPORTDIALOG_H
