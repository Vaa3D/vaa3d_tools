/* Guo_tracing_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2019-3-26 : by Guochanghao
 */
 
#ifndef __GUO_TRACING_PLUGIN_H__
#define __GUO_TRACING_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include "vn.h"
#include "vn_app1.h"
#include "vn_app2.h"

class Guo_tracingPlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

struct input_PARA: public PARA_VN
{
    QString inimg_file;
    V3DLONG channel;
    int bkg_thresh;
    double b_thresh;//missed branchPoint detection threshold
    double t_thresh;//missed tipPoint detection threshold

    bool b_menu;

    input_PARA()
    {
        inimg_file = "";
        channel = 0;
        bkg_thresh = 1;
        b_menu = true;
        b_thresh = 27;
        t_thresh = 5;
    }

    bool initialize_2(V3DPluginCallback2 &callback)//for fix landmarks bug
    {
        v3dhandle curwin = callback.currentImageWindow();
        p4dImage = callback.getImage(curwin);
        if (!p4dImage)
        {
            v3d_msg("The input image is NULL. Do nothing.");
            return false;
        }
        landmarks = callback.getLandmark(curwin);
        V3DLONG in_sz0[4] = {p4dImage->getXDim(), p4dImage->getYDim(), p4dImage->getZDim(), p4dImage->getCDim()};

        V3DLONG view3d_datasz0, view3d_datasz1, view3d_datasz2;
        View3DControl * view3d = callback.getView3DControl(curwin);
        if (!view3d)
        {
            v3d_msg("The view3d pointer is NULL which indicates there is no 3D viewer window open, thus set the cut-box as max.",0);

            xc0 = 0;
            xc1 = in_sz0[0]-1;

            yc0 = 0;
            yc1 = in_sz0[1]-1;

            zc0 = 0;
            zc1 = in_sz0[2]-1;
        }
        else
        {
            view3d_datasz0 = view3d->dataDim1();
            view3d_datasz1 = view3d->dataDim2();
            view3d_datasz2 = view3d->dataDim3();

            xc0 = int(double(view3d->xCut0()) * in_sz0[0] / view3d_datasz0 + 0.5);
            xc1 = int(double(view3d->xCut1()) * in_sz0[0] / view3d_datasz0 + 0.5);
            if (xc1>in_sz0[0]-1) xc1 = in_sz0[0]-1;
            //xc1 = in_sz0[0]-1;//for debug purpose. 130102

            yc0 = int(double(view3d->yCut0()) * in_sz0[1] / view3d_datasz1 + 0.5);
            yc1 = int(double(view3d->yCut1()) * in_sz0[1] / view3d_datasz1 + 0.5);
            if (yc1>in_sz0[1]-1) yc1 = in_sz0[1]-1;

            zc0 = int(double(view3d->zCut0()) * in_sz0[2] / view3d_datasz2 + 0.5);
            zc1 = int(double(view3d->zCut1()) * in_sz0[2] / view3d_datasz2 + 0.5);
            if (zc1>in_sz0[2]-1) zc1 = in_sz0[2]-1;

            //printf("%5.3f, %5.3f, %5.3f\n", float(view3d->xCut1()), float(in_sz0[0]), float(view3d_datasz0));
        }

        if(landmarks.size()>0)
        {
            LocationSimple t;
            for(int i = 0; i < landmarks.size(); i++)
            {
                t.x = landmarks[i].x;
                t.y = landmarks[i].y;
                t.z = landmarks[i].z;

                if(t.x<xc0+1 || t.x>xc1+1 || t.y<yc0+1 || t.y>yc1+1 || t.z<zc0+1 || t.z>zc1+1)//fix bug by guochanghao in 20190420
                {

                    if(t.x<xc0+1)landmarks[i].x=xc0+1;
                    if(t.y<yc0+1)landmarks[i].y=yc0+1;
                    if(t.z<zc0+1)landmarks[i].z=zc0+1;

                    if(t.x>xc1+1)landmarks[i].x=xc1+1;
                    if(t.y>yc1+1)landmarks[i].y=yc1+1;
                    if(t.z>zc1+1)landmarks[i].z=zc1+1;

                }
            }


        }
        return true;
    }

    bool dialog()
    {
        if (!p4dImage || !p4dImage->valid())
            return false;

        int chn_num = p4dImage->getCDim();
        // fetch parameters from dialog
        {
            //set update the dialog
            QDialog * dialog = new QDialog();
            dialog->setWindowTitle("Guo_tracing v1 LinkPoints followed by assumed order");
            QGridLayout * layout = new QGridLayout();

            QSpinBox * channel_spinbox = new QSpinBox();
            channel_spinbox->setRange(1,chn_num);
            channel_spinbox->setValue(1);

            QSpinBox * bkgthresh_spinbox = new QSpinBox();
            bkgthresh_spinbox->setRange(-1, 255);
            bkgthresh_spinbox->setValue(bkg_thresh);

            QSpinBox * b_thresh_spinbox = new QSpinBox();
            b_thresh_spinbox->setRange(0, 40);
            b_thresh_spinbox->setValue(b_thresh);

            QSpinBox * t_thresh_spinbox = new QSpinBox();
            t_thresh_spinbox->setRange(0, 40);
            t_thresh_spinbox->setValue(t_thresh);


            layout->addWidget(new QLabel("color channel"),0,0);
            layout->addWidget(channel_spinbox, 0,1,1,5);
            layout->addWidget(new QLabel("background_threshold \n  (if set as -1, then auto-thresholding)"),1,0);
            layout->addWidget(bkgthresh_spinbox, 1,1,1,5);
            layout->addWidget(new QLabel("missed branchPoint detection threshold"),2,0);
            layout->addWidget(b_thresh_spinbox,2,1,1,5);
            layout->addWidget(new QLabel("missed tipPoint detection threshold"),3,0);
            layout->addWidget(t_thresh_spinbox,3,1,1,5);


            QHBoxLayout * hbox1 = new QHBoxLayout();
            QPushButton * ok = new QPushButton(" ok ");
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox1->addWidget(cancel);
            hbox1->addWidget(ok);

            layout->addLayout(hbox1,4,0,1,6);
            dialog->setLayout(layout);
            QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
            QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

            //run the dialog

            if(dialog->exec() != QDialog::Accepted)
                return false;

            //get the dialog return values
            channel = channel_spinbox->value() - 1;
            bkg_thresh = bkgthresh_spinbox->value();
            b_thresh = b_thresh_spinbox->value();
            t_thresh = t_thresh_spinbox->value();


            if (dialog) {delete dialog; dialog=0;}
        }

        return true;
    }
};

int LinkOnePointToSoma(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &p);
int reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA);
int tipTracingAddingBranch(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA);
int missedTipsTracing(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &p);
int getMaskImg(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA);
int maskImgTracing(V3DPluginCallback2 &callback, QWidget *parent, PARA_APP2 &p);
int gray_scale_stretching(V3DPluginCallback2 &callback, QWidget *parent);
int test(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA);



#endif

