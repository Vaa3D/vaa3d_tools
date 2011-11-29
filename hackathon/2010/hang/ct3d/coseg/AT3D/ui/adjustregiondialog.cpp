#include "adjustregiondialog.h"
#include "ui_adjustregiondialog.h"
#include <QMessageBox>
AdjustRegionDialog::AdjustRegionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdjustRegionDialog)
{
    ui->setupUi(this);
}

AdjustRegionDialog::~AdjustRegionDialog()
{
    delete ui;
}

void AdjustRegionDialog::changeEvent(QEvent *e)
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
