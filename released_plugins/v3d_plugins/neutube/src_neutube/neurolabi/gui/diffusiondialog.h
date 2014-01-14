/**@file diffusiondialog.h
 * @brief Anisotripic diffusion parameter dialog
 * @author Ting Zhao
 */
#ifndef DIFFUSIONDIALOG_H
#define DIFFUSIONDIALOG_H

#include <QDialog>

namespace Ui {
class DiffusionDialog;
}

class DiffusionDialog : public QDialog
{
  Q_OBJECT
public:
  explicit DiffusionDialog(QWidget *parent = 0);
  virtual ~DiffusionDialog();

public:
  int numberOfIteration();
  double timeStep();
  double conductance();
  int method();

signals:

public slots:
  void resetParameters(int method);

private:
  Ui::DiffusionDialog *m_ui;
};

#endif // DIFFUSIONDIALOG_H
