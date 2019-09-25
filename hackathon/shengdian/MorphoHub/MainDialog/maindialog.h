#ifndef MAINDIALOG_H
#define MAINDIALOG_H
#include <QDialog>
class MainDialog: public QDialog
{
    Q_OBJECT
public:
    explicit MainDialog(QWidget *parent = 0);
    MainDialog();
    ~MainDialog();
};

#endif // MAINDIALOG_H
