/* neurontracing_region_app2_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2015-01-16 : by Zhi Zhou
 */
 
#ifndef __NEURONTRACING_REGION_NEURON2_PLUGIN_H__
#define __NEURONTRACING_REGION_NEURON2_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class neurontracing_region_neuron2 : public QObject, public V3DPluginInterface2_1
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

class regionAPP1Dialog : public QDialog
    {
        Q_OBJECT

    public:
        regionAPP1Dialog(V3DPluginCallback2 &cb, QWidget *parent)
        {
            image = 0;

            v3dhandle curwin = cb.currentImageWindow();
            if (!curwin)
            {
                v3d_msg("You don't have any images open in the main window.");
                return;
            }

            image = cb.getImage(curwin);

            if (!image)
            {
                v3d_msg("The image pointer is invalid. Ensure your data is valid and try again!");
                return;
            }

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

            region_editor = new QLineEdit(QString("").setNum(10000));

            layout->addWidget(new QLabel("color channel"),0,0);
            layout->addWidget(channel_spinbox, 0,1,1,5);
            layout->addWidget(new QLabel("background_threshold \n"),1,0);
            layout->addWidget(bkgthresh_spinbox, 1,1,1,5);
            layout->addWidget(new QLabel("visibility threshold \n  (normally do not need to change)"),2,0);
            layout->addWidget(visthresh_spinbox, 2,1,1,5);
            layout->addWidget(new QLabel("region_number"),3,0);
            layout->addWidget(region_editor, 3,1,1,5);

            QHBoxLayout * hbox1 = new QHBoxLayout();
            hbox1->addWidget(new QLabel("downsample factor \n  (set 0 for auto-downsample)"));
            hbox1->addWidget(downsample_spinbox);

            layout->addLayout(hbox1,4,0,1,6);

            QHBoxLayout * hbox2 = new QHBoxLayout();
            QPushButton * ok = new QPushButton(" ok ");
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox2->addWidget(cancel);
            hbox2->addWidget(ok);

            layout->addLayout(hbox2,4,0,1,6);
            setLayout(layout);
            setWindowTitle(QString("Neuron Tracing based on Different Regions using APP1"));

            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));


            connect(channel_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(bkgthresh_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));

            connect(downsample_spinbox, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(visthresh_spinbox, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(region_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));


            update();
        }

        ~regionAPP1Dialog(){}

        public slots:
        void update()
        {
            channel = channel_spinbox->value();
            bkg_thresh = bkgthresh_spinbox->value();
            visible_thresh = visthresh_spinbox->value();
            b_256cube = (downsample_spinbox->value()==0) ? 1 : 0;
            region_number = region_editor->text().toInt();

        }
    public:

        QSpinBox * channel_spinbox;
        QSpinBox * bkgthresh_spinbox;
        QSpinBox * downsample_spinbox;
        QSpinBox * visthresh_spinbox;
        QLineEdit * region_editor;

        Image4DSimple* image;

       // LandmarkList listLandmarks;
        int channel;
        int bkg_thresh;
        int visible_thresh;
        int b_256cube;
        int region_number;

    };

