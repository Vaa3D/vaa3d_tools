#include "mydialog.h"

MyDialog::MyDialog()
{
    //basic init  the whole window
    this->setWindowTitle("Para get Window");
    this->resize(1200, 800);
    vector_label_checked_num=-1;
    vector_label_checked_num_right=-1;
    centralWidget = new QWidget(this);
    gridLayoutWidget_central=new QWidget(centralWidget);
    label_tip=new QLabel;
    label_tip->setText("decide the operation consequence");
    button_add=new QPushButton;
    button_add->setText("<<");
    button_remove=new QPushButton;
    button_remove->setText(">>");
    button_up=new QPushButton;
    button_up->setText("up");
    button_down=new QPushButton;
    button_down->setText("down");
    button_ok=new QPushButton;
    button_ok->setText("ok");
    button_cancel=new QPushButton;
    button_cancel->setText("cancel");

    layout_v=new QVBoxLayout;
    layout_v->addWidget(button_add);
    layout_v->addWidget(button_up);
    layout_v->addWidget(button_down);
    layout_v->addWidget(button_remove);

    layout_h=new QHBoxLayout;
    layout_h->addWidget(button_ok);
    layout_h->addWidget(button_cancel);

    scrollArea_left=new QScrollArea(gridLayoutWidget_central);
    scrollAreaWidgetContents_left = new QWidget();
    scrollAreaWidgetContents_left->setGeometry(QRect(0,0,400,700));
    gridLayoutWidget_left=new QWidget(scrollAreaWidgetContents_left);
    gridLayoutWidget_left->setGeometry(QRect(0, 0, 421, 701));
    layout_left=new QGridLayout(gridLayoutWidget_left);
    scrollAreaWidgetContents_left->setLayout(layout_left);
    scrollArea_left->setWidget(scrollAreaWidgetContents_left);


    scrollArea_right=new QScrollArea(gridLayoutWidget_central);
    scrollAreaWidgetContents_right = new QWidget();
    scrollAreaWidgetContents_right->setGeometry(QRect(0,0,400,700));
    gridLayoutWidget_right=new QWidget(scrollAreaWidgetContents_right);
    gridLayoutWidget_right->setGeometry(QRect(0, 0, 421, 701));
    layout_right=new QGridLayout(gridLayoutWidget_right);
    scrollAreaWidgetContents_right->setLayout(layout_right);
    scrollArea_right->setWidget(scrollAreaWidgetContents_right);

    layout_central=new QGridLayout(gridLayoutWidget_central);
    layout_central->addWidget(scrollArea_left,1,0);
    layout_central->addLayout(layout_v,1,1);
    layout_central->addWidget(scrollArea_right,1,2);
    layout_central->addLayout(layout_h,2,1);

    button_add->raise();
    button_remove->raise();
    button_up->raise();
    button_down->raise();
    button_ok->raise();
    button_cancel->raise();
    this->setLayout(layout_central);
    setWindowFlags(  Qt::WindowCloseButtonHint);

    //signal
    connect(button_add, SIGNAL(clicked()), this, SLOT(on_pushButton_add_clicked()));
    connect(button_remove, SIGNAL(clicked()), this, SLOT(on_pushButton_remove_clicked()));
    connect(button_up, SIGNAL(clicked()), this, SLOT(on_pushButton_up_clicked()));
    connect(button_down, SIGNAL(clicked()), this, SLOT(on_pushButton_down_clicked()));
    connect(button_ok, SIGNAL(clicked()), this, SLOT(accept()));
    connect(button_cancel, SIGNAL(clicked()), this, SLOT(reject()));
}

void MyDialog::on_pushButton_add_clicked()
{
    if(vector_label_checked_num_right==-1||vector_label_checked_num_right>=vector_label_right.size())
    {
        //error
        return;
    }
    MyQLabel* new_label=new MyQLabel;
    new_label->setText(vector_label_right.at(vector_label_checked_num_right)->text());
    new_label->setMinimumHeight(20);
    QFont ft;
    ft.setPointSize(14);
    new_label->setFont(ft);
    new_label->setAlignment(Qt::AlignHCenter);
    vector_label.push_back(new_label);
    this->setLeftScroll();

    connect(new_label, SIGNAL(clicked_num(int)), this, SLOT(on_pushButton_item_clicked(int)));

}
QStringList MyDialog::return_left_para()
{
    QStringList paralist;
    for(int i=0;i<vector_label.size();i++)
    {
        paralist.append(vector_label.at(i)->text());
    }
    return paralist;

}


