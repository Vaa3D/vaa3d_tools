#ifndef MOVIEDIALOG_H
#define MOVIEDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class MovieDialog;
}

class MovieDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit MovieDialog(QWidget *parent = 0);
  ~MovieDialog();

public:
  int getFrameWidth();
  int getFrameHeight();
  void setFrameWidth(int width);
  void setFrameHeight(int height);
  int getFrameRate();
  QString getScriptPath();
  QString getOutputPath();
  void setScriptPath(const QString &path);
  void setOutputPath(const QString &path);
  
private slots:
  void on_scriptPushButton_clicked();

  void on_outputPushButton_clicked();

private:
  Ui::MovieDialog *ui;
};

#endif // MOVIEDIALOG_H
