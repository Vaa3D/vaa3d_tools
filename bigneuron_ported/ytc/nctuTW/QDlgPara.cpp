#include "QDlgPara.h"
#include "ui_QDlgPara.h"
#include "neurontracing.h"

QDlgPara::QDlgPara(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QDlgPara)
{
    ui->setupUi(this);
    ui->dudThreshold->setValue(g_rThreshold);
    ui->nudSomaX->setValue(g_nSomaX);
    ui->nudSomaY->setValue(g_nSomaY);
    ui->nudSomaZ->setValue(g_nSomaZ);
}

QDlgPara::~QDlgPara()
{
    delete ui;
}

void QDlgPara::on_buttonBox_accepted()
{
    g_rThreshold = ui->dudThreshold->value();
    g_nSomaX = ui->nudSomaX->value();
    g_nSomaY = ui->nudSomaY->value();
    g_nSomaZ = ui->nudSomaZ->value();
}

void QDlgPara::on_buttonBox_rejected()
{

}
