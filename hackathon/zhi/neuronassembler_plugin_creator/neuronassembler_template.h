class NeuronAssemblerDialog_raw : public QDialog
    {
        Q_OBJECT

    public:
        NeuronAssemblerDialog_raw(V3DPluginCallback2 &cb, QWidget *parent)
        {

            QGridLayout * layout = new QGridLayout();

            block_spinbox = new QSpinBox();
            block_spinbox->setRange(1,2048);
            block_spinbox->setValue(1024);

            raw_filepath = new QLineEdit();
            openrawFile = new QPushButton(QObject::tr("..."));
            marker_filepath = new QLineEdit();
            openmarkerFile = new QPushButton(QObject::tr("..."));
            image_checker = new QCheckBox();
            image_checker->setChecked(false);

            layout->addWidget(new QLabel("block_size"),0,0);
            layout->addWidget(block_spinbox, 0,1,1,5);
            layout->addWidget(new QLabel(QObject::tr("va3draw/raw image:")),1,0);
            layout->addWidget(raw_filepath,1,1,1,4);
            layout->addWidget(openrawFile,1,5,1,1);

            layout->addWidget(new QLabel(QObject::tr("marker file:")),2,0);
            layout->addWidget(marker_filepath,2,1,1,4);
            layout->addWidget(openmarkerFile,2,5,1,1);

            layout->addWidget(new QLabel(QObject::tr("Tracing the entire image:")),3,0);
            layout->addWidget(image_checker,3,1,1,5);


            QHBoxLayout * hbox2 = new QHBoxLayout();
            QPushButton * ok = new QPushButton(" ok ");
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox2->addWidget(cancel);
            hbox2->addWidget(ok);

            setLayout(layout);

            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

            connect(block_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(openrawFile, SIGNAL(clicked()), this, SLOT(_slots_openrawFile()));
            connect(openmarkerFile, SIGNAL(clicked()), this, SLOT(_slots_openmarkerFile()));
            connect(image_checker, SIGNAL(stateChanged(int)), this, SLOT(update()));
            update();
        }

        ~NeuronAssemblerDialog_raw(){}

        public slots:
        void update()
        {
//            channel = channel_spinbox->value();
            image_checker->isChecked()? is_entire = 1 : is_entire = 0;
            block_size = block_spinbox->value();

            rawfilename = raw_filepath->text();
            markerfilename = marker_filepath->text();

        }

        void _slots_openrawFile()
        {
            QFileDialog d(this);
            QString fileOpenName;
            fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open Raw File"),
                                                        "",
                                                        QObject::tr("Supported file (*.raw *.RAW *.V3DRAW *.v3draw)"
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
            fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open Marker File"),
                                                        "",
                                                        QObject::tr("Supported file (*.marker *.MARKER)"
                                                            ));
            if(!fileOpenName.isEmpty())
            {
                marker_filepath->setText(fileOpenName);
            }
            update();

        }

public:

        QSpinBox * block_spinbox;
        QLineEdit * tc_filepath;
        QPushButton *openTcFile;
        QCheckBox *image_checker;
        QLineEdit * raw_filepath;
        QLineEdit * marker_filepath;
        QPushButton *openrawFile;
        QPushButton *openmarkerFile;

        Image4DSimple* image;
        LandmarkList listLandmarks;
        int block_size;
        int is_entire;
        QString rawfilename;
        QString markerfilename;
    };

class NeuronAssemblerDialog : public QDialog
    {
        Q_OBJECT

    public:
        NeuronAssemblerDialog(V3DPluginCallback2 &cb, QWidget *parent)
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

            listLandmarks = cb.getLandmark(curwin);
            if(listLandmarks.count() ==0)
            {
                v3d_msg("No markers in the current image, please select a marker.");
                return;
            }


            QGridLayout * layout = new QGridLayout();

            block_spinbox = new QSpinBox();
            block_spinbox->setRange(1,2048);
            block_spinbox->setValue(1024);

            tc_filepath = new QLineEdit();
            openTcFile = new QPushButton(QObject::tr("..."));

            layout->addWidget(new QLabel("block_size"),0,0);
            layout->addWidget(block_spinbox, 0,1,1,5);

            layout->addWidget(new QLabel(QObject::tr("tc file path:")),1,0);
            layout->addWidget(tc_filepath,1,1,1,4);
            layout->addWidget(openTcFile,1,5,1,1);

            QHBoxLayout * hbox2 = new QHBoxLayout();
            QPushButton * ok = new QPushButton(" ok ");
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox2->addWidget(cancel);
            hbox2->addWidget(ok);

          //  layout->addLayout(hbox2,2,0,2,6);
            setLayout(layout);
        //    setWindowTitle(QString("Vaa3D-NeuronAssembler"));


            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

            connect(block_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(openTcFile, SIGNAL(clicked()), this, SLOT(_slots_openTcFile()));

            update();
        }

        ~NeuronAssemblerDialog(){}

        public slots:
        void update()
        {
            block_size = block_spinbox->value();
            tcfilename = tc_filepath->text();
        }

        void _slots_openTcFile()
        {
            QFileDialog d(this);
            QString fileOpenName;
            fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open TC File"),
                                                        "",
                                                        QObject::tr("Supported file (*.tc)"
                                                            ));
            if(!fileOpenName.isEmpty())
            {
                tc_filepath->setText(fileOpenName);
            }
            update();

        }

public:

        QSpinBox * block_spinbox;
        QLineEdit * tc_filepath;
        QPushButton *openTcFile;

        Image4DSimple* image;
        LandmarkList listLandmarks;
        int block_size;

        QString tcfilename;

    };

