#ifndef PROJECTIONDIALOG_H
#define PROJECTIONDIALOG_H

#include <QDialog>

namespace Ui {
class ProjectionDialog;
}

class ProjectionDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit ProjectionDialog(QWidget *parent = 0);
  ~ProjectionDialog();
  
  int speedLevel() const;
  bool adjustingContrast() const;
  bool smoothingDepth() const;
  bool usingExisted() const;

private:
  Ui::ProjectionDialog *ui;
};

#endif // PROJECTIONDIALOG_H