//define a simple dialog for choose NEURON2 parameters and TC file
class regionAPP2Dialog : public QDialog
    {
        Q_OBJECT

    public:
        regionAPP2Dialog(V3DPluginCallback2 &cb, QWidget *parent)
        {
            image = 0;

            v3dhandle curwin = cb.currentImageWindow();
            if (!curwin)
            {
                v3d_msg("You don't have any images open in the main window.");
                return;
            }

            image = cb.getImage(curwin);

            if (!image)
            {
                v3d_msg("The image pointer is invalid. Ensure your data is valid and try again!");
                return;
            }

            QGridLayout * layout = new QGridLayout();

            channel_spinbox = new QSpinBox();
            channel_spinbox->setRange(1,image->getCDim());
            channel_spinbox->setValue(1);
            cnntype_spinbox = new QSpinBox();
            cnntype_spinbox->setRange(1,3);
            cnntype_spinbox->setValue(2);
            bkgthresh_spinbox = new QSpinBox();
            bkgthresh_spinbox->setRange(-1, 255);
            bkgthresh_spinbox->setValue(10);
            lenthresh_editor = new QLineEdit(QString("").setNum(20));
            srratio_editor = new QLineEdit(QString("").setNum(3.0/9.0));
            region_editor = new QLineEdit(QString("").setNum(10000));
            isgsdt_checker = new QCheckBox();
            isgsdt_checker->setChecked(false);
            iswb_checker = new QCheckBox();
            iswb_checker->setChecked(false);
            b256_checker = new QCheckBox();
            b256_checker->setChecked(false);
            b_radius2Dchecker = new QCheckBox();
            b_radius2Dchecker->setChecked(true);

            layout->addWidget(new QLabel("color channel"),0,0);
            layout->addWidget(channel_spinbox, 0,1,1,5);
            layout->addWidget(new QLabel("background_threshold"),1,0);
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

            layout->addLayout(hbox1,2,0,1,6);

            layout->addWidget(new QLabel("cnn_type"),3,0);
            layout->addWidget(cnntype_spinbox, 3,1,1,5);
            layout->addWidget(new QLabel("length_thresh"),4,0);
            layout->addWidget(lenthresh_editor, 4,1,1,5);
            layout->addWidget(new QLabel("SR_ratio"),5,0);
            layout->addWidget(srratio_editor, 5,1,1,5);
            layout->addWidget(new QLabel("region_number"),6,0);
            layout->addWidget(region_editor, 6,1,1,5);

            QHBoxLayout * hbox2 = new QHBoxLayout();
            QPushButton * ok = new QPushButton(" ok ");
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox2->addWidget(cancel);
            hbox2->addWidget(ok);

            layout->addLayout(hbox2,7,0,7,6);
            setLayout(layout);
            setWindowTitle(QString("Neuron Tracing based on Different Regions using APP2 "));


            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

            connect(channel_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(cnntype_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(bkgthresh_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));

            connect(lenthresh_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(srratio_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(region_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));

            connect(isgsdt_checker, SIGNAL(stateChanged(int)), this, SLOT(update()));
            connect(iswb_checker, SIGNAL(stateChanged(int)), this, SLOT(update()));
            connect(b256_checker, SIGNAL(stateChanged(int)), this, SLOT(update()));
            connect(b_radius2Dchecker, SIGNAL(stateChanged(int)), this, SLOT(update()));

            update();
        }

        ~regionAPP2Dialog(){}

        public slots:
        void update()
        {
            channel = channel_spinbox->value();
            cnn_type = cnntype_spinbox->value();
            bkg_thresh = bkgthresh_spinbox->value();

            length_thresh = lenthresh_editor->text().toInt();
            SR_ratio = srratio_editor->text().toInt();
            region_number = region_editor->text().toInt();

            isgsdt_checker->isChecked()? is_gsdt = 1 : is_gsdt = 0;
            iswb_checker->isChecked()? is_break_accept = 1 : is_break_accept = 0;
            b256_checker->isChecked()? b_256cube = 1 : b_256cube = 0;
            b_radius2Dchecker->isChecked() ? b_RadiusFrom2D = 1 : b_RadiusFrom2D = 0;

        }
    public:

        QSpinBox * channel_spinbox;
        QSpinBox * cnntype_spinbox;
        QSpinBox * bkgthresh_spinbox;
        QLineEdit * lenthresh_editor;
        QLineEdit * srratio_editor;
        QLineEdit * region_editor;
        QCheckBox * isgsdt_checker;
        QCheckBox * iswb_checker;
        QCheckBox * b256_checker;
        QCheckBox * b_radius2Dchecker;


        Image4DSimple* image;
        int is_gsdt;
        int is_break_accept;
        int  bkg_thresh;
        double length_thresh;
        int  cnn_type;
        int  channel;
        double SR_ratio;
        int  b_256cube;
        int b_RadiusFrom2D;
        int region_number;

    };

#endif
