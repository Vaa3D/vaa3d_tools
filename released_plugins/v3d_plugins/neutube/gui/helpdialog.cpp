#include "helpdialog.h"
#include "ui_helpdialog.h"
#include "neutubeconfig.h"

HelpDialog::HelpDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::HelpDialog)
{
  ui->setupUi(this);

  ui->textBrowser->setSource(
        QUrl(NeutubeConfig::getInstance().getHelpFilePath().c_str()));
}

HelpDialog::~HelpDialog()
{
  delete ui;
}
