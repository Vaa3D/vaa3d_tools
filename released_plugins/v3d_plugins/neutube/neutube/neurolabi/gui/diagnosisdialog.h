#ifndef DIAGNOSISDIALOG_H
#define DIAGNOSISDIALOG_H

#include <QDialog>

namespace Ui {
class DiagnosisDialog;
}

class DiagnosisDialog : public QDialog
{
  Q_OBJECT

public:
  explicit DiagnosisDialog(QWidget *parent = 0);
  ~DiagnosisDialog();

  void scrollToBottom();

private:
  void loadErrorFile();
  void loadWarnFile();
  void loadInfoFile();

private:
  Ui::DiagnosisDialog *ui;
};

#endif // DIAGNOSISDIALOG_H
