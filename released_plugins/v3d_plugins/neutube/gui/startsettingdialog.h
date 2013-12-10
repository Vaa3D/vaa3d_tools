#ifndef STARTSETTINGDIALOG_H
#define STARTSETTINGDIALOG_H

#include <QDialog>

namespace Ui {
class StartSettingDialog;
}

class StartSettingDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit StartSettingDialog(QWidget *parent = 0);
  ~StartSettingDialog();

  QString getAutosaveDir();
  void setAutoSaveDir(const QString &dir);
  
private slots:
  void on_pushButton_clicked();

private:
  Ui::StartSettingDialog *ui;
};

#endif // STARTSETTINGDIALOG_H
