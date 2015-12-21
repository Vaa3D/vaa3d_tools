
#ifndef __VN_RIVULET_H__
#define __VN_RIVULET_H__


struct PARA_RIVULET
{
    unsigned char threshold;
    unsigned char stepsize;
    unsigned char gap; 
    unsigned char dumpbranch; 
    unsigned char connectrate; 
    double percentage;
    double sigmavalue;
    double alpha_one_value;
    double alpha_two_value;
    V3DLONG channel;

    
    QString inimg_file, inmarker_file, outswc_file;
    
    PARA_RIVULET() 
    {
        threshold = 10;
        stepsize = 1;
        gap = 15; 
        dumpbranch = 0; 
        connectrate = 1.5; 
        percentage = 0.98;
        sigmavalue = 3;
        alpha_one_value = 0.5;
        alpha_two_value = 1;

        inimg_file = "";
        inmarker_file = "";
        outswc_file = "";
    }
    
    bool rivulet_dialog()
    {
        // fetch parameters from dialog
        {
            //set update the dialog
            QDialog * dialog = new QDialog();
            dialog->setWindowTitle("Rivulet -- Neuron Tracing");
            QGridLayout * layout = new QGridLayout();

            
            QSpinBox * channel_spinbox = new QSpinBox();
            channel_spinbox->setRange(1,3);
            channel_spinbox->setValue(1);

            QSpinBox * bkgthresh_spinbox = new QSpinBox();
            bkgthresh_spinbox->setRange(-1, 255);
            bkgthresh_spinbox->setValue(10);

            QSpinBox * gap_spinbox = new QSpinBox();
            gap_spinbox->setRange(0,100);
            gap_spinbox->setValue(15);

            QCheckBox * dump_checker = new QCheckBox();
            dump_checker->setChecked(false);

            QSpinBox * connectrate_spinbox = new QSpinBox();
            connectrate_spinbox->setRange(1, 6);
            connectrate_spinbox->setValue(2.0);

            QDoubleSpinBox * percentage_spinbox = new QDoubleSpinBox();
            percentage_spinbox->setRange(0.0, 3.0);
            percentage_spinbox->setValue(0.98);

            QDoubleSpinBox * sigma = new QDoubleSpinBox();
            sigma->setRange(1.0, 6.0);
            sigma->setValue(3);

            QDoubleSpinBox * alpha_one = new QDoubleSpinBox();
            alpha_one->setRange(0.0, 3.0);
            alpha_one->setValue(0.5);

            QDoubleSpinBox * alpha_two = new QDoubleSpinBox();
            alpha_two->setRange(0.0, 3.0);
            alpha_two->setValue(1);

            layout->addWidget(new QLabel("color channel"),0,0);
            layout->addWidget(channel_spinbox, 0,1,1,5);
            layout->addWidget(new QLabel("background"),1,0);
            layout->addWidget(bkgthresh_spinbox, 1,1,1,5);
            layout->addWidget(new QLabel("gap allowed"),2,0);
            layout->addWidget(gap_spinbox, 2,1,1,5);
            layout->addWidget(new QLabel("dump unconnected branches"),3,0);
            layout->addWidget(dump_checker, 3,1,1,5);
            layout->addWidget(new QLabel("connectrate"),4,0);
            layout->addWidget(connectrate_spinbox, 4,1,1,5);
            layout->addWidget(new QLabel("percentage_spinbox"),5,0);
            layout->addWidget(percentage_spinbox, 5,1,1,5);
            layout->addWidget(new QLabel("vesselness sigma"),6,0);
            layout->addWidget(sigma, 6,1,1,5);
            layout->addWidget(new QLabel("vesselness alpha one"),7,0);
            layout->addWidget(alpha_one, 7,1,1,5);
            layout->addWidget(new QLabel("vesselness alpha two"),8,0);
            layout->addWidget(alpha_two, 8,1,1,5);
            
            QHBoxLayout * hbox3 = new QHBoxLayout();
            QPushButton * ok = new QPushButton(" ok ");
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox3->addWidget(cancel);
            hbox3->addWidget(ok);
            
            layout->addLayout(hbox3,9,0,1,6);
            dialog->setLayout(layout);
            QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
            QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
            
            //run the dialog
            
            if(dialog->exec() != QDialog::Accepted)
                return false;
            
            //get the dialog return values
            channel = channel_spinbox->value() - 1;
            threshold = bkgthresh_spinbox->value();
            gap = gap_spinbox->value();
            dumpbranch = dump_checker->isChecked();
            connectrate = connectrate_spinbox->value();
            percentage = percentage_spinbox->value();
            sigmavalue = sigma->value();
            alpha_one_value = alpha_one->value();
            alpha_two_value = alpha_two->value();

          
            if (dialog) {delete dialog; dialog=0;}
        }
        
        return true;
    }
    
};


#endif

