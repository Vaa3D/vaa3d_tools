#ifndef CHANGEACTIVEOVERLAYDIALOG_H
#define CHANGEACTIVEOVERLAYDIALOG_H

#include <QDialog>

namespace Ui {
class ChangeActiveOverlayDialog;
}

class ChangeActiveOverlayDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ChangeActiveOverlayDialog(QWidget *parent = 0);
    ~ChangeActiveOverlayDialog();
    
    int getActiveOverlay();

private:
    Ui::ChangeActiveOverlayDialog *ui;
};

#endif // CHANGEACTIVEOVERLAYDIALOG_H
