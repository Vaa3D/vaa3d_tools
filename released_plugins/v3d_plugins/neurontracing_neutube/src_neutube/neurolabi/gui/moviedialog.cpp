#include "moviedialog.h"

#include "ui_moviedialog.h"
#include <QFileDialog>

MovieDialog::MovieDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::MovieDialog)
{
  ui->setupUi(this);
}

MovieDialog::~MovieDialog()
{
  delete ui;
}

int MovieDialog::getFrameWidth()
{
  return ui->widthSpinBox->value();
}

int MovieDialog::getFrameHeight()
{
  return ui->heightSpinBox->value();
}

void MovieDialog::setFrameWidth(int width)
{
  ui->widthSpinBox->setValue(width);
}

void MovieDialog::setFrameHeight(int height)
{
  ui->heightSpinBox->setValue(height);
}

int MovieDialog::getFrameRate()
{
  switch (ui->fpsComboBox->currentIndex()) {
  case 0:
    return 1;
  case 1:
    return 5;
  case 2:
    return 10;
  case 3:
    return 20;
  default:
    return 25;
  }
}

QString MovieDialog::getScriptPath()
{
  return ui->scriptLineEdit->text();
}

QString MovieDialog::getOutputPath()
{
  return ui->outputLineEdit->text();
}

void MovieDialog::setScriptPath(const QString &path)
{
  ui->scriptLineEdit->setText(path);
}

void MovieDialog::setOutputPath(const QString &path)
{
  ui->outputLineEdit->setText(path);
}

void MovieDialog::on_scriptPushButton_clicked()
{
  QString fileName =
      QFileDialog::getOpenFileName(this, tr("Load Movie Script"),
                                   getScriptPath(),
                                   tr("Movie files (*.json)"),
                                   NULL/*, QFileDialog::DontUseNativeDialog*/);
  if (!fileName.isEmpty()) {
    setScriptPath(fileName);
  }
}

void MovieDialog::on_outputPushButton_clicked()
{
  QString saveFileDir =
      QFileDialog::getExistingDirectory(this, tr("Movie Output"),
                                        getOutputPath(),
                                        QFileDialog::ShowDirsOnly/* |
                                        QFileDialog::DontUseNativeDialog*/);

  if (!saveFileDir.isEmpty()) {
    setOutputPath(saveFileDir);
  }
}
