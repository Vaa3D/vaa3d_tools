#ifndef SET_PAR_DIALOG_H
#define SET_PAR_DIALOG_H
#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include<QDialog>
#include "ui_seting_parameter.h"

class SetDialog :  public QDialog ,public Ui::set_par
{
        Q_OBJECT


public:
        SetDialog()
        {
            setWindowTitle(QString("Change parameters"));
            connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(close()));
           // setupUi(this);


        }

        ~SetDialog(){}

};

#endif // SET_PAR_DIALOG_H
