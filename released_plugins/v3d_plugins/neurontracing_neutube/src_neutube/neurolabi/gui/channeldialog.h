/**@file channeldialog.h
 * @brief Channel dialog
 * @author Ting Zhao
 */
#ifndef CHANNELDIALOG_H
#define CHANNELDIALOG_H

#include <QDialog>

namespace Ui {
    class ChannelDialog;
}

class ChannelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChannelDialog(QWidget *parent = 0, int nchannel = 3);
    ~ChannelDialog();

public: // attribute access
  int channel();

signals:
  void valueChanged();

private:
    Ui::ChannelDialog *ui;
};

#endif // CHANNELDIALOG_H
