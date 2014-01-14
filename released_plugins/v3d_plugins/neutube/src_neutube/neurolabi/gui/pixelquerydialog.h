#ifndef PIXELQUERYDIALOG_H
#define PIXELQUERYDIALOG_H

#include <QDialog>

namespace Ui {
class PixelQueryDialog;
}

class PixelQueryDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit PixelQueryDialog(QWidget *parent = 0);
  ~PixelQueryDialog();
  
private:
  Ui::PixelQueryDialog *ui;
};

#endif // PIXELQUERYDIALOG_H
