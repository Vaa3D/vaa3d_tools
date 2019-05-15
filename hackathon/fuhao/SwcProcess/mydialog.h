#ifndef MYDIALOG_H
#define MYDIALOG_H
#include <QObject>
#include<QScrollArea>
#include<QPushButton>
#include<QLabel>
#include<QGridLayout>
#include<QVector>
#include<QDebug>
#include<QMouseEvent>
#include<QDialog>
class MyQLabel : public QLabel
{
    Q_OBJECT
public :
    MyQLabel(){num=-1;}
    int num;
    void init()
    {this->setStyleSheet("color:black;");}

signals:
    void clicked_num(int num);

protected:
    void mousePressEvent(QMouseEvent * event)
    {
        if(event->button() == Qt::LeftButton)
        {
            this->setStyleSheet("color:red;");
        }
        emit clicked_num(num);
    }

};

class MyDialog : public QDialog
{
    Q_OBJECT
public:
    MyDialog();

public:
    ~MyDialog();
    void setRightParaList(QStringList paralist);
    QStringList return_left_para();
private slots:
    void on_pushButton_add_clicked();
    void on_pushButton_remove_clicked();
    void on_pushButton_up_clicked();
    void on_pushButton_down_clicked();
    void setLeftScroll();
    void on_pushButton_item_clicked(int num);
    void on_pushButton_right_para_clicked(int num);
signals:


private:
    QWidget *centralWidget;
    QWidget *scrollAreaWidgetContents_left;
    QWidget *scrollAreaWidgetContents_right;
    QWidget *gridLayoutWidget_central;
    QLabel *label_tip;
    QScrollArea *scrollArea_left;
    QScrollArea *scrollArea_right;
    QPushButton *button_add;
    QPushButton *button_remove;
    QPushButton *button_up;
    QPushButton *button_down;
    QPushButton *button_ok;
    QPushButton *button_cancel;
    QVBoxLayout *layout_v;
    QHBoxLayout *layout_h;
    QGridLayout *layout_left;
    QGridLayout *layout_right;
    QGridLayout *layout_central;
    QWidget *gridLayoutWidget_left;
    QWidget *gridLayoutWidget_right;

    QVector<MyQLabel*> vector_label;
    int vector_label_checked_num;
    QVector<MyQLabel*> vector_label_right;
    int vector_label_checked_num_right;

};

#endif // MYDIALOG_H
