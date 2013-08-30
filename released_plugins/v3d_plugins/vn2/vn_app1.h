//last change: by Hanchuan Peng. 2013-01-02

#ifndef __VN1_PARA_H__
#define __VN1_PARA_H__

#include "vn.h"

struct PARA_APP1: public PARA_VN
{
    int  bkg_thresh; //for initial reconstruction generation
    int  visible_thresh; //for dark pruning
    int  channel;
    int  downsample_factor; //when set to be 0, then set b_256cube to be 1 which means downsample based on fit to a 256px cube automatically
    int  b_256cube;
    
    QString inimg_file, inmarker_file, outswc_file;
    
    PARA_APP1()
    {
        bkg_thresh = -1; //-1 for auto-thresholding, i.e. using the ave of image channel; //10; //30; change to 10 by PHC 2012-10-11
        visible_thresh = 30;
        channel = 0;
        downsample_factor = 2;
        b_256cube = 1; //whether or not preprocessing to downsample to a 256xYxZ cube UINT8 for tracing
        
        inimg_file = "";
        inmarker_file = "";
        outswc_file = "";
    }
    
    bool app1_dialog()
    {
        if (!p4dImage || !p4dImage->valid())
            return false;
        
        int chn_num = p4dImage->getCDim();
        // fetch parameters from dialog
        {
            //set update the dialog
            QDialog * dialog = new QDialog();
            dialog->setWindowTitle("Vaa3D-Neuron2 Auto_tracing Based on APP1 (All-Path-Pruning-v1)");
            QGridLayout * layout = new QGridLayout();
            
            QSpinBox * channel_spinbox = new QSpinBox();
            channel_spinbox->setRange(1,chn_num);
            channel_spinbox->setValue(1);
            
            QSpinBox * bkgthresh_spinbox = new QSpinBox();
            bkgthresh_spinbox->setRange(-1, 255);
            bkgthresh_spinbox->setValue(bkg_thresh);

            QSpinBox * visthresh_spinbox = new QSpinBox();
            visthresh_spinbox->setRange(1, 255);
            visthresh_spinbox->setValue(visible_thresh);
            
            QSpinBox * downsample_spinbox = new QSpinBox();
            downsample_spinbox->setRange(0, 10);
            downsample_spinbox->setValue(downsample_factor);
            
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
            dialog->setLayout(layout);
            QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
            QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
            
            //run the dialog
            
            if(dialog->exec() != QDialog::Accepted)
                return false;
            
            //get the dialog return values
            channel = channel_spinbox->value() - 1;
            bkg_thresh = bkgthresh_spinbox->value();
            visible_thresh = visthresh_spinbox->value();
            downsample_factor = downsample_spinbox->value();
            
            b_256cube = (downsample_factor==0) ? 1 : 0;
            
            if (dialog) {delete dialog; dialog=0;}
        }
        
        return true;
    }
    
    bool fetch_para_commandline(const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

bool proc_app1(V3DPluginCallback2 &callback, PARA_APP1 &p, const QString & versionStr);

#endif

