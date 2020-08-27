//last change: by Hanchuan Peng. 2012-12-30

#ifndef __VN2_PARA_H__
#define __VN2_PARA_H__

#include "vn.h"

#include "../jba/newmat11/newmatap.h"
#include "../jba/newmat11/newmatio.h"

struct PARA_APP2: public PARA_VN
{
    bool is_gsdt;
    bool is_coverage_prune;
    bool is_break_accept;
    int  bkg_thresh;
    double length_thresh;
    int  cnn_type;
    int  channel;
    double SR_ratio;
    int  b_256cube;
    bool b_RadiusFrom2D; //how to estimate radius of each reconstruction node, from 2D plane (for anisotropic case) or 3D (for isotropic case)
    int b_resample;
    int b_intensity;
    int b_brightfiled;
    bool s2Mode;

    bool b_menu;

    int birfucation_thres;//added by xz

    
    QString inimg_file, inmarker_file, outswc_file;
    
    PARA_APP2() 
    {
        is_gsdt = false; //true; change to false by PHC, 2012-10-11. as there is clear GSDT artifacts
        is_coverage_prune = true;
        is_break_accept = true;
        bkg_thresh = 10; //30; change to 10 by PHC 2012-10-11
        length_thresh = 5; // 1.0; change to 5 by PHC 2012-10-11
        cnn_type = 2; // default connection type 2
        channel = 0;
        SR_ratio = 3.0/9.0;
        b_256cube = 1; //whether or not preprocessing to downsample to a 256xYxZ cube UINT8 for tracing
        b_RadiusFrom2D = true;
        b_resample = 1;
        b_intensity = 0;
        b_brightfiled = 0;
        b_menu = true;

        birfucation_thres = 30;

        inimg_file = "";
        inmarker_file = "";
        outswc_file = "";
    }
    
    bool app2_dialog()
    {
        if (!p4dImage || !p4dImage->valid())
            return false;
        
        int chn_num = p4dImage->getCDim();
        // fetch parameters from dialog
        {
            //set update the dialog
            QDialog * dialog = new QDialog();
            dialog->setWindowTitle("Vaa3D-Neuron2 Auto_tracing Based on APP2 (All-Path-Pruning-v2)");
            QGridLayout * layout = new QGridLayout();
            
            QSpinBox * channel_spinbox = new QSpinBox();
            channel_spinbox->setRange(1,chn_num);
            channel_spinbox->setValue(1);
            QSpinBox * cnntype_spinbox = new QSpinBox();
            cnntype_spinbox->setRange(1,3);
            cnntype_spinbox->setValue(2);
            QSpinBox * bkgthresh_spinbox = new QSpinBox();
            bkgthresh_spinbox->setRange(-1, 255);
            bkgthresh_spinbox->setValue(bkg_thresh);
            QLineEdit * lenthresh_editor = new QLineEdit(QString("").setNum(length_thresh));
            QLineEdit * srratio_editor = new QLineEdit(QString("").setNum(SR_ratio));
            QCheckBox * isgsdt_checker = new QCheckBox();
            isgsdt_checker->setChecked(is_gsdt);
            QCheckBox * iswb_checker = new QCheckBox();
            iswb_checker->setChecked(is_break_accept);
            QCheckBox * b256_checker = new QCheckBox();
            b256_checker->setChecked(b_256cube);
            QCheckBox * b_radius2Dchecker = new QCheckBox();
            b_radius2Dchecker->setChecked(b_RadiusFrom2D);
            QCheckBox * bresample_Checker = new QCheckBox();
            bresample_Checker->setChecked(b_resample);
            QCheckBox * b_intensity_Checker = new QCheckBox();
            b_intensity_Checker->setChecked(b_intensity);
            QCheckBox * b_brightfiled_Checker = new QCheckBox();
            b_brightfiled_Checker->setChecked(b_brightfiled);

            layout->addWidget(new QLabel("color channel"),0,0);
            layout->addWidget(channel_spinbox, 0,1,1,5);
            layout->addWidget(new QLabel("background_threshold \n(if set as -1, \nthen auto-thresholding)"),1,0);
            layout->addWidget(bkgthresh_spinbox, 1,1,1,5);
            
            QHBoxLayout * hbox1 = new QHBoxLayout();
            hbox1->addWidget(new QLabel("auto-downsample"));
            hbox1->addWidget(b256_checker);
            hbox1->addWidget(new QLabel("use GSDT"));
            hbox1->addWidget(isgsdt_checker);
            hbox1->addWidget(new QLabel("allow gap"));
            hbox1->addWidget(iswb_checker);
            hbox1->addWidget(new QLabel("radius from 2D?"));
            hbox1->addWidget(b_radius2Dchecker);

            QHBoxLayout * hbox2 = new QHBoxLayout();
            hbox2->addWidget(new QLabel("auto-resample SWC"));
            hbox2->addWidget(bresample_Checker);
            hbox2->addWidget(new QLabel("high intensity background"));
            hbox2->addWidget(b_intensity_Checker);
            hbox2->addWidget(new QLabel("bright filed"));
            hbox2->addWidget(b_brightfiled_Checker);

            layout->addLayout(hbox1,2,0,1,6);
            layout->addLayout(hbox2,3,0,1,6);

            
            layout->addWidget(new QLabel("cnn_type"),4,0);
            layout->addWidget(cnntype_spinbox, 4,1,1,5);
            layout->addWidget(new QLabel("length_thresh"),5,0);
            layout->addWidget(lenthresh_editor, 5,1,1,5);
            layout->addWidget(new QLabel("SR_ratio"),6,0);
            layout->addWidget(srratio_editor, 6,1,1,5);
            
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
            
            //run the dialog
            
            if(dialog->exec() != QDialog::Accepted)
                return false;
            
            //get the dialog return values
            channel = channel_spinbox->value() - 1;
            cnn_type = cnntype_spinbox->value();
            bkg_thresh = bkgthresh_spinbox->value();
            length_thresh = atof(lenthresh_editor->text().toStdString().c_str());
            SR_ratio = atof(srratio_editor->text().toStdString().c_str());
            is_gsdt = isgsdt_checker->isChecked();
            is_break_accept = iswb_checker->isChecked();
            b_256cube = b256_checker->isChecked();
            b_RadiusFrom2D = b_radius2Dchecker->isChecked();
            b_resample = bresample_Checker->isChecked();
            b_intensity = b_intensity_Checker->isChecked();
            b_brightfiled = b_brightfiled_Checker->isChecked();
            
            if (dialog) {delete dialog; dialog=0;}
        }
        
        return true;
    }
    
