/* gaussianfilter.h
 * 2009-06-03: create this program by Yang Yu
 * 2009-08-14: change into plugin by Yang Yu
 */


#ifndef __GAUSSIANFILTER_H__
#define __GAUSSIANFILTER_H__

//     3D ANISOTROPIC GAUSSIAN FILTERING
//
//     pImage  - input & output
//     N       - width of the image
//     M       - height of the image
//     P       - slice of the image
//     Wx      - window size along x
//     Wy      - window size along y
//     Wz      - window size along z
//
//int GaussianFilter(float *pImage, V3DLONG N, V3DLONG M, V3DLONG P, unsigned int Wx, unsigned int Wy, unsigned int Wz);
//e.g. if(!GaussianFilter(pImg, sz[0], sz[1], sz[2], sigmax, sigmay, sigmaz))
//			cout<<"err"<<endl;
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class GaussianFilterPlugin : public QObject, public V3DPluginInterface2_1
{
     Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);

public:
    float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const;
     bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

//define a simple dialog for choose Gaussian Filter parameters
class GaussianFilterDialog : public QDialog
    {
        Q_OBJECT

    public:
        GaussianFilterDialog(V3DPluginCallback2 &cb, QWidget *parent)
        {
            //initialization of variables
            image = 0;
            gridLayout = 0;

            //

            v3dhandleList win_list = cb.getImageWindowList();

            if(win_list.size()<1)
            {
                QMessageBox::information(0, "Gaussian Filter", QObject::tr("No image is open."));
                return;
            }

            //create a dialog
            gridLayout = new QGridLayout();

            image = cb.getImage(cb.currentImageWindow());

            if (!image || !image->valid())
            {
                v3d_msg("The image is not valid yet. Check your data.");
                return;
            }

            pRoiList=cb.getROI(cb.currentImageWindow());
            int chn_num = image->getCDim();

            wx_editor = new QLineEdit(QString("").setNum(7));
            wy_editor = new QLineEdit(QString("").setNum(7));
            wz_editor = new QLineEdit(QString("").setNum(7));
            sigma_editor = new QLineEdit(QString("").setNum(3));
            channel_spinbox = new QSpinBox();
            channel_spinbox->setRange(1,chn_num);


            gridLayout->addWidget(new QLabel("Window size (# voxels) along x"),0,0);
            gridLayout->addWidget(wx_editor, 0,1,1,5);
            gridLayout->addWidget(new QLabel("Window size (# voxels) along y"),1,0);
            gridLayout->addWidget(wy_editor, 1,1,1,5);
            gridLayout->addWidget(new QLabel("Window size (# voxels) along z"),2,0);
            gridLayout->addWidget(wz_editor, 2,1,1,5);
            gridLayout->addWidget(new QLabel("Sigma value"),3,0);
            gridLayout->addWidget(sigma_editor, 3,1,1,5);
            gridLayout->addWidget(new QLabel("Channel"),4,0);
            gridLayout->addWidget(channel_spinbox, 4,1,1,5);

            ok     = new QPushButton("OK");
            cancel = new QPushButton("Cancel");
            gridLayout->addWidget(cancel, 5,0);
            gridLayout->addWidget(ok,     5,1,1,5);;

            setLayout(gridLayout);
            setWindowTitle(QString("Gaussian Filter"));

            //slot interface
            connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
            update();
        }

        ~GaussianFilterDialog(){}

        public slots:
        void update()
        {
            Wx = atof(wx_editor->text().toStdString().c_str());
            Wy = atof(wy_editor->text().toStdString().c_str());
            Wz = atof(wz_editor->text().toStdString().c_str());
            sigma = atof(sigma_editor->text().toStdString().c_str());
            ch = channel_spinbox->text().toInt();
        }

    public:
        int Wx,Wy,Wz,ch;
        double sigma;
        Image4DSimple* image;
        ROIList pRoiList;
        QGridLayout *gridLayout;
        QPushButton* ok;
        QPushButton* cancel;
        QLineEdit * wx_editor;
        QLineEdit * wy_editor;
        QLineEdit * wz_editor;
        QLineEdit * sigma_editor;
        QSpinBox * channel_spinbox;

    };


#endif



