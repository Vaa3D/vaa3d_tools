#ifndef MYDIALOG_H
#define MYDIALOG_H

#include <QtGui>
#include "cut_image_p.h"

class mydialog:public QDialog
{
    Q_OBJECT
public:
    mydialog();

public:
    ~mydialog();

    parameter1 getParameter();

private slots:
    void on_pushButton_s_eswcfiles_clicked();
    void on_pushButton_s_brain_clicked();
    void on_pushButton_s_savepath_clicked();
signals:


private:
    QDialog* centarldialog;
    QGridLayout* layout_central;
    QGridLayout* layout1;
    QGridLayout* layout2;
    QHBoxLayout* hbox1;
    QLabel* eswcfiles;
    QLabel* brain;
    QLabel* savepath;
    QLineEdit* e_eswcfiles;
    QLineEdit* e_brain;
    QLineEdit* e_savepath;
    QPushButton* s_eswcfiles;
    QPushButton* s_brain;
    QPushButton* s_savepath;
    QLabel* dx;
    QLabel* dy;
    QLabel* dz;
    QSpinBox* e_dx;
    QSpinBox* e_dy;
    QSpinBox* e_dz;
    QCheckBox* singleTree;
    QPushButton* button_ok;
    QPushButton* button_cancel;


};





























#endif // MYDIALOG_H
