#ifndef QDLGPARA_H
#define QDLGPARA_H

#include <QDialog>

namespace Ui {
class QDlgPara;
}

class QDlgPara : public QDialog
{
    Q_OBJECT

public:
    explicit QDlgPara(QWidget *parent = 0);
    ~QDlgPara();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::QDlgPara *ui;
};

#endif // QDLGPARA_H