void MyDialog::on_pushButton_item_clicked(int num)
{
//    qDebug()<<num;
    vector_label_checked_num=num;

    for(int i=0;i<vector_label.size();i++)
    {
        if(i!=num)
        {
           vector_label.at(i)->init();
        }
    }
    this->setLeftScroll();


}
void MyDialog::on_pushButton_remove_clicked()
{
    if(vector_label_checked_num<=-1||vector_label_checked_num>=vector_label.size())
    {
        //error
        return;
    }
    delete  vector_label.at(vector_label_checked_num);
    for(int i=vector_label_checked_num;i<vector_label.size()-1;i++)
    {
        vector_label[i]=vector_label[i+1];
    }
    vector_label.pop_back();
    vector_label_checked_num=-1;
    this->setLeftScroll();


}
void MyDialog::on_pushButton_up_clicked()
{
    if(vector_label_checked_num<=-1||vector_label_checked_num>=vector_label.size())
    {
        //error
        return;
    }
    if(vector_label_checked_num==0)
    {
        //no need for up
        return;
    }
    MyQLabel* temp=vector_label[vector_label_checked_num-1];
    vector_label[vector_label_checked_num-1]=vector_label[vector_label_checked_num];
    vector_label[vector_label_checked_num]=temp;
    for(int i=0;i<vector_label.size();i++)
    {
        vector_label.at(i)->num=i;
    }
    vector_label_checked_num--;
    this->setLeftScroll();

}
void MyDialog::on_pushButton_down_clicked()
{
    if(vector_label_checked_num<=-1||vector_label_checked_num>=vector_label.size())
    {
        //error
        return;
    }
    if(vector_label_checked_num==vector_label.size()-1)
    {
        //no need for down
        return;
    }
    MyQLabel* temp=vector_label[vector_label_checked_num];
    vector_label[vector_label_checked_num]=vector_label[vector_label_checked_num+1];
    vector_label[vector_label_checked_num+1]=temp;
    for(int i=0;i<vector_label.size();i++)
    {
        vector_label.at(i)->num=i;
    }
    vector_label_checked_num++;
    this->setLeftScroll();

}


void MyDialog::setLeftScroll()
{
    QLayoutItem *child;
    while((child = layout_left->takeAt(0)) != 0)
    {
        layout_left->removeWidget(child->widget());
    }
    for(int i=0;i<vector_label.size();i++)
    {
        layout_left->addWidget(vector_label.at(i));
        vector_label.at(i)->num=i;
    }
}

void MyDialog::on_pushButton_right_para_clicked(int num)
{
//    qDebug()<<num;
    vector_label_checked_num_right=num;

    for(int i=0;i<vector_label_right.size();i++)
    {
        if(i!=num)
        {
           vector_label_right.at(i)->init();
        }
    }

}
void MyDialog::setRightParaList(QStringList paralist)
{
    for(int temp=0;temp<paralist.size();temp++)
    {
        MyQLabel* new_label=new MyQLabel;
        new_label->setText(paralist.at(temp));
        new_label->setMinimumHeight(40);
        QFont ft;
        ft.setPointSize(14);
        new_label->setFont(ft);
        new_label->setAlignment(Qt::AlignHCenter);
        connect(new_label, SIGNAL(clicked_num(int)), this, SLOT(on_pushButton_right_para_clicked(int)));
        layout_right->addWidget(new_label);
        new_label->num=temp;
        vector_label_right.push_back(new_label);
    }
}



MyDialog::~MyDialog()
{
    //delete
    delete centralWidget;
    delete scrollAreaWidgetContents_left;
    delete scrollAreaWidgetContents_right;
    delete label_tip;
    delete scrollArea_left;
    delete scrollArea_right;
    delete button_add;
    delete button_remove;
    delete button_up;
    delete button_down;
    delete button_ok;
    delete button_cancel;
    delete layout_v;
    delete layout_h;
    //some thing error happened
//    delete layout_left;
//    delete layout_right;
//    delete layout_central;
//    delete gridLayoutWidget_central;
//    for(int i=0;i<vector_label.size();i++)
//    {
//        delete vector_label.at(i);
//    }
}


