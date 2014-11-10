/* neuronassembler_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2014-11-09 : by Zhi Zhou
 */
 
#ifndef __NEURONASSEMBLER_PLUGIN_H__
#define __NEURONASSEMBLER_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class neuronassembler : public QObject, public V3DPluginInterface2_1
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

            channel_spinbox = new QSpinBox();
            channel_spinbox->setRange(1,image->getCDim());
            channel_spinbox->setValue(1);
            bkgthresh_spinbox = new QSpinBox();
            bkgthresh_spinbox->setRange(-1, 255);
            bkgthresh_spinbox->setValue(10);

            block_spinbox = new QSpinBox();
            block_spinbox->setRange(1,2048);
            block_spinbox->setValue(1024);

            tc_filepath = new QLineEdit();
            openTcFile = new QPushButton(QObject::tr("..."));

            QStringList methodList;
            methodList << "MOST Tracing" << "NeuTube Tracing" << "Farsight Snake Tracing";

            layout->addWidget(new QLabel("color channel"),0,0);
            layout->addWidget(channel_spinbox, 0,1,1,5);
            layout->addWidget(new QLabel("background_threshold \n(if set as -1, \nthen auto-thresholding)"),1,0);
            layout->addWidget(bkgthresh_spinbox, 1,1,1,5);

            layout->addWidget(new QLabel("block_size"),2,0);
            layout->addWidget(block_spinbox, 2,1,1,5);

            layout->addWidget(new QLabel(QObject::tr("tc file path:")),3,0);
            layout->addWidget(tc_filepath,3,1,1,4);
            layout->addWidget(openTcFile,3,5,1,1);

            layout->addWidget(new QLabel("Tracing method:"),4,0);
            combo_method = new QComboBox();
            combo_method->addItems(methodList);
            layout->addWidget(combo_method, 4,1,1,5);

            QHBoxLayout * hbox2 = new QHBoxLayout();
            QPushButton * ok = new QPushButton(" ok ");
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox2->addWidget(cancel);
            hbox2->addWidget(ok);

            layout->addLayout(hbox2,5,0,5,6);
            setLayout(layout);
            setWindowTitle(QString("Vaa3D-NeuronAssembler"));


            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

            connect(channel_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(bkgthresh_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(combo_method, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));

            connect(block_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(openTcFile, SIGNAL(clicked()), this, SLOT(_slots_openTcFile()));

            update();
        }

        ~NeuronAssemblerDialog(){}

        public slots:
        void update()
        {
            channel = channel_spinbox->value();
            bkg_thresh = bkgthresh_spinbox->value();

            block_size = block_spinbox->value();
            tcfilename = tc_filepath->text();
            tracing_method = combo_method->currentIndex();

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

        QSpinBox * channel_spinbox;
        QSpinBox * bkgthresh_spinbox;
        QSpinBox * block_spinbox;
        QLineEdit * tc_filepath;
        QPushButton *openTcFile;
        QComboBox *combo_method;

        Image4DSimple* image;
        LandmarkList listLandmarks;
        int  channel;
        int block_size;
        int  bkg_thresh;
        int tracing_method;

        QString tcfilename;

    };
