/* imPreProcess_plugin.h
 * This is a plugin for image enhancement.
 * 2020-12-10 : by S.GUO
 */
 
#ifndef __IMPREPROCESS_PLUGIN_H__
#define __IMPREPROCESS_PLUGIN_H__

#include <QtGui>
#include <QStringList>
#include <QCheckBox>
#include "v3d_interface.h"
#include "stackutil.h"
#include "helpFunc.h"
#include "fun_fft.h"
#include "morphology.h"
#include "bilateral_filter.h"

bool sigma_correction_domenu(V3DPluginCallback2 &callback, QWidget *parent);
bool sigma_correction_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);

bool subtract_minimum_domenu(V3DPluginCallback2 &callback, QWidget *parent);
bool subtract_minimum_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);

bool bilateral_filter_domenu(V3DPluginCallback2 &callback, QWidget *parent);
bool bilateral_filter_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);

bool fft_domenu(V3DPluginCallback2 &callback, QWidget *parent);
bool fft_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);

bool enhancement_domenu(V3DPluginCallback2 &callback, QWidget *parent);
bool enhancement_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);

bool grey_morphology_domenu(V3DPluginCallback2 &callback, QWidget *parent);
bool grey_morphology_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);

long quality_test_domenu(V3DPluginCallback2 &callback, QWidget *parent);
long quality_test_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);

long test_enhancement_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);

class TestPlugin : public QObject, public V3DPluginInterface2_1
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

