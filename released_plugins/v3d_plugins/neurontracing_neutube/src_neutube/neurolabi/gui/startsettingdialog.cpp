#include "startsettingdialog.h"
#include <QFileDialog>
#include "ui_startsettingdialog.h"

StartSettingDialog::StartSettingDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::StartSettingDialog)
{
  ui->setupUi(this);
}

StartSettingDialog::~StartSettingDialog()
{
  delete ui;
}

QString StartSettingDialog::getAutosaveDir()
{
  return ui->autoSaveLineEdit->text();
}

void StartSettingDialog::setAutoSaveDir(const QString &dir)
{
  ui->autoSaveLineEdit->setText(dir);
}

void StartSettingDialog::on_pushButton_clicked()
{
  QString fileName = QFileDialog::getExistingDirectory(this, tr("Autosave"),
        ".", QFileDialog::ShowDirsOnly/* | QFileDialog::DontUseNativeDialog*/);
  if (!fileName.isEmpty()) {
    ui->autoSaveLineEdit->setText(fileName);
  }
}
