/* neurontracer_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2016-2-16 : by Zhi Zhou
 */
 
#ifndef __NERUONCRAWLER_APP2_PLUGIN_H__
#define __NERUONCRAWLER_APP2_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class neurontracer : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
    float getPluginVersion() const {return 1.48f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

class neurontracer_app2_raw : public QDialog
    {
        Q_OBJECT

    public:
        neurontracer_app2_raw(V3DPluginCallback2 &cb, QWidget *parent)
        {
            image = 0;

            v3dhandle curwin = cb.currentImageWindow();
            if (curwin)
            {
                image = cb.getImage(curwin);
                listLandmarks = cb.getLandmark(curwin);
                if(listLandmarks.count() ==0)
                {
                    v3d_msg("No markers in the current image, please select a marker.");
                    return;
                }
            }

            QGridLayout * layout = new QGridLayout();
            channel_spinbox = new QSpinBox();
            channel_spinbox->setRange(1,3);
            channel_spinbox->setValue(1);
            cnntype_spinbox = new QSpinBox();
            cnntype_spinbox->setRange(1,3);
            cnntype_spinbox->setValue(2);
            bkgthresh_spinbox = new QSpinBox();
            bkgthresh_spinbox->setRange(-1, 255);
            bkgthresh_spinbox->setValue(13);
            lenthresh_editor = new QLineEdit(QString("").setNum(5));
         //   srratio_editor = new QLineEdit(QString("").setNum(3.0/9.0));
            isgsdt_checker = new QCheckBox();
            isgsdt_checker->setChecked(false);
            iswb_checker = new QCheckBox();
            iswb_checker->setChecked(false);
            b256_checker = new QCheckBox();
            b256_checker->setChecked(false);
            b_radius2Dchecker = new QCheckBox();
            b_radius2Dchecker->setChecked(true);

            block_spinbox = new QSpinBox();
            block_spinbox->setRange(1,2048);
            block_spinbox->setValue(1024);

            b_adapWinchecker = new QCheckBox();
            b_adapWinchecker->setChecked(true);

            b_3Dchecker = new QCheckBox();
            b_3Dchecker->setChecked(false);

            b_multiTracerchecker = new QCheckBox();
            b_multiTracerchecker->setChecked(false);

            raw_filepath = new QLineEdit();
            openrawFile = new QPushButton(QObject::tr("..."));

            terafly_filepath = new QLineEdit();
            openteraflyFile = new QPushButton(QObject::tr("..."));

            if(curwin)
            {
                raw_filepath->setText(cb.getImageName(curwin));
                raw_filepath->setDisabled(true);
                openrawFile->setDisabled(true);
                terafly_filepath->setDisabled(true);
                openteraflyFile->setDisabled(true);
            }

            marker_filepath = new QLineEdit();
            openmarkerFile = new QPushButton(QObject::tr("..."));
            if(curwin)
            {
                openmarkerFile->setDisabled(true);
                marker_filepath->setDisabled(true);
            }

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

            layout->addLayout(hbox1,2,0,1,6);

            layout->addWidget(new QLabel("cnn_type"),3,0);
            layout->addWidget(cnntype_spinbox, 3,1,1,5);
            layout->addWidget(new QLabel("length_thresh"),4,0);
            layout->addWidget(lenthresh_editor, 4,1,1,5);

            QHBoxLayout * hbox2 = new QHBoxLayout();
            hbox2->addWidget(new QLabel("adaptive size?"));
            hbox2->addWidget(b_adapWinchecker);
            hbox2->addWidget(new QLabel("3D crawler?"));
            hbox2->addWidget(b_3Dchecker);
            hbox2->addWidget(new QLabel("multiple tracers?"));
            hbox2->addWidget(b_multiTracerchecker);
//            hbox2->addWidget(new QLabel("allow gap"));
//            hbox2->addWidget(iswb_checker);
            layout->addLayout(hbox2,5,0,1,6);


//            layout->addWidget(new QLabel("SR_ratio"),5,0);
//            layout->addWidget(srratio_editor, 5,1,1,3);
//            layout->addWidget(new QLabel("3D crawler?"),5,0);
//            layout->addWidget(b_3Dchecker);
            layout->addWidget(new QLabel("block_size"),6,0);
            layout->addWidget(block_spinbox, 6,1,1,3);
//            layout->addWidget(new QLabel("adaptive size?"),5,2);
//            layout->addWidget(b_adapWinchecker);


            layout->addWidget(new QLabel(QObject::tr("va3draw/raw image or tc file:")),7,0);
            layout->addWidget(raw_filepath,7,1,1,4);
            layout->addWidget(openrawFile,7,5,1,1);

            layout->addWidget(new QLabel(QObject::tr("marker file:")),8,0);
            layout->addWidget(marker_filepath,8,1,1,4);
            layout->addWidget(openmarkerFile,8,5,1,1);

            layout->addWidget(new QLabel(QObject::tr("terafly format file:")),9,0);
            layout->addWidget(terafly_filepath,9,1,1,4);
            layout->addWidget(openteraflyFile,9,5,1,1);

            QHBoxLayout * hbox3 = new QHBoxLayout();
            QPushButton * ok = new QPushButton(" ok ");
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox3->addWidget(cancel);
            hbox3->addWidget(ok);

            layout->addLayout(hbox3,10,0,10,6);
            setLayout(layout);
            setWindowTitle(QString("UltraTracer_APP2"));


            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

            connect(channel_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(cnntype_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(bkgthresh_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));

            connect(lenthresh_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));
          //  connect(srratio_editor, SIGNAL(selectionChanged ()), this, SLOT(update()));

            connect(isgsdt_checker, SIGNAL(stateChanged(int)), this, SLOT(update()));
            connect(iswb_checker, SIGNAL(stateChanged(int)), this, SLOT(update()));
            connect(b256_checker, SIGNAL(stateChanged(int)), this, SLOT(update()));
            connect(b_radius2Dchecker, SIGNAL(stateChanged(int)), this, SLOT(update()));
            connect(b_adapWinchecker, SIGNAL(stateChanged(int)), this, SLOT(update()));
            connect(b_3Dchecker, SIGNAL(stateChanged(int)), this, SLOT(update()));
            connect(b_multiTracerchecker, SIGNAL(stateChanged(int)), this, SLOT(update()));


            connect(block_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(openrawFile, SIGNAL(clicked()), this, SLOT(_slots_openrawFile()));
            connect(openmarkerFile, SIGNAL(clicked()), this, SLOT(_slots_openmarkerFile()));
            connect(openteraflyFile, SIGNAL(clicked()), this, SLOT(_slots_openteraflyFile()));


            update();
        }

        ~neurontracer_app2_raw(){}

        public slots:
        void update()
        {
            channel = channel_spinbox->value();
            cnn_type = cnntype_spinbox->value();
            bkg_thresh = bkgthresh_spinbox->value();

            length_thresh = lenthresh_editor->text().toInt();
            SR_ratio = 0.333333;

            isgsdt_checker->isChecked()? is_gsdt = 1 : is_gsdt = 0;
            iswb_checker->isChecked()? is_break_accept = 1 : is_break_accept = 0;
            b256_checker->isChecked()? b_256cube = 1 : b_256cube = 0;
            b_radius2Dchecker->isChecked() ? b_RadiusFrom2D = 1 : b_RadiusFrom2D = 0;
            b_adapWinchecker->isChecked() ? adap_win = 1 : adap_win = 0;
            b_3Dchecker->isChecked() ? tracing_3D = 1 : tracing_3D = 0;
            b_multiTracerchecker->isChecked() ? tracing_comb = 1 : tracing_comb = 0;


            block_size = block_spinbox->value();
            rawfilename = raw_filepath->text();
            markerfilename = marker_filepath->text();
            teraflyfilename = terafly_filepath->text();

        }

        void _slots_openrawFile()
        {
            QFileDialog d(this);
            QString fileOpenName;
            fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open Raw File/TC File"),
                                                        "",
                                                        QObject::tr("Supported file (*.raw *.RAW *.V3DRAW *.v3draw *.tc)"
                                                            ));
            if(!fileOpenName.isEmpty())
            {
                raw_filepath->setText(fileOpenName);
            }
            update();

        }

        void _slots_openmarkerFile()
        {
            QFileDialog d(this);
            QString fileOpenName;
            fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open Marker/Point Cloud File"),
                                                        "",
                                                        QObject::tr("Supported file (*.marker *.MARKER *.apo *.APO)"
                                                            ));
            if(!fileOpenName.isEmpty())
            {
                marker_filepath->setText(fileOpenName);
            }
            update();

        }

        void _slots_openteraflyFile()
        {
            QFileDialog d(this);
            QString fileOpenName;
            fileOpenName = QFileDialog::getExistingDirectory(0, QObject::tr("Open Terafly File"),
                                                        "");
            if(!fileOpenName.isEmpty())
            {
                terafly_filepath->setText(fileOpenName);
            }
            update();

        }
    public:

        QSpinBox * channel_spinbox;
        QSpinBox * cnntype_spinbox;
        QSpinBox * bkgthresh_spinbox;
        QLineEdit * lenthresh_editor;
       // QLineEdit * srratio_editor;
        QCheckBox * isgsdt_checker;
        QCheckBox * iswb_checker;
        QCheckBox * b256_checker;
        QCheckBox * b_radius2Dchecker;
        QCheckBox * b_adapWinchecker;
        QCheckBox * b_3Dchecker;
        QCheckBox * b_multiTracerchecker;


        QSpinBox * block_spinbox;

        QLineEdit * raw_filepath;
        QLineEdit * terafly_filepath;
        QLineEdit * marker_filepath;
        QPushButton *openrawFile;
        QPushButton *openteraflyFile;
        QPushButton *openmarkerFile;

        Image4DSimple* image;
        LandmarkList listLandmarks;
        int is_gsdt;
        int is_break_accept;
        int  bkg_thresh;
        double length_thresh;
        int  cnn_type;
        int  channel;
        double SR_ratio;
        int  b_256cube;
        int b_RadiusFrom2D;
        int block_size;
        int adap_win;
        int tracing_3D;
        int tracing_comb;

        QString rawfilename;
        QString markerfilename;
        QString teraflyfilename;

    };

