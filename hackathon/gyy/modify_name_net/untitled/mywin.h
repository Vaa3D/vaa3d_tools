#ifndef MYWIN_H
#define MYWIN_H
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>


class MyWin : public QWidget
{
    Q_OBJECT
private:
    QLabel *lbl_dir, *lbl_str_before, *lbl_str_after;
    QLineEdit *lnd_dir; //目录路径的输入框
    QLineEdit *lnd_str_before; //修改前的字符串输入框
    QLineEdit *lnd_str_after;  //修改后的字符串输入框
    QPushButton *btn_dir, *btn_change;
    QProgressBar *bar;
    QHBoxLayout *hlayout1, *hlayout2;
    QVBoxLayout *vlayout;

public:
    explicit MyWin(QWidget *parent = 0);
    ~MyWin();

signals:

public slots:
    void slot_btn_dir();
    void slot_btn_change();
};
#endif // MYWIN_H