//define a simple dialog for choose Gaussian Filter parameters
class SigmaCorrectionDialog : public QDialog
    {
        Q_OBJECT

    public:
        SigmaCorrectionDialog(V3DPluginCallback2 &cb, QWidget *parent)
        {
            //initialization of variables
            image = 0;
            gridLayout = 0;

            v3dhandleList win_list = cb.getImageWindowList();

            if(win_list.size()<1)
            {
                QMessageBox::information(0, "Morphology Operator", QObject::tr("No image is open."));
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

            cutoff_editor = new QLineEdit(QString("").setNum(25));
            gain_editor = new QLineEdit(QString("").setNum(5));
            channel_spinbox = new QSpinBox();
            channel_spinbox->setRange(1,chn_num);

            gridLayout->addWidget(new QLabel("Intensity cutoff (in percentile)"),0,0);
            gridLayout->addWidget(cutoff_editor, 0,1,1,5);
            gridLayout->addWidget(new QLabel("Intensity gain"),1,0);
            gridLayout->addWidget(gain_editor, 1,1,1,5);
            gridLayout->addWidget(new QLabel("Channel"),2,0);
            gridLayout->addWidget(channel_spinbox, 2,1,1,5);

            ok     = new QPushButton("OK");
            cancel = new QPushButton("Cancel");
            gridLayout->addWidget(cancel, 3,0);
            gridLayout->addWidget(ok,     3,1,1,5);;

            setLayout(gridLayout);
            setWindowTitle(QString("Sigma Correction"));

            //slot interface
            connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
            update();
        }

        ~SigmaCorrectionDialog(){}

        public slots:
        void update()
        {
            cutoff = atof(cutoff_editor->text().toStdString().c_str());
            gain = atof(gain_editor->text().toStdString().c_str());
            ch = channel_spinbox->text().toInt();
        }

    public:
        int ch;
        double cutoff, gain;
        Image4DSimple* image;
        ROIList pRoiList;
        QGridLayout *gridLayout;
        QPushButton* ok;
        QPushButton* cancel;
        QLineEdit * cutoff_editor;
        QLineEdit * gain_editor;
        QSpinBox * channel_spinbox;
    };

//define a simple dialog for choose Gaussian Filter parameters
class BilateralFilterDialog : public QDialog
    {
        Q_OBJECT

    public:
        BilateralFilterDialog(V3DPluginCallback2 &cb, QWidget *parent)
        {
            //initialization of variables
            image = 0;
            gridLayout = 0;

            v3dhandleList win_list = cb.getImageWindowList();

            if(win_list.size()<1)
            {
                QMessageBox::information(0, "Bilateral Filter", QObject::tr("No image is open."));
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

            wx_editor = new QLineEdit(QString("").setNum(3));
            wy_editor = new QLineEdit(QString("").setNum(3));
            wz_editor = new QLineEdit(QString("").setNum(1));
            sigma_editor = new QLineEdit(QString("").setNum(35));
            channel_spinbox = new QSpinBox();
            channel_spinbox->setRange(1,chn_num);


            gridLayout->addWidget(new QLabel("Window size (# voxels) along x"),0,0);
            gridLayout->addWidget(wx_editor, 0,1,1,5);
            gridLayout->addWidget(new QLabel("Window size (# voxels) along y"),1,0);
            gridLayout->addWidget(wy_editor, 1,1,1,5);
            gridLayout->addWidget(new QLabel("Window size (# voxels) along z"),2,0);
            gridLayout->addWidget(wz_editor, 2,1,1,5);
            gridLayout->addWidget(new QLabel("Color sigma value"),3,0);
            gridLayout->addWidget(sigma_editor, 3,1,1,5);
            gridLayout->addWidget(new QLabel("Channel"),4,0);
            gridLayout->addWidget(channel_spinbox, 4,1,1,5);

            ok     = new QPushButton("OK");
            cancel = new QPushButton("Cancel");
            gridLayout->addWidget(cancel, 5,0);
            gridLayout->addWidget(ok,     5,1,1,5);;

            setLayout(gridLayout);
            setWindowTitle(QString("Bilateral Filter"));

            //slot interface
            connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
            update();
        }

        ~BilateralFilterDialog(){}

        public slots:
        void update()
        {
            Wx = atof(wx_editor->text().toStdString().c_str());
            Wy = atof(wy_editor->text().toStdString().c_str());
            Wz = atof(wz_editor->text().toStdString().c_str());
            colorSigma = atof(sigma_editor->text().toStdString().c_str());
            ch = channel_spinbox->text().toInt();
        }

    public:
        int Wx,Wy,Wz,ch;
        double colorSigma;
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

//define a simple dialog for choose Gaussian Filter parameters
class MorphologyDialog : public QDialog
    {
        Q_OBJECT

    public:
        MorphologyDialog(V3DPluginCallback2 &cb, QWidget *parent)
        {
            //initialization of variables
            image = 0;
            gridLayout = 0;

            v3dhandleList win_list = cb.getImageWindowList();

            if(win_list.size()<1)
            {
                QMessageBox::information(0, "Morphology Operator", QObject::tr("No image is open."));
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

            wx_editor = new QLineEdit(QString("").setNum(11));
            wy_editor = new QLineEdit(QString("").setNum(11));
            channel_spinbox = new QSpinBox();
            channel_spinbox->setRange(1,chn_num);
            operator_combobox = new QComboBox();

            QStringList operators;
            operators << "Tophat" << "Erosion" <<"Dilation";
            operator_combobox->addItems(operators);
            operator_combobox->setCurrentIndex(0);

            gridLayout->addWidget(new QLabel("Window size (# voxels) along x"),0,0);
            gridLayout->addWidget(wx_editor, 0,1,1,5);
            gridLayout->addWidget(new QLabel("Window size (# voxels) along y"),1,0);
            gridLayout->addWidget(wy_editor, 1,1,1,5);
            gridLayout->addWidget(new QLabel("Channel"),2,0);
            gridLayout->addWidget(channel_spinbox, 2,1,1,5);
            gridLayout->addWidget(new QLabel("Operator"),3,0);
            gridLayout->addWidget(operator_combobox, 3,1,1,5);

            ok     = new QPushButton("OK");
            cancel = new QPushButton("Cancel");
            gridLayout->addWidget(cancel, 4,0);
            gridLayout->addWidget(ok,     4,1,1,5);;

            setLayout(gridLayout);
            setWindowTitle(QString("Grey Morphology Operator"));

            //slot interface
            connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
            update();
        }

        ~MorphologyDialog(){}

        public slots:
        void update()
        {
            Wx = atof(wx_editor->text().toStdString().c_str());
            Wy = atof(wy_editor->text().toStdString().c_str());
            ch = channel_spinbox->text().toInt();
            opt = operator_combobox->currentIndex();
        }

    public:
        int Wx,Wy,ch, opt;
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
        QComboBox * operator_combobox;
    };

//define a simple dialog for choose Gaussian Filter parameters
class PreProcessDialog : public QDialog
    {
        Q_OBJECT

    public:
        PreProcessDialog(V3DPluginCallback2 &cb, QWidget *parent)
        {
            //initialization of variables
            image = 0;
            gridLayout = 0;

            v3dhandleList win_list = cb.getImageWindowList();

            if(win_list.size()<1)
            {
                QMessageBox::information(0, "Image pre-processing", QObject::tr("No image is open."));
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

            wxy_bf_editor = new QLineEdit(QString("").setNum(3));
            wz_bf_editor = new QLineEdit(QString("").setNum(1));
            sigma_editor = new QLineEdit(QString("").setNum(35));

            th_checkbox = new QCheckBox("do tophat");
            th_checkbox->setCheckState(Qt::Unchecked);
            wx_th_editor = new QLineEdit(QString("").setNum(11));
            wy_th_editor = new QLineEdit(QString("").setNum(11));

            cutoff_editor = new QLineEdit(QString("").setNum(25));
            gain_editor = new QLineEdit(QString("").setNum(5));

            channel_spinbox = new QSpinBox();
            channel_spinbox->setRange(1,chn_num);


            gridLayout->addWidget(new QLabel("Parameters of bilateral filter:"),0,0);
            gridLayout->addWidget(new QLabel("Window size (# voxels) along xy"),1,0);
            gridLayout->addWidget(wxy_bf_editor, 1,1,1,5);
            gridLayout->addWidget(new QLabel("Window size (# voxels) along z"),2,0);
            gridLayout->addWidget(wz_bf_editor, 2,1,1,5);
            gridLayout->addWidget(new QLabel("Color sigma value"),3,0);
            gridLayout->addWidget(sigma_editor, 3,1,1,5);

            gridLayout->addWidget(new QLabel("Parameters of tophat operator:"),4,0);
            gridLayout->addWidget(th_checkbox, 5,0,1,5);
            gridLayout->addWidget(new QLabel("Window size (# voxels) along x"),6,0);
            gridLayout->addWidget(wx_th_editor, 6,1,1,5);
            gridLayout->addWidget(new QLabel("Window size (# voxels) along y"),7,0);
            gridLayout->addWidget(wy_th_editor, 7,1,1,5);

            gridLayout->addWidget(new QLabel("Parameters of sigma correction:"),8,0);
            gridLayout->addWidget(new QLabel("Intensity cutoff (in percentile)"),9,0);
            gridLayout->addWidget(cutoff_editor, 9,1,1,5);
            gridLayout->addWidget(new QLabel("Intensity gain"),10,0);
            gridLayout->addWidget(gain_editor, 10,1,1,5);

            gridLayout->addWidget(new QLabel("Channel"),11,0);
            gridLayout->addWidget(channel_spinbox, 11,1,1,5);

            ok     = new QPushButton("OK");
            cancel = new QPushButton("Cancel");
            gridLayout->addWidget(cancel, 12,0);
            gridLayout->addWidget(ok,     12,1,1,5);;

            setLayout(gridLayout);
            setWindowTitle(QString("Pre-Processing"));

            //slot interface
            connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
            update();
        }

        ~PreProcessDialog(){}

        public slots:
        void update()
        {
            Wxy_bf = atof(wxy_bf_editor->text().toStdString().c_str());
            Wz_bf = atof(wz_bf_editor->text().toStdString().c_str());
            colorSigma = atof(sigma_editor->text().toStdString().c_str());

            b_do_th = th_checkbox->checkState();
            Wx_th = atof(wx_th_editor->text().toStdString().c_str());
            Wy_th = atof(wy_th_editor->text().toStdString().c_str());

            cutoff = atof(cutoff_editor->text().toStdString().c_str());
            gain = atof(gain_editor->text().toStdString().c_str());

            ch = channel_spinbox->text().toInt();
        }

    public:
        int Wxy_bf, Wz_bf, Wx_th, Wy_th, ch;
        double colorSigma, cutoff, gain;
        bool b_do_th;
        Image4DSimple* image;
        ROIList pRoiList;
        QGridLayout *gridLayout;
        QPushButton* ok;
        QPushButton* cancel;
        QLineEdit * wxy_bf_editor;
        QLineEdit * wz_bf_editor;
        QLineEdit * sigma_editor;
        QCheckBox * th_checkbox;
        QLineEdit * wx_th_editor;
        QLineEdit * wy_th_editor;
        QLineEdit * cutoff_editor;
        QLineEdit * gain_editor;
        QSpinBox * channel_spinbox;

    };


#endif

