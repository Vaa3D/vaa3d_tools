#include "projectiondialog.h"
#include "ui_projectiondialog.h"

ProjectionDialog::ProjectionDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ProjectionDialog)
{
  ui->setupUi(this);
}

ProjectionDialog::~ProjectionDialog()
{
  delete ui;
}

int ProjectionDialog::speedLevel() const
{
  return ui->speedComboBox->currentIndex();
}

bool ProjectionDialog::adjustingContrast() const
{
  return ui->contrastCheckBox->isChecked();
}

bool ProjectionDialog::smoothingDepth() const
{
  return ui->depthCheckBox->isChecked();
}

bool ProjectionDialog::usingExisted() const
{
  return ui->usingExistedCheckBox->isChecked();
}
