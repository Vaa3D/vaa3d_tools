#include "changeactiveoverlaydialog.h"
#include "ui_changeactiveoverlaydialog.h"

ChangeActiveOverlayDialog::ChangeActiveOverlayDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeActiveOverlayDialog)
{
    ui->setupUi(this);
}

ChangeActiveOverlayDialog::~ChangeActiveOverlayDialog()
{
    delete ui;
}

int ChangeActiveOverlayDialog::getActiveOverlay()
{
    return ui->sbActiveOverlay->value();
}
