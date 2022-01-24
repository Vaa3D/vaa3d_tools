#ifndef MYSWITCH_H
#define MYSWITCH_H

#include <QWidget>
#include <QLayout>
#include <QPushButton>
#include "v3d_interface.h"
#include <QLabel>
class myswitch: public QWidget
{
    Q_OBJECT
public:
    explicit myswitch(V3DPluginCallback2* callback, QWidget *parent = 0)
    {

        this->callback = callback;
        label = new QLabel("Please do not switch the resolution");
        before_img=new QPushButton;
        before_img->setText("before");
        after_img=new QPushButton;
        after_img->setText("after");
        layout=new QHBoxLayout;
        layout->addWidget(label);
        layout->addWidget(before_img);
        layout->addWidget(after_img);
        this->setLayout(layout);
        connect(before_img,SIGNAL(clicked()),this,SLOT(beforeClickedSlot()));
        connect(after_img,SIGNAL(clicked()),this,SLOT(afterClickedSlot()));
    }
    v3dhandleList handlelist;//存放两个图


public slots:
        void beforeClickedSlot();
        void afterClickedSlot();
        void closeEvent(QCloseEvent *event);

private:
    QPushButton *before_img;
    QPushButton *after_img;
    QHBoxLayout *layout;
    QLabel *label;
    V3DPluginCallback2 *callback;
};

#endif // MYSWITCH_H
