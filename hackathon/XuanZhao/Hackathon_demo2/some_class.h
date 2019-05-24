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

class PARA_DEMO2:public QObject
{
    Q_OBJECT
public:

private slots:

    void getFiles();

    void savePath();
    void getDirectory();
public:
    QWidget* parent;
    int dx;
    int dy;
    int dz;
    QList<QString> eswcfiles;
    QList<QString> tiffiles;
    QString brainpath;
    QString savepath;
    bool singleTree;
    bool classify;
    ~PARA_DEMO2()
    {

    }
    PARA_DEMO2(const PARA_DEMO2 &_other)
    {}
    PARA_DEMO2()
    {
        dx=128;
        dy=128;
        dz=64;
        singleTree=false;
        classify=false;
        eswcfiles.clear();
        tiffiles.clear();
        brainpath="";
    }

    bool demo2_dialog()
    {
        {
            {

                QDialog * dialog = new QDialog();
                dialog->setWindowTitle("Hackathon_demo2");
                //dialog->setBaseSize(50,50);
                //QVBoxLayout * alayout = new QVBoxLayout();

                QGridLayout * layout = new QGridLayout();

                QPushButton * getEswcFiles = new QPushButton("select eswcfiles or swcfiles");
                //QSpacerItem * spacer = new QSpacerItem(5,2);
                QPushButton * getBrain = new QPushButton("select brain");
                QPushButton * save = new QPushButton("select the path of saving");
                QObject::connect(getEswcFiles,SIGNAL(clicked()),this,SLOT(getFiles()));
                QObject::connect(getBrain,SIGNAL(clicked()),this,SLOT(getDirectory()));
                QObject::connect(save,SIGNAL(clicked()),this,SLOT(savePath()));

                QHBoxLayout * hbox1=new QHBoxLayout();
                hbox1->addWidget(getEswcFiles);
                //hbox1->addWidget(spacer);
                hbox1->addWidget(getBrain);
                hbox1->addWidget(save);

                layout->addLayout(hbox1,0,0);


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

                layout->addWidget(new QLabel("dx"),1,0);
                layout->addWidget(dx_spinbox,1,1,1,5);
                layout->addWidget(new QLabel("dy"),2,0);
                layout->addWidget(dy_spinbox,2,1,1,5);
                layout->addWidget(new QLabel("dz"),3,0);
                layout->addWidget(dz_spinbox,3,1,1,5);

                QHBoxLayout * hbox2 = new QHBoxLayout();
                QCheckBox * single = new QCheckBox();
                single->setText("singleTree");
                single->setChecked(singleTree);
                hbox2->addWidget(single);

                QHBoxLayout * hbox3 = new QHBoxLayout();
                QPushButton * ok = new QPushButton(" ok ");
                ok->setDefault(true);
                QPushButton * cancel = new QPushButton("cancel");
                hbox3->addWidget(cancel);
                hbox3->addWidget(ok);



                //layout->addWidget(hbox1,3,0,1,6);
                layout->addLayout(hbox2,4,2);

                layout->addLayout(hbox3,7,0,1,6);
                dialog->setLayout(layout);


                QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
                QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));


                if(dialog->exec() != QDialog::Accepted)
                    return false;

                dx=dx_spinbox->value();
                dy=dy_spinbox->value();
                dz=dz_spinbox->value();
                singleTree=single->isChecked();

                if(dialog)
                {
                    delete dialog; dialog=0;
                }
            }

            return true;

        }
    }

};


#endif
