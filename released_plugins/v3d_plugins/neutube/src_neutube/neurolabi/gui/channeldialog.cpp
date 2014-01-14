#include "channeldialog.h"
#include "ui_channeldialog.h"

ChannelDialog::ChannelDialog(QWidget *parent, int nchannel) :
    QDialog(parent),
    ui(new Ui::ChannelDialog)
{
  ui->setupUi(this);
  ui->comboBox->clear();
  for (int i=0; i<nchannel; i++ ) {
    ui->comboBox->addItem(QString("Ch%1").arg(i+1));
  }
  ui->comboBox->setCurrentIndex(0);
}

ChannelDialog::~ChannelDialog()
{
  delete ui;
}

int ChannelDialog::channel()
{
  return ui->comboBox->currentIndex();
}
