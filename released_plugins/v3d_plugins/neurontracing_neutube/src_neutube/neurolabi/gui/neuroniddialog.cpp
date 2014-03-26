#include "neuroniddialog.h"
#include "ui_neuroniddialog.h"

NeuronIdDialog::NeuronIdDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::NeuronIdDialog)
{
  ui->setupUi(this);
}

NeuronIdDialog::~NeuronIdDialog()
{
  delete ui;
}

int NeuronIdDialog::getNeuronId()
{
  return ui->neuronIdSpinBox->value();
}

int NeuronIdDialog::getDownsampleRate()
{
  return ui->downsampleSpinBox->value();
}
