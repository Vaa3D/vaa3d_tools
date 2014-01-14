#include "connectedthresholddialog.h"
#include "ui_connectedthresholddialog.h"

ConnectedThresholdDialog::ConnectedThresholdDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectedThresholdDialog)
{
    ui->setupUi(this);
}

ConnectedThresholdDialog::~ConnectedThresholdDialog()
{
    delete ui;
}

void ConnectedThresholdDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

int ConnectedThresholdDialog::lowerThreshold()
{
  return ui->lowerSpinBox->value();
}

int ConnectedThresholdDialog::upperThreshold()
{
  return ui->upperSpinBox->value();
}
