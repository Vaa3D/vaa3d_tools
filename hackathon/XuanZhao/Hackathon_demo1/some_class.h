#ifndef SOME_CLASS_H
#define SOME_CLASS_H


#include <vector>
#include <QtGui>
#include <v3d_interface.h>
using namespace std;

struct block{
    int n;
    NeuronSWC o;
    vector<NeuronSWC> tips;
    //vector<point> pointTree;
    V3DLONG min_x,max_x,min_y,max_y,min_z,max_z;
    int parent;
};

class blockTree{
public:
    QList<block> blocklist;
    QString name;
};

struct PARA_DEMO1
{
    int dx;
    int dy;
    int dz;
    PARA_DEMO1()
    {
        dx=128;
        dy=128;
        dz=64;
    }

    bool DEMO1()
    {
        {
            QDialog * dialog= new QDialog();
            dialog->setWindowTitle("get parameters on Hackathon_demo1");
            //dialog->setBaseSize(50,50);
            QGridLayout * layout = new QGridLayout();

            QSpinBox * dx_spinbox = new QSpinBox();
            //dx_spinbox->show();
            dx_spinbox->setRange(1,65535);
            dx_spinbox->setValue(dx);
            QSpinBox * dy_spinbox = new QSpinBox();
            //dy_spinbox->show();
            dy_spinbox->setRange(1,65535);
            dy_spinbox->setValue(dy);
            QSpinBox * dz_spinbox = new QSpinBox();
            //dz_spinbox->show();
            dz_spinbox->setRange(1,65535);
            dz_spinbox->setValue(dz);

            layout->addWidget(new QLabel("dx"),0,0);
            layout->addWidget(dx_spinbox,0,1,1,5);
            layout->addWidget(new QLabel("dy"),1,0);
            layout->addWidget(dy_spinbox,1,1,1,5);
            layout->addWidget(new QLabel("dz"),2,0);
            layout->addWidget(dz_spinbox,2,1,1,5);

            QHBoxLayout * hbox3 = new QHBoxLayout();
            QPushButton * ok = new QPushButton(" ok ");
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox3->addWidget(cancel);
            hbox3->addWidget(ok);

            layout->addLayout(hbox3,7,0,1,6);
            dialog->setLayout(layout);


            QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
            QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));


            if(dialog->exec() != QDialog::Accepted)
                return false;

            dx=dx_spinbox->value();
            dy=dy_spinbox->value();
            dz=dz_spinbox->value();

            if(dialog)
            {
                delete dialog; dialog=0;
            }
        }

        return true;

    }
};































#endif // SOME_CLASS_H
