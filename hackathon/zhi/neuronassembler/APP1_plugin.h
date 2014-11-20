/* neuronassembler_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2014-11-09 : by Zhi Zhou
 */
 
#ifndef __TReMap_PLUGIN_H__
#define __NEURONASSEMBLER_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class APP1TracingeDialog : public QDialog
    {
        Q_OBJECT

    public:
        APP1TracingeDialog(QWidget *parent)
        {

            QGridLayout * layout = new QGridLayout();

            channel_spinbox = new QSpinBox();
            channel_spinbox->setRange(1,1);
            channel_spinbox->setValue(1);

            bkgthresh_spinbox = new QSpinBox();
            bkgthresh_spinbox->setRange(-1, 255);
            bkgthresh_spinbox->setValue(-1);

            visthresh_spinbox = new QSpinBox();
            visthresh_spinbox->setRange(1, 255);
            visthresh_spinbox->setValue(30);

            downsample_spinbox = new QSpinBox();
            downsample_spinbox->setRange(0, 10);
            downsample_spinbox->setValue(2);

            layout->addWidget(new QLabel("color channel"),0,0);
            layout->addWidget(channel_spinbox, 0,1,1,5);
            layout->addWidget(new QLabel("background_threshold \n  (if set as -1, \nthen auto-thresholding)"),1,0);
            layout->addWidget(bkgthresh_spinbox, 1,1,1,5);
            layout->addWidget(new QLabel("visibility threshold \n  (normally do not need to change)"),2,0);
            layout->addWidget(visthresh_spinbox, 2,1,1,5);

            QHBoxLayout * hbox1 = new QHBoxLayout();
            hbox1->addWidget(new QLabel("downsample factor \n  (set 0 for auto-downsample)"));
            hbox1->addWidget(downsample_spinbox);

            layout->addLayout(hbox1,3,0,1,6);

            QHBoxLayout * hbox2 = new QHBoxLayout();
            QPushButton * ok = new QPushButton(" ok ");
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox2->addWidget(cancel);
            hbox2->addWidget(ok);

            layout->addLayout(hbox2,4,0,1,6);
            setLayout(layout);
            setWindowTitle(QString("Vaa3D-Neuron2 Auto_tracing Based on APP1 (All-Path-Pruning-v1)"));

            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));


            connect(channel_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(bkgthresh_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));

            connect(downsample_spinbox, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(visthresh_spinbox, SIGNAL(selectionChanged ()), this, SLOT(update()));

            update();
        }

        ~APP1TracingeDialog(){}

        public slots:
        void update()
        {
            channel = channel_spinbox->value();
            bkg_thresh = bkgthresh_spinbox->value();
            visible_thresh = visthresh_spinbox->value();
            b_256cube = (downsample_spinbox->value()==0) ? 1 : 0;
        }
    public:

        QSpinBox * channel_spinbox;
        QSpinBox * bkgthresh_spinbox;
        QSpinBox * downsample_spinbox;
        QSpinBox * visthresh_spinbox;

       // LandmarkList listLandmarks;
        int channel;
        int bkg_thresh;
        int visible_thresh;
        int b_256cube;
    };
#endif
