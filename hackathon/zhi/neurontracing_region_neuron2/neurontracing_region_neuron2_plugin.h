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
            downsample_spinbox->setRange(0, 255);
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

            connect(downsample_spinbox, SIGNAL(valueChanged (int)), this, SLOT(update()));
            connect(visthresh_spinbox, SIGNAL(valueChanged (int)), this, SLOT(update()));
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

            length_thresh = lenthresh_editor->text().toFloat();
            SR_ratio = srratio_editor->text().toFloat();
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

class regionADVANTRADialog : public QDialog
    {
        Q_OBJECT

    public:
        regionADVANTRADialog(V3DPluginCallback2 &cb, QWidget *parent)
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

            scale_editor = new QLineEdit(QString("").setNum(10));
            background_ratio_editor = new QLineEdit(QString("").setNum(0.5));
            correlation_thresh_editor = new QLineEdit(QString("").setNum(0.75));
            nr_directions_editor = new QLineEdit(QString("").setNum(20));
            angular_sigma_editor = new QLineEdit(QString("").setNum(60));
            nr_iters_editor = new QLineEdit(QString("").setNum(5));
            nr_states_editor = new QLineEdit(QString("").setNum(5));
            z_layer_dist_editor = new QLineEdit(QString("").setNum(1));
            bkgthresh_spinbox = new QSpinBox();
            bkgthresh_spinbox->setRange(-1, 255);
            bkgthresh_spinbox->setValue(10);
            region_editor = new QLineEdit(QString("").setNum(10000));

            layout->addWidget(new QLabel("scale pix (5, 20]"),0,0);
            layout->addWidget(scale_editor, 0,1,1,5);
            layout->addWidget(new QLabel("background ratio (0, 1]"), 1, 0);
            layout->addWidget(background_ratio_editor, 1, 1, 1, 5);
            layout->addWidget(new QLabel("correlation threshold [0.5, 1.0)"),2,0);
            layout->addWidget(correlation_thresh_editor, 2,1,1,5);
            layout->addWidget(new QLabel("nr directions [5, 20]"),3,0);
            layout->addWidget(nr_directions_editor, 3,1,1,5);
            layout->addWidget(new QLabel("angular sigma degs [20, 90]"),4,0);
            layout->addWidget(angular_sigma_editor, 4,1,1,5);
            layout->addWidget(new QLabel("nr iterations [20, 50]"),5,0);
            layout->addWidget(nr_iters_editor, 5,1,1,5);
            layout->addWidget(new QLabel("nr states [1, 20]"),6,0);
            layout->addWidget(nr_states_editor, 6,1,1,5);
            layout->addWidget(new QLabel("z layer dist pixels [1, 4]"),7,0);
            layout->addWidget(z_layer_dist_editor, 7,1,1,5);
            layout->addWidget(new QLabel("background threshold"),8,0);
            layout->addWidget(bkgthresh_spinbox,8,1,1,5);
            layout->addWidget(new QLabel("region number"),9,0);
            layout->addWidget(region_editor,9,1,1,5);

            QHBoxLayout * hbox = new QHBoxLayout();
            QPushButton * ok = new QPushButton(" ok ");
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox->addWidget(cancel);
            hbox->addWidget(ok);

            layout->addLayout(hbox,10,0,1,6);
            setLayout(layout);
            setWindowTitle(QString("Neuron Tracing based on Different Regions using Advantra"));

            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

            connect(scale_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(background_ratio_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(correlation_thresh_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(nr_directions_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(angular_sigma_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(nr_iters_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(nr_states_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(z_layer_dist_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(region_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));

            connect(bkgthresh_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));


            update();
        }

        ~regionADVANTRADialog(){}

        public slots:
        void update()
        {
            scale = scale_editor->text().toInt();
            bkg_ratio = background_ratio_editor->text().toFloat();
            correlation_thresh = correlation_thresh_editor->text().toFloat();
            nr_dirs = nr_directions_editor->text().toInt();
            angular_sigma = angular_sigma_editor->text().toFloat();
            nr_iters = nr_iters_editor->text().toInt();
            nr_states = nr_states_editor->text().toInt();
            z_dist = z_layer_dist_editor->text().toFloat();
            bkgd_thresh = bkgthresh_spinbox->value();
            region_num = region_editor->text().toInt();
        }
    public:

        QLineEdit * scale_editor;
        QLineEdit * background_ratio_editor;
        QLineEdit * correlation_thresh_editor;
        QLineEdit * nr_directions_editor;
        QLineEdit * angular_sigma_editor;
        QLineEdit * nr_iters_editor;
        QLineEdit * nr_states_editor;
        QLineEdit * z_layer_dist_editor;
        QSpinBox * bkgthresh_spinbox;
        QLineEdit * region_editor;


        Image4DSimple* image;

        int scale;
        double bkg_ratio;
        double correlation_thresh;
        int nr_dirs;
        double angular_sigma;
        int nr_iters;
        int nr_states;
        double z_dist;
        int bkgd_thresh;
        int region_num;

    };

class regionMSTDialog : public QDialog
    {
        Q_OBJECT

    public:
        regionMSTDialog(V3DPluginCallback2 &cb, QWidget *parent)
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
            bkgthresh_spinbox = new QSpinBox();
            bkgthresh_spinbox->setRange(-1, 255);
            bkgthresh_spinbox->setValue(10);
            region_editor = new QLineEdit(QString("").setNum(10000));
            window_size_spinbox = new QSpinBox();
            window_size_spinbox->setRange(1, 1000);
            window_size_spinbox->setValue(10);


            layout->addWidget(new QLabel("Background Threshold"),0,0);
            layout->addWidget(bkgthresh_spinbox,0,1,1,5);
            layout->addWidget(new QLabel("Region Number"), 1, 0);
            layout->addWidget(region_editor,1,1,1,5);
            layout->addWidget(new QLabel("Window Size"),2,0);
            layout->addWidget(window_size_spinbox,2,1,1,5);

            QHBoxLayout * hbox = new QHBoxLayout();
            QPushButton * ok = new QPushButton(" ok ");
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox->addWidget(cancel);
            hbox->addWidget(ok);

            layout->addLayout(hbox,3,0,1,6);
            setLayout(layout);
            setWindowTitle(QString("Neuron Tracing based on Different Regions using MST"));

            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

            connect(bkgthresh_spinbox, SIGNAL(valueChanged (int)), this, SLOT(update()));
            connect(region_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(window_size_spinbox, SIGNAL(valueChanged (int)), this, SLOT(update()));

            update();

        }

        ~regionMSTDialog(){}

        public slots:
        void update()
        {
            bkgd_thresh = bkgthresh_spinbox->value();
            region_num = region_editor->text().toInt();
            window_sz = window_size_spinbox->value();
        }

    public:

        QSpinBox * bkgthresh_spinbox;
        QLineEdit * region_editor;
        QSpinBox * window_size_spinbox;

        Image4DSimple* image;

        int bkgd_thresh;
        int region_num;
        int window_sz;

    };

class regionNCDialog : public QDialog
    {
        Q_OBJECT

    public:
        regionNCDialog(V3DPluginCallback2 &cb, QWidget *parent)
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
            bkgthresh_spinbox = new QSpinBox();
            bkgthresh_spinbox->setRange(-1, 255);
            bkgthresh_spinbox->setValue(10);
            region_editor = new QLineEdit(QString("").setNum(10000));
            channel_editor = new QLineEdit(QString("").setNum(1));
            scale_editor = new QLineEdit(QString("").setNum(12));
            corr_thresh_editor = new QLineEdit(QString("").setNum(.6));
            nr_dirs_spinbox = new QLineEdit(QString("").setNum(15));
            angsig_editor = new QLineEdit(QString("").setNum(60));
            nr_iter_spinbox = new QSpinBox();
            nr_iter_spinbox->setRange(2, 50);
            nr_iter_spinbox->setValue(30);
            nr_states_spinbox = new QSpinBox();
            nr_states_spinbox->setRange(1, 20);
            nr_states_spinbox->setValue(5);
            z_dist_editor = new QLineEdit(QString("").setNum(1));
            save_spinbox = new QSpinBox();
            save_spinbox->setRange(0, 1);
            save_spinbox->setValue(0);



            layout->addWidget(new QLabel("Background Threshold"),0,0);
            layout->addWidget(bkgthresh_spinbox,0,1,1,5);
            layout->addWidget(new QLabel("Region Number"), 1, 0);
            layout->addWidget(region_editor,1,1,1,5);
            layout->addWidget(new QLabel("Channel"),2,0);
            layout->addWidget(channel_editor,2,1,1,5);
            layout->addWidget(new QLabel("Scale pix (5, 20]"),3,0);
            layout->addWidget(scale_editor,3,1,1,5);
            layout->addWidget(new QLabel("Correlation Threshold [0.5, 1.0)"),4,0);
            layout->addWidget(corr_thresh_editor,4,1,1,5);
            layout->addWidget(new QLabel("nr. Directions"),5,0);
            layout->addWidget(nr_dirs_spinbox,5,1,1,5);
            layout->addWidget(new QLabel("Angular Sigma degrees [1, 60]"),6,0);
            layout->addWidget(angsig_editor,6,1,1,5);
            layout->addWidget(new QLabel("nr. Iterations"),7,0);
            layout->addWidget(nr_iter_spinbox,7,1,1,5);
            layout->addWidget(new QLabel("nr. States"),8,0);
            layout->addWidget(nr_states_spinbox,8,1,1,5);
            layout->addWidget(new QLabel("Z Layer Dist. pixels [1, 4]"), 9, 0);
            layout->addWidget(z_dist_editor,9,1,1,5);
            layout->addWidget(new QLabel("Save Midresults 0-No/1-Yes"),10,0);
            layout->addWidget(save_spinbox,10,1,1,5);

            QHBoxLayout * hbox = new QHBoxLayout();
            QPushButton * ok = new QPushButton(" ok ");
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox->addWidget(cancel);
            hbox->addWidget(ok);

            layout->addLayout(hbox,11,0,1,6);
            setLayout(layout);
            setWindowTitle(QString("Neuron Tracing based on Different Regions using NeuronChaser"));

            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

            connect(bkgthresh_spinbox, SIGNAL(valueChanged (int)), this, SLOT(update()));
            connect(region_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(channel_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(scale_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(corr_thresh_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(nr_dirs_spinbox, SIGNAL(valueChanged (int)), this, SLOT(update()));
            connect(angsig_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(nr_iter_spinbox, SIGNAL(valueChanged (int)), this, SLOT(update()));
            connect(nr_states_spinbox, SIGNAL(valueChanged (int)), this, SLOT(update()));
            connect(z_dist_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(save_spinbox, SIGNAL(valueChanged (int)), this, SLOT(update()));

            update();

        }

        ~regionNCDialog(){}

        public slots:
        void update()
        {
            bkgd_thresh = bkgthresh_spinbox->value();
            region_num = region_editor->text().toInt();
            channel = channel_editor->text().toInt();
            scale = scale_editor->text().toInt();
            corr_thresh = corr_thresh_editor->text().toFloat();
            nr_dirs = nr_dirs_spinbox->text().toInt();
            angular_sigma = angsig_editor->text().toFloat();
            nr_iters = nr_iter_spinbox->value();
            nr_states = nr_states_spinbox->value();
            z_dist = z_dist_editor->text().toFloat();
            save_midres = save_spinbox->value();
        }

    public:

        QSpinBox * bkgthresh_spinbox;
        QLineEdit * region_editor;
        QLineEdit * channel_editor;
        QLineEdit * scale_editor;
        QLineEdit * corr_thresh_editor;
        QLineEdit * nr_dirs_spinbox;
        QLineEdit * angsig_editor;
        QSpinBox * nr_iter_spinbox;
        QSpinBox * nr_states_spinbox;
        QLineEdit * z_dist_editor;
        QSpinBox * save_spinbox;

        Image4DSimple* image;

        int bkgd_thresh;
        int region_num;
        int channel;
        int scale;
        double corr_thresh;
        int nr_dirs;
        double angular_sigma;
        int nr_iters;
        int nr_states;
        double z_dist;\
        int save_midres;


    };

class regionAADialog : public QDialog
    {
        Q_OBJECT

    public:
        regionAADialog(V3DPluginCallback2 &cb, QWidget *parent)
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
            bkgthresh_spinbox = new QSpinBox();
            bkgthresh_spinbox->setRange(-1, 255);
            bkgthresh_spinbox->setValue(10);
            region_editor = new QLineEdit(QString("").setNum(10000));
            thresh_editor = new QLineEdit(QString("").setNum(998));
            trees_editor = new QLineEdit(QString("").setNum(15));

            layout->addWidget(new QLabel("Background Threshold"),0,0);
            layout->addWidget(bkgthresh_spinbox,0,1,1,5);
            layout->addWidget(new QLabel("Region Number"), 1, 0);
            layout->addWidget(region_editor,1,1,1,5);
            layout->addWidget(new QLabel("Threshold"),2,0);
            layout->addWidget(thresh_editor,2,1,1,5);
            layout->addWidget(new QLabel("Number of Trees"),3,0);
            layout->addWidget(trees_editor,3,1,1,5);

            QHBoxLayout * hbox = new QHBoxLayout();
            QPushButton * ok = new QPushButton(" ok ");
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox->addWidget(cancel);
            hbox->addWidget(ok);

            layout->addLayout(hbox,4,0,1,6);
            setLayout(layout);
            setWindowTitle(QString("Neuron Tracing based on Different Regions"));

            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

            connect(bkgthresh_spinbox, SIGNAL(valueChanged (int)), this, SLOT(update()));
            connect(region_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(thresh_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(trees_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));

            update();

        }

        ~regionAADialog(){}

        public slots:
        void update()
        {
            bkgd_thresh = bkgthresh_spinbox->value();
            region_num = region_editor->text().toInt();
            thresh = thresh_editor->text().toInt();
            num_trees = trees_editor->text().toInt();
        }

    public:

        QSpinBox * bkgthresh_spinbox;
        QLineEdit * region_editor;
        QLineEdit * thresh_editor;
        QLineEdit * trees_editor;

        Image4DSimple* image;

        int bkgd_thresh;
        int region_num;
        int thresh;
        int num_trees;

    };

class regionNCTUTWDialog : public QDialog
    {
        Q_OBJECT

    public:
        regionNCTUTWDialog(V3DPluginCallback2 &cb, QWidget *parent)
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
            bkgthresh_spinbox = new QSpinBox();
            bkgthresh_spinbox->setRange(-1, 255);
            bkgthresh_spinbox->setValue(10);
            region_editor = new QLineEdit(QString("").setNum(10000));
            thresh_editor = new QLineEdit(QString("").setNum(998));
            x_editor = new QLineEdit(QString("").setNum(-1));
            y_editor = new QLineEdit(QString("").setNum(-1));
            z_editor = new QLineEdit(QString("").setNum(-1));

            layout->addWidget(new QLabel("Background Threshold"),0,0);
            layout->addWidget(bkgthresh_spinbox,0,1,1,5);
            layout->addWidget(new QLabel("Region Number"), 1, 0);
            layout->addWidget(region_editor,1,1,1,5);
            layout->addWidget(new QLabel("Threshold"),2,0);
            layout->addWidget(thresh_editor,2,1,1,5);

            QBoxLayout * hbox_coord = new QHBoxLayout();
            hbox_coord->addWidget(new QLabel("x-coordinate"));
            hbox_coord->addWidget(x_editor);
            hbox_coord->addWidget(new QLabel("y-coordinate"));
            hbox_coord->addWidget(y_editor);
            hbox_coord->addWidget(new QLabel("z-coordinate"));
            hbox_coord->addWidget(z_editor);

            layout->addLayout(hbox_coord,3,0,1,6);

            QHBoxLayout * hbox = new QHBoxLayout();
            QPushButton * ok = new QPushButton(" ok ");
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox->addWidget(cancel);
            hbox->addWidget(ok);

            layout->addLayout(hbox,4,0,1,6);
            setLayout(layout);
            setWindowTitle(QString("Neuron Tracing based on Different Regions"));

            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

            connect(bkgthresh_spinbox, SIGNAL(valueChanged (int)), this, SLOT(update()));
            connect(region_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(thresh_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(x_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(y_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
            connect(z_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));


            update();

        }

        ~regionNCTUTWDialog(){}

        public slots:
        void update()
        {
            bkgd_thresh = bkgthresh_spinbox->value();
            region_num = region_editor->text().toInt();
            thresh = thresh_editor->text().toFloat();
            x = x_editor->text().toInt();
            y = y_editor->text().toInt();
            z = z_editor->text().toInt();
        }

    public:

        QSpinBox * bkgthresh_spinbox;
        QLineEdit * region_editor;
        QLineEdit * thresh_editor;
        QLineEdit * x_editor;
        QLineEdit * y_editor;
        QLineEdit * z_editor;


        Image4DSimple* image;

        int bkgd_thresh;
        int region_num;
        double thresh;
        int x;
        int y;
        int z;

    };

class regionBasicDialog : public QDialog
    {
        Q_OBJECT

    public:
        regionBasicDialog(V3DPluginCallback2 &cb, QWidget *parent)
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
            bkgthresh_spinbox = new QSpinBox();
            bkgthresh_spinbox->setRange(-1, 255);
            bkgthresh_spinbox->setValue(10);
            region_editor = new QLineEdit(QString("").setNum(10000));

            layout->addWidget(new QLabel("Background Threshold"),0,0);
            layout->addWidget(bkgthresh_spinbox,0,1,1,5);
            layout->addWidget(new QLabel("Region Number"), 1, 0);
            layout->addWidget(region_editor,1,1,1,5);

            QHBoxLayout * hbox = new QHBoxLayout();
            QPushButton * ok = new QPushButton(" ok ");
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox->addWidget(cancel);
            hbox->addWidget(ok);

            layout->addLayout(hbox,2,0,1,6);
            setLayout(layout);
            setWindowTitle(QString("Neuron Tracing based on Different Regions"));

            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

            connect(bkgthresh_spinbox, SIGNAL(valueChanged ()), this, SLOT(update()));
            connect(region_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));

            update();

        }

        ~regionBasicDialog(){}

        public slots:
        void update()
        {
            bkgd_thresh = bkgthresh_spinbox->value();
            region_num = region_editor->text().toInt();
        }

    public:

        QSpinBox * bkgthresh_spinbox;
        QLineEdit * region_editor;

        Image4DSimple* image;

        int bkgd_thresh;
        int region_num;

    };

#endif
