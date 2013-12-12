#include <QFileDialog>
#include "traceoutputdialog.h"
#include "ui_traceoutputdialog.h"

TraceOutputDialog::TraceOutputDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::TraceOutputDialog)
{
  m_ui->setupUi(this);
}

TraceOutputDialog::~TraceOutputDialog()
{
  delete m_ui;
}

void TraceOutputDialog::changeEvent(QEvent *e)
{
  QDialog::changeEvent(e);
  switch (e->type()) {
  case QEvent::LanguageChange:
    m_ui->retranslateUi(this);
    break;
  default:
    break;
  }
}

QString TraceOutputDialog::dir()
{
  return m_ui->dirLineEdit->text();
}

QString TraceOutputDialog::prefix()
{
  return m_ui->prefixLineEdit->text();
}

void TraceOutputDialog::on_dirPushButton_clicked()
{
  QString dirpath = QFileDialog::getExistingDirectory(this, tr("Tube Directory"),
    ".", QFileDialog::ShowDirsOnly);

  if (!dirpath.isEmpty()) {
    m_ui->dirLineEdit->setText(dirpath);
  }
}
