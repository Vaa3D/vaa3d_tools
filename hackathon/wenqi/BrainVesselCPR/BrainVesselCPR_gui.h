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
    int wl;
    int ww;
    QLabel* WL_label;
    QLabel* WW_label;
    QSpinBox* WL_spinbox;
    QSpinBox* WW_spinbox;


    QSlider* WL_slider;
    QSlider* WW_slider;
    //QPushButton* ok_button;
    //QPushButton* cancel_button;

    //QLineEdit* WL_line;
    //QLineEdit* WW_line;

    QHBoxLayout* hbox;
    QVBoxLayout* vbox1;
    QVBoxLayout* vbox2;
    QVBoxLayout* vbox3;
    //QVBoxLayout* vbox4;

    unsigned short int * data1d;

    V3DLONG N, M, P;



    SetContrastWidget(V3DPluginCallback2 &callback, QWidget * parent) : QWidget(parent)
    {
        cout << "break" << __LINE__ << endl;
        this->windowTitle() = tr("Adjust Contrast");
        this->callback = &callback;
        this->curwin = callback.currentImageWindow();
        cout << "break" << __LINE__ << endl;
        Image4DSimple * p4DImage = callback.getImage(curwin);

        cout << "break" << __LINE__ << endl;



        this->N = p4DImage->getXDim();
        this->M = p4DImage->getYDim();
        this->P = p4DImage->getZDim();
        cout << "break" << __LINE__ << endl;
        this->data1d = new unsigned short int[N*M*P];

//        if(NULL == (this->data1d = (unsigned short int*)malloc(sizeof(unsigned short int )*(N*M*P+10))))
//        {
//            cout << "malloc error!!!!!!" << endl;
//        }
//        else
//        {
//            cout << "malloc succeed!" << endl;
//        }
//
        memcpy(this->data1d, (unsigned short int *)p4DImage->getRawData(), N*M*P*sizeof(unsigned short int));
//        for(int i=0;i<N*M*P;i++)
//        {
//            this->data1d[i] = ((unsigned short int *)p4DImage->getRawData())[i];
//        }
        //this->data1d = (unsigned short int *)p4DImage->getRawData();
        cout << "break" << __LINE__ << endl;
        //this->data_container = data1d;
        wl = 400;
        ww = 1000;

        int WL_max = 4095;
        int WL_min = 0;
        int WW_max = 4095;
        int WW_min = 1;

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
        WL_slider->setFixedWidth(200);
        WW_slider->setFixedWidth(200);

        //ok_button = new QPushButton("OK");
        cout << "break" << __LINE__ << endl;


        cout << "break" << __LINE__ << endl;
        WL_spinbox->setValue(400);
        WW_spinbox->setValue(500);

        hbox = new QHBoxLayout();
        vbox1 = new QVBoxLayout();
        vbox2 = new QVBoxLayout();
        vbox3 = new QVBoxLayout();

        //vbox4 = new QVBoxLayout();

        //WW_line = new QLineEdit(tr("200"));
        //WL_line = new QLineEdit(tr("500"));

        vbox1->addWidget(WL_label);
        vbox1->addWidget(WW_label);
        vbox2->addWidget(WL_spinbox);
        vbox2->addWidget(WW_spinbox);
        //vbox2->addWidget(WL_line);
        //vbox2->addWidget(WW_line);
        vbox3->addWidget(WL_slider);
        vbox3->addWidget(WW_slider);

        //vbox4->addWidget(WL_line);
        //vbox4->addWidget(WW_line);


        hbox->addLayout(vbox1);
        hbox->addLayout(vbox2);
        hbox->addLayout(vbox3);

        //hbox->addLayout(vbox4);

        //hbox->addWidget(ok_button);


        setLayout(hbox);
        cout << "break" << __LINE__ << endl;

        connect(WL_spinbox, SIGNAL(valueChanged(int)), WL_slider, SLOT(setValue(int)));
        connect(WL_slider, SIGNAL(valueChanged(int)), WL_spinbox, SLOT(setValue(int)));
        connect(WW_spinbox, SIGNAL(valueChanged(int)), WW_slider, SLOT(setValue(int)));
        connect(WW_slider, SIGNAL(valueChanged(int)), WW_spinbox, SLOT(setValue(int)));

//        connect(WL_line, SIGNAL(valueChanged(int)), WL_slider, SLOT(setValue(int)));
//        connect(WL_slider, SIGNAL(valueChanged(int)), WL_line, SLOT(setValue(int)));
//        connect(WW_line, SIGNAL(valueChanged(int)), WW_slider, SLOT(setValue(int)));
//        connect(WW_slider, SIGNAL(valueChanged(int)), WW_line, SLOT(setValue(int)));
        connect(WL_slider, SIGNAL(sliderReleased()), this, SLOT(updateImage()));
        connect(WW_slider, SIGNAL(sliderReleased()), this, SLOT(updateImage()));
        //connect(WL_spinbox, SIGNAL(valueChanged(int)), this, SLOT(WWWL_spinboxValueChanged()));
        //connect(WW_spinbox, SIGNAL(valueChanged(int)), this, SLOT(WWWL_spinboxValueChanged()));
        //WW_spinbox->va

        //connect(ok_button,SIGNAL(clicked()), this, SLOT(updateImage()));

    }
    //~SetContrastWidget(){}

