/* neuronassembler_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2014-11-09 : by Zhi Zhou
 */
 
#ifndef __NEURONASSEMBLER_PLUGIN_H__
#define __NEURONASSEMBLER_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include "TReMap_plugin.h"
#include "APP1_plugin.h"


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


class NeuronAssemblerDialog_raw : public QDialog
    {
        Q_OBJECT

    public:
        NeuronAssemblerDialog_raw(V3DPluginCallback2 &cb, QWidget *parent)
        {

            QGridLayout * layout = new QGridLayout();

            channel_spinbox = new QSpinBox();
            channel_spinbox->setRange(1,1);
            channel_spinbox->setValue(1);
            bkgthresh_spinbox = new QSpinBox();
            bkgthresh_spinbox->setRange(-1, 255);
            bkgthresh_spinbox->setValue(10);

            block_spinbox = new QSpinBox();
            block_spinbox->setRange(1,2048);
            block_spinbox->setValue(1024);

            raw_filepath = new QLineEdit();
            openrawFile = new QPushButton(QObject::tr("..."));

            marker_filepath = new QLineEdit();
            openmarkerFile = new QPushButton(QObject::tr("..."));


            QStringList methodList;
            methodList << "" << "MOST Tracing" << "NeuTube Tracing" << "Farsight Snake Tracing" << "TReMap Tracing" << "All-path Pruning 1(APP1) Tracing";

            layout->addWidget(new QLabel("color channel"),0,0);
            layout->addWidget(channel_spinbox, 0,1,1,5);
            layout->addWidget(new QLabel("background_threshold"),1,0);
            layout->addWidget(bkgthresh_spinbox, 1,1,1,5);

            layout->addWidget(new QLabel("block_size"),2,0);
            layout->addWidget(block_spinbox, 2,1,1,5);

            layout->addWidget(new QLabel(QObject::tr("va3draw/raw image:")),3,0);
            layout->addWidget(raw_filepath,3,1,1,4);
            layout->addWidget(openrawFile,3,5,1,1);

            layout->addWidget(new QLabel(QObject::tr("marker file:")),4,0);
            layout->addWidget(marker_filepath,4,1,1,4);
            layout->addWidget(openmarkerFile,4,5,1,1);

            layout->addWidget(new QLabel("Tracing method:"),5,0);
            combo_method = new QComboBox();
            combo_method->addItems(methodList);
            layout->addWidget(combo_method, 5,1,1,5);

            QHBoxLayout * hbox2 = new QHBoxLayout();
            QPushButton * ok = new QPushButton(" ok ");
            ok->setDefault(true);
            QPushButton * cancel = new QPushButton("cancel");
            hbox2->addWidget(cancel);
            hbox2->addWidget(ok);

            layout->addLayout(hbox2,6,0,6,6);
            setLayout(layout);
            setWindowTitle(QString("Vaa3D-NeuronAssembler"));


            connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

            connect(channel_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(bkgthresh_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(combo_method, SIGNAL(currentIndexChanged(int)), this, SLOT(_slots_method()));

            connect(block_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));
            connect(openrawFile, SIGNAL(clicked()), this, SLOT(_slots_openrawFile()));
            connect(openmarkerFile, SIGNAL(clicked()), this, SLOT(_slots_openmarkerFile()));

            update();
        }

        ~NeuronAssemblerDialog_raw(){}

        public slots:
        void update()
        {
            channel = channel_spinbox->value();
            bkg_thresh = bkgthresh_spinbox->value();

            block_size = block_spinbox->value();
            tracing_method = combo_method->currentIndex();

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

        void _slots_method()
        {
            if(combo_method->currentIndex() == 2)
            {
                merge = 0;
                if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Merge Close Nodes?"), QMessageBox::Yes, QMessageBox::No))
                    merge = 1;
            }
            else if (combo_method->currentIndex() == 4)
            {
                mipTracingeDialog dialog(this);
                if (dialog.exec()!=QDialog::Accepted)
                    return;
                is_gsdt = dialog.is_gsdt;
                is_break_accept = dialog.is_break_accept;
                bkgthresh_spinbox->setValue(dialog.bkg_thresh);
                length_thresh = dialog.length_thresh;
                cnn_type = dialog.cnn_type;
                SR_ratio = dialog.SR_ratio;
                b_256cube = dialog.b_256cube;
                b_RadiusFrom2D = dialog.b_RadiusFrom2D;
                mip_plane = dialog.mip_plane;
            }
            else if (combo_method->currentIndex() == 5)
            {
                APP1TracingeDialog dialog(this);
                if (dialog.exec()!=QDialog::Accepted)
                    return;
                bkgthresh_spinbox->setValue(dialog.bkg_thresh);
                b_256cube = dialog.b_256cube;


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

        QLineEdit * raw_filepath;
        QLineEdit * marker_filepath;
        QPushButton *openrawFile;
        QPushButton *openmarkerFile;

        Image4DSimple* image;
        LandmarkList listLandmarks;
        int  channel;
        int block_size;
        int  bkg_thresh;
        int tracing_method;

        //NeuTube
        int merge;

        //TReMap
        int is_gsdt;
        int is_break_accept;
        double length_thresh;
        int  cnn_type;
        double SR_ratio;
        int  b_256cube;
        int b_RadiusFrom2D;
        int mip_plane;

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
            methodList << "" << "MOST Tracing" << "NeuTube Tracing" << "Farsight Snake Tracing" << "TReMap Tracing" << "All-path Pruning 1(APP1) Tracing";

            layout->addWidget(new QLabel("color channel"),0,0);
            layout->addWidget(channel_spinbox, 0,1,1,5);
            layout->addWidget(new QLabel("background_threshold"),1,0);
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
            connect(combo_method, SIGNAL(currentIndexChanged(int)), this, SLOT(_slots_method()));

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


        void _slots_method()
        {
            if(combo_method->currentIndex() == 2)
            {
                merge = 0;
                if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Merge Close Nodes?"), QMessageBox::Yes, QMessageBox::No))
                    merge = 1;
            }
            else if (combo_method->currentIndex() == 4)
            {
                mipTracingeDialog dialog(this);
                if (dialog.exec()!=QDialog::Accepted)
                    return;
                is_gsdt = dialog.is_gsdt;
                is_break_accept = dialog.is_break_accept;
                bkgthresh_spinbox->setValue(dialog.bkg_thresh);
                length_thresh = dialog.length_thresh;
                cnn_type = dialog.cnn_type;
                SR_ratio = dialog.SR_ratio;
                b_256cube = dialog.b_256cube;
                b_RadiusFrom2D = dialog.b_RadiusFrom2D;
                mip_plane = dialog.mip_plane;
            }
            else if (combo_method->currentIndex() == 5)
            {
                APP1TracingeDialog dialog(this);
                if (dialog.exec()!=QDialog::Accepted)
                    return;
                bkgthresh_spinbox->setValue(dialog.bkg_thresh);
                b_256cube = dialog.b_256cube;

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

        //NeuTube
        int merge;

        //TReMap
        int is_gsdt;
        int is_break_accept;
        double length_thresh;
        int  cnn_type;
        double SR_ratio;
        int  b_256cube;
        int b_RadiusFrom2D;
        int mip_plane;

        QString tcfilename;

    };

