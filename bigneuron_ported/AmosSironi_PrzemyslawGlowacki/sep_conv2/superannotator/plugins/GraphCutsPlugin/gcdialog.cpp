#include "gcdialog.h"
#include "ui_gcdialog.h"

GCDialog::GCDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GCDialog)
{
    ui->setupUi(this);
}

GCDialog::~GCDialog()
{
    delete ui;
}

float GCDialog::getEdgeWeight()
{
    return ui->sbEdgeWeight_2->value();
}

float GCDialog::getVariance()
{
    return ui->sbVariance_2->value();
}
