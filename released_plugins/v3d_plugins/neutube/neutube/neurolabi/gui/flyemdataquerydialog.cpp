#include "flyemdataquerydialog.h"
#include <QComboBox>
#include "ui_flyemdataquerydialog.h"


FlyEmDataQueryDialog::FlyEmDataQueryDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmDataQueryDialog)
{
  ui->setupUi(this);
  connect(ui->sourceComboBox, SIGNAL(currentIndexChanged(const QString&)),
          this, SLOT(updateWidget(const QString&)));
  updateWidget(ui->sourceComboBox->currentText());
}

FlyEmDataQueryDialog::~FlyEmDataQueryDialog()
{
  delete ui;
}

QString FlyEmDataQueryDialog::getQueryString() const
{
  return ui->sourceComboBox->currentText() + " " +
      ui->targetComboBox->currentText() + " " + ui->sourceLineEdit->text();
}

QString FlyEmDataQueryDialog::getSourceType() const
{
  return ui->sourceComboBox->currentText();
}

QString FlyEmDataQueryDialog::getSourceValue() const
{
  return ui->sourceLineEdit->text();
}

QString FlyEmDataQueryDialog::getTarget() const
{
  return ui->targetComboBox->currentText();
}

bool FlyEmDataQueryDialog::usingRegularExpression() const
{
  return ui->regexpCheckBox->isChecked();
}

void FlyEmDataQueryDialog::updateWidget(const QString &text)
{
  if (text.toLower() != "class") {
    ui->regexpCheckBox->setVisible(false);
  } else {
    ui->regexpCheckBox->setVisible(true);
  }

  if (text.toLower() == "all") {
    ui->sourceLineEdit->setEnabled(false);
  } else {
    ui->sourceLineEdit->setEnabled(true);
  }

}
