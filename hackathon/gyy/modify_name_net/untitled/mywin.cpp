#include "mywin.h"
#include <QFile>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
MyWin::MyWin(QWidget *parent) : QWidget(parent)
{
    lbl_dir = new QLabel("path:");
    lnd_dir = new QLineEdit;
    btn_dir = new QPushButton;
    btn_dir->setText("...");
    hlayout1 = new QHBoxLayout;
    hlayout1->addWidget(lbl_dir, 1);
    hlayout1->addWidget(lnd_dir, 8);
    hlayout1->addWidget(btn_dir, 1);
    lbl_str_before = new QLabel("before:");
    lnd_str_before = new QLineEdit;
    lbl_str_after = new QLabel("after:");
    lnd_str_after = new QLineEdit;
    btn_change = new QPushButton("change");
    hlayout2 = new QHBoxLayout;
    hlayout2->addWidget(lbl_str_before, 2);
    hlayout2->addWidget(lnd_str_before, 3);
    hlayout2->addWidget(lbl_str_after, 2);
    hlayout2->addWidget(lnd_str_after, 3);
    hlayout2->addWidget(btn_change, 2);
    bar = new QProgressBar;
    vlayout = new QVBoxLayout(this);
    vlayout->addLayout(hlayout1);
    vlayout->addLayout(hlayout2);
    vlayout->addWidget(bar);
    resize(320, 140);
    // 信号与槽的参数需要匹配，但如果信号的参数有默认值，可以连接没参数的槽
    connect(btn_dir, SIGNAL(clicked(bool)), this, SLOT(slot_btn_dir()));
    connect(btn_change, SIGNAL(clicked(bool)), this, SLOT(slot_btn_change()));
}
MyWin::~MyWin()
{
    delete lbl_dir;
    delete lbl_str_after;
    delete lbl_str_before;
    delete lnd_dir;
    delete lnd_str_after;
    delete lnd_str_before;
    delete btn_change;
    delete btn_dir;
    delete hlayout1;
    delete hlayout2;
    delete vlayout;
}
void MyWin::slot_btn_change()
{
    QString str_dir = lnd_dir->text();
    if (str_dir.isEmpty())
    {
        QMessageBox::critical(this, "error", "no dir selected");
        lnd_dir->setFocus();
        return;
    }
    QDir dir(str_dir);
    if (!dir.exists()) //目录不存在
    {
        QMessageBox::critical(this, "error", "invlaid dir");
        btn_dir->setFocus();
        return;
    }
    QString str_before = lnd_str_before->text();
    QString str_after = lnd_str_after->text();
    if (str_before.isEmpty())
    {
        QMessageBox::critical(this, "error", "no string");
        lnd_str_before->setFocus();
        return;
    }
    // 获取目录里的文件，并过滤".", ".."
    QStringList files = dir.entryList(QDir::AllEntries|QDir::NoDotAndDotDot);
    QFile f;
    QString str;
    int n;
    bar->setMaximum(files.size()); // 设置滚动条的最大值为文件的个数，每处理一个文件，滚动条的值+1
    for (int i = 0; i < files.size(); i++)
    {
        str = files.at(i);
        if (str.contains(str_before)) //如果包括有要修改的字符串
        {
            qDebug() << "before" << str;
            f.setFileName(str_dir + "/" + str);
            n = str.indexOf(str_before);
            str = str.replace(n, str_before.size(), str_after);
            qDebug() << "after: " << str;
            f.rename(str_dir + "/" + str);
        }
        bar->setValue(i+1);
 //       QThread::msleep(2000/files.size()); //纯属装X
    }
}
void MyWin::slot_btn_dir()
{
    //从根目录打开， 以当前窗口对象为中心， 标题栏为"open"
    QString str = QFileDialog::getExistingDirectory(this, "open", "/");
    if (str.isEmpty()) //用户取消选择目录
        return;
    lnd_dir->setText(str);
    lnd_str_before->setFocus(); //光标移动到修改前的字符串输入框
}
