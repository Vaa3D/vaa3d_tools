#ifndef NEURONIDDIALOG_H
#define NEURONIDDIALOG_H

#include <QDialog>

namespace Ui {
class NeuronIdDialog;
}

class NeuronIdDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit NeuronIdDialog(QWidget *parent = 0);
  ~NeuronIdDialog();
  
  int getNeuronId();
  int getDownsampleRate();

private:
  Ui::NeuronIdDialog *ui;
};

#endif // NEURONIDDIALOG_H
