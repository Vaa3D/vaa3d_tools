#ifndef BRAINVESSELCPR_GUI_H
#define BRAINVESSELCPR_GUI_H

#include <QtGui>
#include <v3d_interface.h>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <string>
#include <QtDebug>
#include<iostream>
using namespace std;
class SetContrastWidget: public QWidget
{
    Q_OBJECT

public:
    V3DPluginCallback2 * callback;
    v3dhandle curwin;
    int WL;
    int WW;
    QLabel* WL_label;
    QLabel* WW_label;
    QSpinBox* WL_spinbox;
    QSpinBox* WW_spinbox;
    QSlider* WL_slider;
    QSlider* WW_slider;
    QPushButton* ok_button;
    QPushButton* cancel_button;

    QLineEdit* WL_line;
    QLineEdit* WW_line;

    QHBoxLayout* hbox;
    QVBoxLayout* vbox1;
    QVBoxLayout* vbox2;
    QVBoxLayout* vbox3;
    QVBoxLayout* vbox4;

    unsigned short int * data1d;
    unsigned short int * data_container;
    V3DLONG N, M, P;

    Image4DSimple * new4DImage;


    SetContrastWidget(V3DPluginCallback2 &callback, QWidget * parent) : QWidget(parent)
    {
        this->windowTitle() = tr("Adjust Contrast");
        this->callback = &callback;
        this->curwin = callback.currentImageWindow();
        Image4DSimple * p4DImage = callback.getImage(curwin);

        this->new4DImage = new Image4DSimple();

        this->data1d = (unsigned short int *)p4DImage->getRawData();



        this->N = p4DImage->getXDim();
        this->M = p4DImage->getYDim();
        this->P = p4DImage->getZDim();

        this->new4DImage->setData((unsigned char *)data1d, N, M, P, 1, V3D_UINT16);

        this->data_container = new unsigned short int[N*M*P];
        //this->data_container = data1d;
        WL = 2048;
        WW = 4096;

        int WL_max = 4095;
        int WL_min = 0;
        int WW_max = 4095*2;
        int WW_min = 0;

        WL_label = new QLabel(tr("WL:"));
        WW_label = new QLabel(tr("WW:"));

        WL_spinbox = new QSpinBox(this);
        WW_spinbox = new QSpinBox(this);
        WW_spinbox->setMinimum(WW_min);
        WW_spinbox->setMaximum(WW_max);
        WL_spinbox->setMinimum(WL_min);
        WL_spinbox->setMaximum(WL_max);
        WL_spinbox->setFocusPolicy(Qt::NoFocus);
        WW_spinbox->setFocusPolicy(Qt::NoFocus);


        WL_slider = new QSlider(this);
        WW_slider = new QSlider(this);
        WL_slider->setOrientation(Qt::Horizontal);
        WW_slider->setOrientation(Qt::Horizontal);
        WL_slider->setMinimum(WL_min);
        WL_slider->setMaximum(WL_max);
        WW_slider->setMinimum(WW_min);
        WW_slider->setMaximum(WW_max);
        WL_slider->setSingleStep(1);
        WW_slider->setSingleStep(1);

        ok_button = new QPushButton("OK");

        connect(WL_spinbox, SIGNAL(valueChanged(int)), WL_slider, SLOT(setValue(int)));
        connect(WL_slider, SIGNAL(valueChanged(int)), WL_spinbox, SLOT(setValue(int)));
        connect(WW_spinbox, SIGNAL(valueChanged(int)), WW_slider, SLOT(setValue(int)));
        connect(WW_slider, SIGNAL(valueChanged(int)), WW_spinbox, SLOT(setValue(int)));
        //connect(WL_slider, SIGNAL(valueChanged(int)), this, SLOT(updateImage()));
        //connect(WW_slider, SIGNAL(valueChanged(int)), this, SLOT(updateImage()));
        connect(ok_button,SIGNAL(clicked()), this, SLOT(updateImage()));



        WL_spinbox->setValue(2048);
        WW_spinbox->setValue(4096);

        hbox = new QHBoxLayout();
        vbox1 = new QVBoxLayout();
        vbox2 = new QVBoxLayout();
        vbox3 = new QVBoxLayout();

        vbox4 = new QVBoxLayout();

        WW_line = new QLineEdit(tr("315"));
        WL_line = new QLineEdit(tr("478"));

        vbox1->addWidget(WL_label);
        vbox1->addWidget(WW_label);
        vbox2->addWidget(WL_spinbox);
        vbox2->addWidget(WW_spinbox);
        vbox3->addWidget(WL_slider);
        vbox3->addWidget(WW_slider);

        vbox4->addWidget(WL_line);
        vbox4->addWidget(WW_line);


        hbox->addLayout(vbox1);
        hbox->addLayout(vbox2);
        hbox->addLayout(vbox3);

        hbox->addLayout(vbox4);

        hbox->addWidget(ok_button);


        setLayout(hbox);


    }
    //~SetContrastWidget(){}

public slots:
    void updateImage()
    {

        V3DLONG totalpxls = this->N * this->M * this->P;
        //int wl = this->WL_slider->value();
        //int ww = this->WW_slider->value();
        int wl = this->WL_line->text().toInt();
        int ww = this->WW_line->text().toInt();
        cout<<"break" << __LINE__ <<endl;

        unsigned short int tmp;
        for(int i=0; i<totalpxls; i++)
        {
            if(this->data1d[i] > wl + ww / 2)
            {
                tmp = 4095;
            }
            else if(this->data1d[i] < wl - ww / 2)
            {
                tmp = 0;
            }
            else
            {
                //this->new4DImage->getRawData()[i] = (unsigned short int)(this->data1d[i] - (wl - ww / 2)) * (4096 / ww);
                tmp = (this->data1d[i] - (wl - ww / 2)) * (4096 / ww);
            }
            cout<<"break" << __LINE__ <<endl;
            this->new4DImage->getRawData()[2*i+1] = (unsigned char)(tmp >> 8 & 0x00ff);
            this->new4DImage->getRawData()[2*i] = (unsigned char)(tmp & 0x00ff);
        }



//new4DImage->setNewRawDataPointer();
        //new4DImage->setData((unsigned char *)data_container, N, M, P, 1, V3D_UINT16);
        //this->new4DImage->setData((unsigned char *)data1d, N, M, P, 1, V3D_UINT16);

// display in new window
//        v3dhandle newwin = callback->newImageWindow("new image");
//        callback->setImage(newwin, new4DImage);
//        qDebug() << "modified" << endl;
//        callback->setImageName(newwin, QObject::tr("Contrast Adjusted Image"));

//        callback->updateImageWindow(newwin);

        // display in current window

        v3dhandle newwin;
        newwin = callback->currentImageWindow();
        callback->setImage(newwin, new4DImage);
        cout<<"break" << __LINE__ <<endl;
        callback->setImageName(newwin, QString("contrast adjusted image"));
        callback->updateImageWindow(newwin);

cout<<"break" << __LINE__ <<endl;

//        if(!callback->currentImageWindow())
//            this->curwin = this->callback->newImageWindow();
//        else
//            this->curwin = this->callback->currentImageWindow();

//        this->callback->setImage(this->curwin, this->new4DImage);
//        qDebug() << "modified" << endl;



//        this->callback->setImageName(this->curwin,tr("modified"));

//        this->callback->updateImageWindow(this->curwin);
    }

};




#endif // BRAINVESSELCPR_GUI_H
