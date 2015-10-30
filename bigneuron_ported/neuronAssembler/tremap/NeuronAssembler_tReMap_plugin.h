/* NeuronAssembler_tReMap_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2015-10-30 : by Zhi Zhou
 */
 
#ifndef __NEURONASSEMBLER_TREMAP_PLUGIN_H__
#define __NEURONASSEMBLER_TREMAP_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class NeuronAssembler_tReMap : public QObject, public V3DPluginInterface2_1
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

#endif

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
            marker_filepath = new QLineEdit("NULL");
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
			mip_box = new QSpinBox();
			mip_box->setValue(0);
			layout->addWidget(new QLabel(QObject::tr("mip:")),4,0);
			layout->addWidget(mip_box,4,1,1,5);

			ch_box = new QSpinBox();
			ch_box->setValue(1);
			layout->addWidget(new QLabel(QObject::tr("ch:")),5,0);
			layout->addWidget(ch_box,5,1,1,5);

			th_box = new QDoubleSpinBox();
			th_box->setValue(10);
			layout->addWidget(new QLabel(QObject::tr("th:")),6,0);
			layout->addWidget(th_box,6,1,1,5);

			b_256_box = new QSpinBox();
			b_256_box->setValue(0);
			layout->addWidget(new QLabel(QObject::tr("b_256:")),7,0);
			layout->addWidget(b_256_box,7,1,1,5);

			is_gsdt_box = new QSpinBox();
			is_gsdt_box->setValue(1);
			layout->addWidget(new QLabel(QObject::tr("is_gsdt:")),8,0);
			layout->addWidget(is_gsdt_box,8,1,1,5);

			is_gap_box = new QSpinBox();
			is_gap_box->setValue(0);
			layout->addWidget(new QLabel(QObject::tr("is_gap:")),9,0);
			layout->addWidget(is_gap_box,9,1,1,5);

			length_th_box = new QSpinBox();
			length_th_box->setValue(5);
			layout->addWidget(new QLabel(QObject::tr("length_th:")),10,0);
			layout->addWidget(length_th_box,10,1,1,5);

			layout->addLayout(hbox2,11,0,3,6);
			setWindowTitle(QString("Vaa3D-NeuronAssembler(TReMap)"));
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox2->addWidget(cancel);
            hbox2->addWidget(ok);

            setLayout(layout);

            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
			connect(mip_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(ch_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(th_box, SIGNAL(valueChanged(double)), this, SLOT(update()));
			connect(b_256_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(is_gsdt_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(is_gap_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(length_th_box, SIGNAL(valueChanged(int)), this, SLOT(update()));

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
			mip = mip_box->value();
			ch = ch_box->value();
			th = th_box->value();
			b_256 = b_256_box->value();
			is_gsdt = is_gsdt_box->value();
			is_gap = is_gap_box->value();
			length_th = length_th_box->value();
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
		QSpinBox *mip_box;
		int mip;
		QSpinBox *ch_box;
		int ch;
		QDoubleSpinBox *th_box;
		double th;
		QSpinBox *b_256_box;
		int b_256;
		QSpinBox *is_gsdt_box;
		int is_gsdt;
		QSpinBox *is_gap_box;
		int is_gap;
		QSpinBox *length_th_box;
		int length_th;
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

			mip_box = new QSpinBox();
			mip_box->setValue(0);
			layout->addWidget(new QLabel(QObject::tr("mip:")),2,0);
			layout->addWidget(mip_box,2,1,1,5);

			ch_box = new QSpinBox();
			ch_box->setValue(1);
			layout->addWidget(new QLabel(QObject::tr("ch:")),3,0);
			layout->addWidget(ch_box,3,1,1,5);

			th_box = new QDoubleSpinBox();
			th_box->setValue(10);
			layout->addWidget(new QLabel(QObject::tr("th:")),4,0);
			layout->addWidget(th_box,4,1,1,5);

			b_256_box = new QSpinBox();
			b_256_box->setValue(0);
			layout->addWidget(new QLabel(QObject::tr("b_256:")),5,0);
			layout->addWidget(b_256_box,5,1,1,5);

			is_gsdt_box = new QSpinBox();
			is_gsdt_box->setValue(1);
			layout->addWidget(new QLabel(QObject::tr("is_gsdt:")),6,0);
			layout->addWidget(is_gsdt_box,6,1,1,5);

			is_gap_box = new QSpinBox();
			is_gap_box->setValue(0);
			layout->addWidget(new QLabel(QObject::tr("is_gap:")),7,0);
			layout->addWidget(is_gap_box,7,1,1,5);

			length_th_box = new QSpinBox();
			length_th_box->setValue(5);
			layout->addWidget(new QLabel(QObject::tr("length_th:")),8,0);
			layout->addWidget(length_th_box,8,1,1,5);

			layout->addLayout(hbox2,9,0,2,6);
			setWindowTitle(QString("Vaa3D-NeuronAssembler(TReMap)"));
          //  layout->addLayout(hbox2,2,0,2,6);
            setLayout(layout);
        //    setWindowTitle(QString("Vaa3D-NeuronAssembler"));


            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
			connect(mip_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(ch_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(th_box, SIGNAL(valueChanged(double)), this, SLOT(update()));
			connect(b_256_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(is_gsdt_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(is_gap_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(length_th_box, SIGNAL(valueChanged(int)), this, SLOT(update()));

            connect(block_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(openTcFile, SIGNAL(clicked()), this, SLOT(_slots_openTcFile()));

            update();
        }

        ~NeuronAssemblerDialog(){}

        public slots:
        void update()
        {
            block_size = block_spinbox->value();
			mip = mip_box->value();
			ch = ch_box->value();
			th = th_box->value();
			b_256 = b_256_box->value();
			is_gsdt = is_gsdt_box->value();
			is_gap = is_gap_box->value();
			length_th = length_th_box->value();
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

		QSpinBox *mip_box;
		int mip;
		QSpinBox *ch_box;
		int ch;
		QDoubleSpinBox *th_box;
		double th;
		QSpinBox *b_256_box;
		int b_256;
		QSpinBox *is_gsdt_box;
		int is_gsdt;
		QSpinBox *is_gap_box;
		int is_gap;
		QSpinBox *length_th_box;
		int length_th;
	};