public slots:
//    void WWWLspinboxValueChanged()
//    {
//        wl = this->WL_spinbox->value();
//        ww = this->WW_spinbox->value();
//        updateImage();
//    }
    void updateImage()
    {
        wl = this->WL_slider->value();
        ww = this->WW_slider->value();
        V3DLONG totalpxls = this->N * this->M * this->P;

        //int wl = this->WL_line->text().toInt();
        //int ww = this->WW_line->text().toInt();
        cout<<"break" << __LINE__ <<endl;


        Image4DSimple * p4dImageNew = 0;
        p4dImageNew = new Image4DSimple();

        cout<<"break" << __LINE__ <<endl;

        if(!p4dImageNew->createImage(N,M,P,1, V3D_UINT16))
        {
            return;
        }
        cout<<"break" << __LINE__ <<endl;
        unsigned short int *tmp = (unsigned short int*)p4dImageNew->getRawData();

        for(int i=0; i<totalpxls; i++)
        {
            if(this->data1d[i] > wl + ww / 2)
            {
                tmp[i] = 4095;
            }
            else if(this->data1d[i] < wl - ww / 2)
            {
                tmp[i] = 0;
            }
            else
            {
                //this->new4DImage->getRawData()[i] = (unsigned short int)(this->data1d[i] - (wl - ww / 2)) * (4096 / ww);
                tmp[i] = (this->data1d[i] - (wl - ww / 2)) * (4096 / ww);
            }
            //cout<<"break" << __LINE__ <<endl;
            //this->new4DImage->getRawData()[2*i+1] = (unsigned char)(tmp >> 8 & 0x00ff);
            //this->new4DImage->getRawData()[2*i] = (unsigned char)(tmp & 0x00ff);
        }

        // display in current window
        cout<<"break" << __LINE__ <<endl;
        v3dhandle newwin;
        newwin = callback->currentImageWindow();
        cout<<"break" << __LINE__ <<endl;
        callback->setImage(newwin, p4dImageNew);
        cout<<"break" << __LINE__ <<endl;
        callback->setImageName(newwin, QString("contrast adjusted image"));
        callback->updateImageWindow(newwin);
        cout<<"break" << __LINE__ <<endl;
    }

};





class lookPanel: public QDialog
{
    Q_OBJECT

public:
    lookPanel(V3DPluginCallback2 &v3d, QWidget *parent);
    ~lookPanel();

public:
    QComboBox* combo_master;
    QComboBox* combo_slave;
    QLabel* label_master;
    QLabel* label_slave;
    QCheckBox* check_rotation;
    QCheckBox* check_shift;
    QCheckBox* check_zoom;
    QGridLayout *gridLayout;
    v3dhandleList win_list;
    v3dhandleList win_list_past;
    V3DPluginCallback2 & m_v3d;
    QTimer *m_pTimer;
    QPushButton* syncAuto;
    View3DControl *view_master;
    View3DControl *view_slave;
    int xRot_past, yRot_past,zRot_past;
    int xShift_past,yShift_past,zShift_past;
    int zoom_past;
    bool b_autoON;

private:
    void resetSyncAutoState();

private slots:
    void _slot_syncAuto();
    void _slot_sync_onetime();
    void _slot_timerupdate();
    void reject();
};

class MyComboBox : public QComboBox
{
    Q_OBJECT

public:
    V3DPluginCallback2 * m_v3d;
    MyComboBox(V3DPluginCallback2 * ini_v3d) {m_v3d = ini_v3d;}

    void enterEvent(QEvent * event);

public slots:
    void updateList();
};
class controlPanel: public QDialog
{
    Q_OBJECT

public:
    controlPanel(V3DPluginCallback2 &v3d, QWidget *parent);
    ~controlPanel();

public:
    v3dhandle curwin;
    V3dR_MainWindow *surface_win;
    View3DControl *view;

    QList <V3dR_MainWindow *> list_3dviewer;
    v3dhandleList list_triview;
    QGridLayout *gridLayout;
    V3DPluginCallback2 & m_v3d;

    MyComboBox* combo_surface;
    QLabel* label_surface;
    QCheckBox* check_rotation;
    QCheckBox* check_shift;
    QCheckBox* check_zoom;
    QTimer *m_pTimer;
    QPushButton* syncAuto;
    View3DControl *view_master;
    View3DControl *view_slave;
    int xRot_past, yRot_past,zRot_past;
    int xShift_past,yShift_past,zShift_past;
    int zoom_past;
    bool b_autoON;

private:
    void resetSyncAutoState();

private slots:
     void _slot_syncAuto();
     void _slot_sync_onetime();
     void _slot_timerupdate();
     void reject();
};


#endif // BRAINVESSELCPR_GUI_H
