#include "informationdialog.h"
#include "ui_informationdialog.h"

InformationDialog::InformationDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::InformationDialog)
{
  ui->setupUi(this);
}

InformationDialog::~InformationDialog()
{
  delete ui;
}

void InformationDialog::setText(const std::string &text)
{
  ui->textEdit->setText(text.c_str());
}
