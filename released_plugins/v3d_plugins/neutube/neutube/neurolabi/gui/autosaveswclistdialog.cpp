#include "autosaveswclistdialog.h"

#include <iostream>
#include "ui_autosaveswclistdialog.h"
#include "neutubeconfig.h"
#include "zstackframe.h"
#include "tz_error.h"

AutosaveSwcListDialog::AutosaveSwcListDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::AutosaveSwcListDialog)
{
  ui->setupUi(this);
  ui->listView->setModel(&m_fileList);
  connect(ui->listView, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(viewSwc(QModelIndex)));
}

AutosaveSwcListDialog::~AutosaveSwcListDialog()
{
  delete ui;
}

void AutosaveSwcListDialog::updateFile()
{
  m_fileList.loadDir(
        NeutubeConfig::getInstance().getPath(NeutubeConfig::AUTO_SAVE).c_str());
}

void AutosaveSwcListDialog::viewSwc(const QModelIndex &index)
{
  QString fileName = m_fileList.data(index).toString();

  if (!fileName.isEmpty()) {
    fileName = "/" + fileName + ".swc";
    fileName =
        NeutubeConfig::getInstance().getPath(NeutubeConfig::AUTO_SAVE).c_str() +
        fileName;


    ZStackFrame *frame = new ZStackFrame;

    if (frame->readStack(fileName.toStdString().c_str()) == SUCCESS) {
      frame->open3DWindow(this->parentWidget());
      delete frame;
    } else {
      std::cerr << "Cannot open " << fileName.toStdString() << std::endl;
    }
  }
}

void AutosaveSwcListDialog::on_pushButton_clicked()
{
  ui->listView->setUpdatesEnabled(false);
  m_fileList.deleteAll();
  ui->listView->setUpdatesEnabled(true);
}
