#include "pixelquerydialog.h"
#include "ui_pixelquerydialog.h"

PixelQueryDialog::PixelQueryDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::PixelQueryDialog)
{
  ui->setupUi(this);
}

PixelQueryDialog::~PixelQueryDialog()
{
  delete ui;
}
