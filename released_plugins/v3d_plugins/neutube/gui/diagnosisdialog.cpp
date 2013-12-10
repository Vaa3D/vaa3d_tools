#include "diagnosisdialog.h"
#include "ui_diagnosisdialog.h"
#include <QTextBrowser>
#include <QFile>
#include <QTextStream>
#include <QScrollBar>
#include <iostream>

#include "neutube.h"

DiagnosisDialog::DiagnosisDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DiagnosisDialog)
{
  ui->setupUi(this);
  loadErrorFile();
  loadWarnFile();
  loadInfoFile();
}

DiagnosisDialog::~DiagnosisDialog()
{
  delete ui;
}

void DiagnosisDialog::loadErrorFile()
{
  QFile file(NeuTube::getErrorFile().c_str());
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    ui->errorTextBrowser->setPlainText(QTextStream(&file).readAll());
    file.close();
  }
}

void DiagnosisDialog::loadWarnFile()
{
  QFile file(NeuTube::getWarnFile().c_str());
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    ui->warnTextBrowser->setPlainText(QTextStream(&file).readAll());
    file.close();
  }
}

void DiagnosisDialog::loadInfoFile()
{
  QFile file(NeuTube::getInfoFile().c_str());
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    ui->infoTextBrowser->setPlainText(QTextStream(&file).readAll());
    file.close();
  }
}

void DiagnosisDialog::scrollToBottom()
{
  ui->errorTextBrowser->verticalScrollBar()->setValue(
        ui->errorTextBrowser->verticalScrollBar()->maximum());
  ui->warnTextBrowser->verticalScrollBar()->setValue(
        ui->warnTextBrowser->verticalScrollBar()->maximum());
  ui->infoTextBrowser->verticalScrollBar()->setValue(
        ui->infoTextBrowser->verticalScrollBar()->maximum());
}