    bool fetch_para_commandline(const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

bool proc_app2(V3DPluginCallback2 &callback, PARA_APP2 &p, const QString & versionStr);

bool proc_app3(V3DPluginCallback2 &callback, PARA_APP2 &p, const QString & versionStr);

template <class T>
bool computeCubePcaEigVec(T* data1d, V3DLONG* sz,
                          V3DLONG x0, V3DLONG y0, V3DLONG z0,
                          V3DLONG wx, V3DLONG wy, V3DLONG wz,
                          double &pc1, double &pc2, double &pc3,
                          double *vec1, double *vec2, double *vec3){
    V3DLONG xb = x0 - wx; if(xb<0) xb = 0; if(xb >= sz[0]) xb = sz[0] - 1;
    V3DLONG xe = x0 + wx; if(xe<0) xe = 0; if(xe >= sz[0]) xe = sz[0] - 1;
    V3DLONG yb = y0 - wy; if(yb<0) yb = 0; if(yb >= sz[1]) yb = sz[1] - 1;
    V3DLONG ye = y0 + wy; if(ye<0) ye = 0; if(ye >= sz[1]) ye = sz[1] - 1;
    V3DLONG zb = z0 - wz; if(zb<0) zb = 0; if(zb >= sz[2]) zb = sz[2] - 1;
    V3DLONG ze = z0 + wz; if(ze<0) ze = 0; if(ze >= sz[2]) ze = sz[2] - 1;

    V3DLONG i,j,k;
    V3DLONG index;

    double w;

    //first get the center of mass
    double xm = 0, ym = 0, zm = 0, s = 0, mv = 0;
    for (k = zb; k <= ze; k++)
    {
        for (j = yb; j <= ye; j++)
        {
            for (i = xb; i <= xe; i++)
            {
                index = k*sz[0]*sz[1] + j*sz[0] + i;
                w = (double) data1d[index];
                xm += w*i;
                ym += w*j;
                zm += w*k;
                s += w;
            }
        }
    }

    if (s>0)
    {
        xm /= s; ym /= s; zm /= s;
        mv = s / ((double)((ze - zb + 1)*(ye - yb + 1)*(xe - xb + 1)));
    }
    else
    {
        printf("Sum of window pixels equals or is smaller than 0. The window is not valid or some other problems in the data. Do nothing.\n");
        return false;
    }

    double cc11 = 0, cc12 = 0, cc13 = 0, cc22 = 0, cc23 = 0, cc33 = 0;
    double dfx, dfy, dfz;
    for (k = zb; k <= ze; k++)
    {
        dfz = double(k) - zm;
        for (j = yb; j <= ye; j++)
        {
            dfy = double(j) - ym;
            for (i = xb; i <= xe; i++)
            {
                dfx = double(i) - xm;

                //                w = img3d[k][j][i]; //140128
                index = k*sz[0]*sz[1] + j*sz[0] + i;
                w = data1d[index] - mv;  if (w<0) w = 0; //140128 try the new formula

                cc11 += w*dfx*dfx;
                cc12 += w*dfx*dfy;
                cc13 += w*dfx*dfz;
                cc22 += w*dfy*dfy;
                cc23 += w*dfy*dfz;
                cc33 += w*dfz*dfz;
            }
        }
    }

    cc11 /= s; 	cc12 /= s; 	cc13 /= s; 	cc22 /= s; 	cc23 /= s; 	cc33 /= s;
//            if(count%1000 == 0){
//                qDebug()<<" cc11:"<<cc11<<" cc12:"<<cc12<<" cc13:"<<cc13<<" cc22:"<<cc22<<" cc23:"<<cc23<<" cc33:"<<cc33;
//            }



    try
    {
        //then find the eigen vector
        SymmetricMatrix Cov_Matrix(3);
        Cov_Matrix.Row(1) << cc11;
        Cov_Matrix.Row(2) << cc12 << cc22;
        Cov_Matrix.Row(3) << cc13 << cc23 << cc33;

        DiagonalMatrix DD;
        Matrix VV;
        EigenValues(Cov_Matrix, DD, VV);

        //output the result
        pc1 = DD(3);
        pc2 = DD(2);
        pc3 = DD(1);
//                qDebug()<<"pc1:"<<pc1<<"pc2:"<<pc2<<"pc3:"<<pc3;

        //output the vector
        for (int i = 1; i <= 3; i++){
            vec1[i-1] = VV(i, 3);
            vec2[i-1] = VV(i, 2);
            vec3[i-1] = VV(i, 1);
//                    qDebug()<<"vec1i:"<<vec1[i]<<"vec2i:"<<vec2[i]<<"vec3i:"<<vec3[i];
        }
    }catch (...)
    {
        pc1 = VAL_INVALID;
        pc2 = VAL_INVALID;
        pc3 = VAL_INVALID;
        return false;
    }

    return true;
}

#endif