class neurontracer_app1_raw : public QDialog
    {
        Q_OBJECT

    public:
        neurontracer_app1_raw(V3DPluginCallback2 &cb, QWidget *parent)
        {
            image = 0;
            int chn_num = 3;
            v3dhandle curwin = cb.currentImageWindow();
            if (curwin)
            {
                image = cb.getImage(curwin);
                chn_num = image->getCDim();

                listLandmarks = cb.getLandmark(curwin);
                if(listLandmarks.count() ==0)
                {
                    v3d_msg("No markers in the current image, please select a marker.");
                    return;
                }
            }

            QGridLayout * layout = new QGridLayout();
            channel_spinbox = new QSpinBox();
            channel_spinbox->setRange(1,chn_num);
            channel_spinbox->setValue(1);

            bkgthresh_spinbox = new QSpinBox();
            bkgthresh_spinbox->setRange(-1, 255);
            bkgthresh_spinbox->setValue(20);

            visthresh_spinbox = new QSpinBox();
            visthresh_spinbox->setRange(1, 255);
            visthresh_spinbox->setValue(30);

            downsample_spinbox = new QSpinBox();
            downsample_spinbox->setRange(0, 10);
            downsample_spinbox->setValue(2);

            block_spinbox = new QSpinBox();
            block_spinbox->setRange(1,2048);
            block_spinbox->setValue(768);

            b_adapWinchecker = new QCheckBox();
            b_adapWinchecker->setChecked(true);

            raw_filepath = new QLineEdit();
            openrawFile = new QPushButton(QObject::tr("..."));
            if(curwin)
            {
                raw_filepath->setText(cb.getImageName(curwin));
                raw_filepath->setDisabled(true);
                openrawFile->setDisabled(true);
            }

            marker_filepath = new QLineEdit();
            openmarkerFile = new QPushButton(QObject::tr("..."));
            if(curwin)
            {
                openmarkerFile->setDisabled(true);
                marker_filepath->setDisabled(true);
            }

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

            layout->addWidget(new QLabel("block_size"),4,0);
            layout->addWidget(block_spinbox, 4,1,1,3);
            layout->addWidget(new QLabel("adaptive size?"),4,4);
            layout->addWidget(b_adapWinchecker);

            layout->addWidget(new QLabel(QObject::tr("va3draw/raw image or tc file:")),5,0);
            layout->addWidget(raw_filepath,5,1,1,4);
            layout->addWidget(openrawFile,5,5,1,1);

            layout->addWidget(new QLabel(QObject::tr("marker file:")),6,0);
            layout->addWidget(marker_filepath,6,1,1,4);
            layout->addWidget(openmarkerFile,6,5,1,1);

            QHBoxLayout * hbox2 = new QHBoxLayout();
            QPushButton * ok = new QPushButton(" ok ");
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox2->addWidget(cancel);
            hbox2->addWidget(ok);

            layout->addLayout(hbox2,7,0,7,6);
            setLayout(layout);
            setWindowTitle(QString("UltraTracer_APP1"));


            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

            connect(channel_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(bkgthresh_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(visthresh_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(downsample_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(b_adapWinchecker, SIGNAL(stateChanged(int)), this, SLOT(update()));

            connect(block_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(openrawFile, SIGNAL(clicked()), this, SLOT(_slots_openrawFile()));
            connect(openmarkerFile, SIGNAL(clicked()), this, SLOT(_slots_openmarkerFile()));

            update();
        }

        ~neurontracer_app1_raw(){}

        public slots:
        void update()
        {
            channel = channel_spinbox->value();
            bkg_thresh = bkgthresh_spinbox->value();
            block_size = block_spinbox->value();
            visible_thresh = visthresh_spinbox->value();

            rawfilename = raw_filepath->text();
            markerfilename = marker_filepath->text();
            downsample_factor = downsample_spinbox->value();

            b_adapWinchecker->isChecked() ? adap_win = 1 : adap_win = 0;

            b_256cube = (downsample_factor==0) ? 1 : 0;

        }

        void _slots_openrawFile()
        {
            QFileDialog d(this);
            QString fileOpenName;
            fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open Raw File/TC File"),
                                                        "",
                                                        QObject::tr("Supported file (*.raw *.RAW *.V3DRAW *.v3draw *.tc)"
                                                            ));
            if(!fileOpenName.isEmpty())
            {
                raw_filepath->setText(fileOpenName);
            }
            update();

        }

        void _slots_openmarkerFile()
        {
            QFileDialog d(this);
            QString fileOpenName;
            fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open Marker/Point Cloud File"),
                                                        "",
                                                        QObject::tr("Supported file (*.marker *.MARKER *.apo *.APO)"
                                                            ));
            if(!fileOpenName.isEmpty())
            {
                marker_filepath->setText(fileOpenName);
            }
            update();

        }
    public:

        QSpinBox * channel_spinbox;
        QSpinBox * bkgthresh_spinbox;
        QSpinBox * visthresh_spinbox;
        QSpinBox * downsample_spinbox;
        QCheckBox * b_adapWinchecker;

        QSpinBox * block_spinbox;


        QLineEdit * raw_filepath;
        QLineEdit * marker_filepath;
        QPushButton *openrawFile;
        QPushButton *openmarkerFile;

        Image4DSimple* image;
        LandmarkList listLandmarks;
        int  bkg_thresh;
        int  channel;
        int  visible_thresh;
        int  b_256cube;
        int block_size;
        int downsample_factor;
        int adap_win;

        QString rawfilename;
        QString markerfilename;

    };

class neurontracer_most_raw : public QDialog
    {
        Q_OBJECT

    public:
        neurontracer_most_raw(V3DPluginCallback2 &cb, QWidget *parent)
        {
            image = 0;
            int chn_num = 3;
            v3dhandle curwin = cb.currentImageWindow();
            if (curwin)
            {
                image = cb.getImage(curwin);
                chn_num = image->getCDim();

                listLandmarks = cb.getLandmark(curwin);
                if(listLandmarks.count() ==0)
                {
                    v3d_msg("No markers in the current image, please select a marker.");
                    return;
                }
            }

            QGridLayout * layout = new QGridLayout();
            channel_spinbox = new QSpinBox();
            channel_spinbox->setRange(1,chn_num);
            channel_spinbox->setValue(1);

            bkgthresh_spinbox = new QSpinBox();
            bkgthresh_spinbox->setRange(0, 255);
            bkgthresh_spinbox->setValue(20);

            seedwin_spinbox = new QSpinBox();
            seedwin_spinbox->setRange(1, 100);
            seedwin_spinbox->setValue(20);

            slipwin_spinbox = new QSpinBox();
            slipwin_spinbox->setRange(1, 100);
            slipwin_spinbox->setValue(20);

            block_spinbox = new QSpinBox();
            block_spinbox->setRange(1,2048);
            block_spinbox->setValue(768);

            b_adapWinchecker = new QCheckBox();
            b_adapWinchecker->setChecked(true);

            raw_filepath = new QLineEdit();
            openrawFile = new QPushButton(QObject::tr("..."));

            terafly_filepath = new QLineEdit();
            openteraflyFile = new QPushButton(QObject::tr("..."));

            b_3Dchecker = new QCheckBox();
            b_3Dchecker->setChecked(false);


            if(curwin)
            {
                raw_filepath->setText(cb.getImageName(curwin));
                raw_filepath->setDisabled(true);
                openrawFile->setDisabled(true);
                terafly_filepath->setDisabled(true);
                openteraflyFile->setDisabled(true);
            }

            marker_filepath = new QLineEdit();
            openmarkerFile = new QPushButton(QObject::tr("..."));
            if(curwin)
            {
                openmarkerFile->setDisabled(true);
                marker_filepath->setDisabled(true);
            }

            layout->addWidget(new QLabel("color channel"),0,0);
            layout->addWidget(channel_spinbox, 0,1,1,5);
            layout->addWidget(new QLabel("background_threshold)"),1,0);
            layout->addWidget(bkgthresh_spinbox, 1,1,1,5);
            layout->addWidget(new QLabel("seed window size)"),2,0);
            layout->addWidget(seedwin_spinbox, 2,1,1,5);


            layout->addWidget(new QLabel("slip window size"),3,0);
            layout->addWidget(slipwin_spinbox, 3,1,1,3);
            layout->addWidget(new QLabel("3D crawler?"),3,4);
            layout->addWidget(b_3Dchecker);

            layout->addWidget(new QLabel("block_size"),4,0);
            layout->addWidget(block_spinbox, 4,1,1,3);
            layout->addWidget(new QLabel("adaptive size?"),4,4);
            layout->addWidget(b_adapWinchecker);

            layout->addWidget(new QLabel(QObject::tr("va3draw/raw image:")),5,0);
            layout->addWidget(raw_filepath,5,1,1,4);
            layout->addWidget(openrawFile,5,5,1,1);

            layout->addWidget(new QLabel(QObject::tr("marker file:")),6,0);
            layout->addWidget(marker_filepath,6,1,1,4);
            layout->addWidget(openmarkerFile,6,5,1,1);

            layout->addWidget(new QLabel(QObject::tr("terafly format file:")),7,0);
            layout->addWidget(terafly_filepath,7,1,1,4);
            layout->addWidget(openteraflyFile,7,5,1,1);

            QHBoxLayout * hbox2 = new QHBoxLayout();
            QPushButton * ok = new QPushButton(" ok ");
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox2->addWidget(cancel);
            hbox2->addWidget(ok);

            layout->addLayout(hbox2,8,0,8,6);
            setLayout(layout);
            setWindowTitle(QString("UltraTracer_MOST"));


            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

            connect(channel_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(bkgthresh_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(seedwin_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(slipwin_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));

            connect(b_adapWinchecker, SIGNAL(stateChanged(int)), this, SLOT(update()));

            connect(block_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(openrawFile, SIGNAL(clicked()), this, SLOT(_slots_openrawFile()));
            connect(openmarkerFile, SIGNAL(clicked()), this, SLOT(_slots_openmarkerFile()));

            connect(b_3Dchecker, SIGNAL(stateChanged(int)), this, SLOT(update()));
            connect(openteraflyFile, SIGNAL(clicked()), this, SLOT(_slots_openteraflyFile()));

            update();
        }

        ~neurontracer_most_raw(){}

        public slots:
        void update()
        {
            channel = channel_spinbox->value();
            bkg_thresh = bkgthresh_spinbox->value();

            block_size = block_spinbox->value();
            seed_win = seedwin_spinbox->value();

            rawfilename = raw_filepath->text();
            markerfilename = marker_filepath->text();
            teraflyfilename = terafly_filepath->text();

            slip_win = slipwin_spinbox->value();

            b_adapWinchecker->isChecked() ? adap_win = 1 : adap_win = 0;
            b_3Dchecker->isChecked() ? tracing_3D = 1 : tracing_3D = 0;

        }

        void _slots_openrawFile()
        {
            QFileDialog d(this);
            QString fileOpenName;
            fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open Raw File/TC File"),
                                                        "",
                                                        QObject::tr("Supported file (*.raw *.RAW *.V3DRAW *.v3draw *.tc)"
                                                            ));
            if(!fileOpenName.isEmpty())
            {
                raw_filepath->setText(fileOpenName);
            }
            update();

        }

        void _slots_openmarkerFile()
        {
            QFileDialog d(this);
            QString fileOpenName;
            fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open Marker/Point Cloud File"),
                                                        "",
                                                        QObject::tr("Supported file (*.marker *.MARKER *.apo *.APO)"
                                                            ));
            if(!fileOpenName.isEmpty())
            {
                marker_filepath->setText(fileOpenName);
            }
            update();

        }

        void _slots_openteraflyFile()
        {
            QFileDialog d(this);
            QString fileOpenName;
            fileOpenName = QFileDialog::getExistingDirectory(0, QObject::tr("Open Terafly File"),
                                                        "");
            if(!fileOpenName.isEmpty())
            {
                terafly_filepath->setText(fileOpenName);
            }
            update();

        }
    public:

        QSpinBox * channel_spinbox;
        QSpinBox * bkgthresh_spinbox;
        QSpinBox * seedwin_spinbox;
        QSpinBox * slipwin_spinbox;
        QCheckBox * b_adapWinchecker;
        QCheckBox * b_3Dchecker;


        QSpinBox * block_spinbox;


        QLineEdit * raw_filepath;
        QLineEdit * marker_filepath;
        QPushButton *openrawFile;
        QPushButton *openmarkerFile;

        QLineEdit * terafly_filepath;
        QPushButton *openteraflyFile;


        Image4DSimple* image;
        LandmarkList listLandmarks;
        int  bkg_thresh;
        int  channel;
        int  seed_win;
        int  slip_win;
        int block_size;
        int downsample_factor;
        int adap_win;
        int tracing_3D;

        QString rawfilename;
        QString markerfilename;
        QString teraflyfilename;

    };

class neurontracer_neutube_raw : public QDialog
    {
        Q_OBJECT

    public:
        neurontracer_neutube_raw(V3DPluginCallback2 &cb, QWidget *parent)
        {
            image = 0;
            v3dhandle curwin = cb.currentImageWindow();
            if (curwin)
            {
                image = cb.getImage(curwin);
                listLandmarks = cb.getLandmark(curwin);
                if(listLandmarks.count() ==0)
                {
                    v3d_msg("No markers in the current image, please select a marker.");
                    return;
                }
            }

            QGridLayout * layout = new QGridLayout();
            block_spinbox = new QSpinBox();
            block_spinbox->setRange(1,2048);
            block_spinbox->setValue(768);

            b_adapWinchecker = new QCheckBox();
            b_adapWinchecker->setChecked(false);

            raw_filepath = new QLineEdit();
            openrawFile = new QPushButton(QObject::tr("..."));

            b_3Dchecker = new QCheckBox();
            b_3Dchecker->setChecked(false);


            b_grid = new QCheckBox();
            b_grid->setChecked(false);

            terafly_filepath = new QLineEdit();
            openteraflyFile = new QPushButton(QObject::tr("..."));

            if(curwin)
            {
                raw_filepath->setText(cb.getImageName(curwin));
                raw_filepath->setDisabled(true);
                openrawFile->setDisabled(true);
                terafly_filepath->setDisabled(true);
                openteraflyFile->setDisabled(true);
            }

            marker_filepath = new QLineEdit();
            openmarkerFile = new QPushButton(QObject::tr("..."));
            if(curwin)
            {
                openmarkerFile->setDisabled(true);
                marker_filepath->setDisabled(true);
            }

            layout->addWidget(new QLabel("block_size"),4,0);
            layout->addWidget(block_spinbox, 4,1);
            layout->addWidget(new QLabel("adaptive size?"),4,2);
            layout->addWidget(b_adapWinchecker,4,3);
            layout->addWidget(new QLabel("3D crawler?"),4,4);
            layout->addWidget(b_3Dchecker,4,5);
            layout->addWidget(new QLabel("Grid trace?"),4,6);
            layout->addWidget(b_grid,4,7);



            layout->addWidget(new QLabel(QObject::tr("va3draw/raw image:")),5,0);
            layout->addWidget(raw_filepath,5,1,1,4);
            layout->addWidget(openrawFile,5,5,1,1);

            layout->addWidget(new QLabel(QObject::tr("marker file:")),6,0);
            layout->addWidget(marker_filepath,6,1,1,4);
            layout->addWidget(openmarkerFile,6,5,1,1);

            layout->addWidget(new QLabel(QObject::tr("terafly format file:")),7,0);
            layout->addWidget(terafly_filepath,7,1,1,4);
            layout->addWidget(openteraflyFile,7,5,1,1);

            QHBoxLayout * hbox2 = new QHBoxLayout();
            QPushButton * ok = new QPushButton(" ok ");
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox2->addWidget(cancel);
            hbox2->addWidget(ok);

            layout->addLayout(hbox2,8,0,8,6);
            setLayout(layout);
            setWindowTitle(QString("UltraTracer"));


            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

            connect(block_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(openrawFile, SIGNAL(clicked()), this, SLOT(_slots_openrawFile()));
            connect(openmarkerFile, SIGNAL(clicked()), this, SLOT(_slots_openmarkerFile()));

            connect(b_adapWinchecker, SIGNAL(stateChanged(int)), this, SLOT(update()));
            connect(openteraflyFile, SIGNAL(clicked()), this, SLOT(_slots_openteraflyFile()));
            connect(b_3Dchecker, SIGNAL(stateChanged(int)), this, SLOT(update()));
            connect(b_grid, SIGNAL(stateChanged(int)), this, SLOT(update()));



            update();
        }

        ~neurontracer_neutube_raw(){}

        public slots:
        void update()
        {
            block_size = block_spinbox->value();
            rawfilename = raw_filepath->text();
            markerfilename = marker_filepath->text();

            b_adapWinchecker->isChecked() ? adap_win = 1 : adap_win = 0;
            b_3Dchecker->isChecked() ? tracing_3D = 1 : tracing_3D = 0;
            b_grid->isChecked() ? grid_trace = 1 : grid_trace = 0;
            teraflyfilename = terafly_filepath->text();

        }

        void _slots_openrawFile()
        {
            QFileDialog d(this);
            QString fileOpenName;
            fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open Raw File/TC File"),
                                                        "",
                                                        QObject::tr("Supported file (*.raw *.RAW *.V3DRAW *.v3draw *.tc)"
                                                            ));
            if(!fileOpenName.isEmpty())
            {
                raw_filepath->setText(fileOpenName);
            }
            update();

        }

        void _slots_openmarkerFile()
        {
            QFileDialog d(this);
            QString fileOpenName;
            fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open Marker/Point Cloud File"),
                                                        "",
                                                        QObject::tr("Supported file (*.marker *.MARKER *.apo *.APO)"
                                                            ));
            if(!fileOpenName.isEmpty())
            {
                marker_filepath->setText(fileOpenName);
            }
            update();

        }

        void _slots_openteraflyFile()
        {
            QFileDialog d(this);
            QString fileOpenName;
            fileOpenName = QFileDialog::getExistingDirectory(0, QObject::tr("Open Terafly File"),
                                                        "");
            if(!fileOpenName.isEmpty())
            {
                terafly_filepath->setText(fileOpenName);
            }
            update();

        }
    public:


        QSpinBox * block_spinbox;
        QCheckBox * b_adapWinchecker;
        QCheckBox * b_3Dchecker;
        QCheckBox * b_grid;



        QLineEdit * raw_filepath;
        QLineEdit * marker_filepath;
        QPushButton *openrawFile;
        QPushButton *openmarkerFile;

        QLineEdit * terafly_filepath;
        QPushButton *openteraflyFile;


        Image4DSimple* image;
        LandmarkList listLandmarks;
        int block_size;
        int adap_win;
        int tracing_3D;
        int grid_trace;

        QString rawfilename;
        QString markerfilename;
        QString teraflyfilename;

    };

#endif

