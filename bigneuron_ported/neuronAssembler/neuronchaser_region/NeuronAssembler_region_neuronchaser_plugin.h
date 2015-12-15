/* NeuronAssembler_region_neuronchaser_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2015-01-01 : by Zhi Zhou 
 */
 
#ifndef __NEURONASSEMBLER_REGION_NEURONCHASER_PLUGIN_H__
#define __NEURONASSEMBLER_REGION_NEURONCHASER_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class NeuronAssembler_region_neuronchaser : public QObject, public V3DPluginInterface2_1
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
			bkg_thresh_box = new QSpinBox();
			bkg_thresh_box->setValue(40);
			layout->addWidget(new QLabel(QObject::tr("bkg_thresh:")),4,0);
			layout->addWidget(bkg_thresh_box,4,1,1,5);

			region_number_box = new QSpinBox();
			region_number_box->setValue(10000);
			layout->addWidget(new QLabel(QObject::tr("region_number:")),5,0);
			layout->addWidget(region_number_box,5,1,1,5);

			channel_box = new QSpinBox();
			channel_box->setValue(1);
			layout->addWidget(new QLabel(QObject::tr("channel:")),6,0);
			layout->addWidget(channel_box,6,1,1,5);

			scal_box = new QSpinBox();
			scal_box->setValue(10);
			layout->addWidget(new QLabel(QObject::tr("scal:")),7,0);
			layout->addWidget(scal_box,7,1,1,5);

			perc_box = new QSpinBox();
			perc_box->setValue(90);
			layout->addWidget(new QLabel(QObject::tr("perc:")),8,0);
			layout->addWidget(perc_box,8,1,1,5);

			znccTh_box = new QDoubleSpinBox();
			znccTh_box->setValue(0.6);
			layout->addWidget(new QLabel(QObject::tr("znccTh:")),9,0);
			layout->addWidget(znccTh_box,9,1,1,5);

			Ndir_box = new QSpinBox();
			Ndir_box->setValue(15);
			layout->addWidget(new QLabel(QObject::tr("Ndir:")),10,0);
			layout->addWidget(Ndir_box,10,1,1,5);

			angSig_box = new QDoubleSpinBox();
			angSig_box->setValue(30);
			layout->addWidget(new QLabel(QObject::tr("angSig:")),11,0);
			layout->addWidget(angSig_box,11,1,1,5);

			Ni_box = new QSpinBox();
			Ni_box->setValue(30);
			layout->addWidget(new QLabel(QObject::tr("Ni:")),12,0);
			layout->addWidget(Ni_box,12,1,1,5);

			Ns_box = new QSpinBox();
			Ns_box->setValue(5);
			layout->addWidget(new QLabel(QObject::tr("Ns:")),13,0);
			layout->addWidget(Ns_box,13,1,1,5);

			zDist_box = new QDoubleSpinBox();
			zDist_box->setValue(1);
			layout->addWidget(new QLabel(QObject::tr("zDist:")),14,0);
			layout->addWidget(zDist_box,14,1,1,5);

			saveMidres_box = new QSpinBox();
			saveMidres_box->setValue(0);
			layout->addWidget(new QLabel(QObject::tr("saveMidres:")),15,0);
			layout->addWidget(saveMidres_box,15,1,1,5);

			layout->addLayout(hbox2,16,0,3,6);
			setWindowTitle(QString("Vaa3D-NeuronAssembler(Region_neuronchaser)"));
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox2->addWidget(cancel);
            hbox2->addWidget(ok);

            setLayout(layout);

            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
			connect(bkg_thresh_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(region_number_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(channel_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(scal_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(perc_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(znccTh_box, SIGNAL(valueChanged(double)), this, SLOT(update()));
			connect(Ndir_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(angSig_box, SIGNAL(valueChanged(double)), this, SLOT(update()));
			connect(Ni_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(Ns_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(zDist_box, SIGNAL(valueChanged(double)), this, SLOT(update()));
			connect(saveMidres_box, SIGNAL(valueChanged(int)), this, SLOT(update()));

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
			bkg_thresh = bkg_thresh_box->value();
			region_number = region_number_box->value();
			channel = channel_box->value();
			scal = scal_box->value();
			perc = perc_box->value();
			znccTh = znccTh_box->value();
			Ndir = Ndir_box->value();
			angSig = angSig_box->value();
			Ni = Ni_box->value();
			Ns = Ns_box->value();
			zDist = zDist_box->value();
			saveMidres = saveMidres_box->value();
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
		QSpinBox *bkg_thresh_box;
		int bkg_thresh;
		QSpinBox *region_number_box;
		int region_number;
		QSpinBox *channel_box;
		int channel;
		QSpinBox *scal_box;
		int scal;
		QSpinBox *perc_box;
		int perc;
		QDoubleSpinBox *znccTh_box;
		double znccTh;
		QSpinBox *Ndir_box;
		int Ndir;
		QDoubleSpinBox *angSig_box;
		double angSig;
		QSpinBox *Ni_box;
		int Ni;
		QSpinBox *Ns_box;
		int Ns;
		QDoubleSpinBox *zDist_box;
		double zDist;
		QSpinBox *saveMidres_box;
		int saveMidres;
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

			bkg_thresh_box = new QSpinBox();
			bkg_thresh_box->setValue(40);
			layout->addWidget(new QLabel(QObject::tr("bkg_thresh:")),2,0);
			layout->addWidget(bkg_thresh_box,2,1,1,5);

			region_number_box = new QSpinBox();
			region_number_box->setValue(10000);
			layout->addWidget(new QLabel(QObject::tr("region_number:")),3,0);
			layout->addWidget(region_number_box,3,1,1,5);

			channel_box = new QSpinBox();
			channel_box->setValue(1);
			layout->addWidget(new QLabel(QObject::tr("channel:")),4,0);
			layout->addWidget(channel_box,4,1,1,5);

			scal_box = new QSpinBox();
			scal_box->setValue(10);
			layout->addWidget(new QLabel(QObject::tr("scal:")),5,0);
			layout->addWidget(scal_box,5,1,1,5);

			perc_box = new QSpinBox();
			perc_box->setValue(90);
			layout->addWidget(new QLabel(QObject::tr("perc:")),6,0);
			layout->addWidget(perc_box,6,1,1,5);

			znccTh_box = new QDoubleSpinBox();
			znccTh_box->setValue(0.6);
			layout->addWidget(new QLabel(QObject::tr("znccTh:")),7,0);
			layout->addWidget(znccTh_box,7,1,1,5);

			Ndir_box = new QSpinBox();
			Ndir_box->setValue(15);
			layout->addWidget(new QLabel(QObject::tr("Ndir:")),8,0);
			layout->addWidget(Ndir_box,8,1,1,5);

			angSig_box = new QDoubleSpinBox();
			angSig_box->setValue(30);
			layout->addWidget(new QLabel(QObject::tr("angSig:")),9,0);
			layout->addWidget(angSig_box,9,1,1,5);

			Ni_box = new QSpinBox();
			Ni_box->setValue(30);
			layout->addWidget(new QLabel(QObject::tr("Ni:")),10,0);
			layout->addWidget(Ni_box,10,1,1,5);

			Ns_box = new QSpinBox();
			Ns_box->setValue(5);
			layout->addWidget(new QLabel(QObject::tr("Ns:")),11,0);
			layout->addWidget(Ns_box,11,1,1,5);

			zDist_box = new QDoubleSpinBox();
			zDist_box->setValue(1);
			layout->addWidget(new QLabel(QObject::tr("zDist:")),12,0);
			layout->addWidget(zDist_box,12,1,1,5);

			saveMidres_box = new QSpinBox();
			saveMidres_box->setValue(0);
			layout->addWidget(new QLabel(QObject::tr("saveMidres:")),13,0);
			layout->addWidget(saveMidres_box,13,1,1,5);

			layout->addLayout(hbox2,14,0,2,6);
			setWindowTitle(QString("Vaa3D-NeuronAssembler(Region_neuronchaser)"));
          //  layout->addLayout(hbox2,2,0,2,6);
            setLayout(layout);
        //    setWindowTitle(QString("Vaa3D-NeuronAssembler"));


            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
			connect(bkg_thresh_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(region_number_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(channel_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(scal_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(perc_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(znccTh_box, SIGNAL(valueChanged(double)), this, SLOT(update()));
			connect(Ndir_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(angSig_box, SIGNAL(valueChanged(double)), this, SLOT(update()));
			connect(Ni_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(Ns_box, SIGNAL(valueChanged(int)), this, SLOT(update()));
			connect(zDist_box, SIGNAL(valueChanged(double)), this, SLOT(update()));
			connect(saveMidres_box, SIGNAL(valueChanged(int)), this, SLOT(update()));

            connect(block_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(openTcFile, SIGNAL(clicked()), this, SLOT(_slots_openTcFile()));

            update();
        }

        ~NeuronAssemblerDialog(){}

        public slots:
        void update()
        {
            block_size = block_spinbox->value();
			bkg_thresh = bkg_thresh_box->value();
			region_number = region_number_box->value();
			channel = channel_box->value();
			scal = scal_box->value();
			perc = perc_box->value();
			znccTh = znccTh_box->value();
			Ndir = Ndir_box->value();
			angSig = angSig_box->value();
			Ni = Ni_box->value();
			Ns = Ns_box->value();
			zDist = zDist_box->value();
			saveMidres = saveMidres_box->value();
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

		QSpinBox *bkg_thresh_box;
		int bkg_thresh;
		QSpinBox *region_number_box;
		int region_number;
		QSpinBox *channel_box;
		int channel;
		QSpinBox *scal_box;
		int scal;
		QSpinBox *perc_box;
		int perc;
		QDoubleSpinBox *znccTh_box;
		double znccTh;
		QSpinBox *Ndir_box;
		int Ndir;
		QDoubleSpinBox *angSig_box;
		double angSig;
		QSpinBox *Ni_box;
		int Ni;
		QSpinBox *Ns_box;
		int Ns;
		QDoubleSpinBox *zDist_box;
		double zDist;
		QSpinBox *saveMidres_box;
		int saveMidres;
	};
