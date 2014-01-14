#ifndef FLYEMDATAQUERYDIALOG_H
#define FLYEMDATAQUERYDIALOG_H

#include <QDialog>

namespace Ui {
class FlyEmDataQueryDialog;
}

class FlyEmDataQueryDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlyEmDataQueryDialog(QWidget *parent = 0);
  ~FlyEmDataQueryDialog();

  QString getQueryString() const;
  QString getSourceType() const;
  QString getSourceValue() const;
  QString getTarget() const;

  bool usingRegularExpression() const;

public slots:
  void updateWidget(const QString &text);

private:
  Ui::FlyEmDataQueryDialog *ui;
};

#endif // FLYEMDATAQUERYDIALOG_H
