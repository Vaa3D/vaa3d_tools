#ifndef DEEPNEURONUI_H
#define DEEPNEURONUI_H

#include <qsettings.h>

#include "v3d_interface.h"
#include "ui_DeepNeuronForm.h"
#include "imgOperator.h"
#include "DLOperator.h"

namespace Ui
{
	class DeepNeuronDialog;
}

class DeepNeuronUI : public QDialog
{
	Q_OBJECT

public:
	DeepNeuronUI(QWidget* parent, V3DPluginCallback2* callback);
	~DeepNeuronUI();

	LandmarkList markerList;
	QString deployDisplay;
	QString modelDisplay;
	QString meanDisplay;

	QString imageName;
	Image4DSimple* curImg4DPtr;

public slots:
	void okClicked();
	void filePath();
	void closeClicked();

	void progressBarUpdater(QString taskName, int percentage);

private:
	Ui::DeepNeuronDialog* ui;
	V3DPluginCallback2* mainCallBack;

	imgOperator* curImgProcessor;
	DLOperator* curImgDLProcessor;

	QString QSettingFileName;
	void loadSettings();
};

class DNInputDialog : public QDialog
    {
        Q_OBJECT

    public:
        DNInputDialog(V3DPluginCallback2* callback,QWidget *parent)
        {

            QGridLayout * layout = new QGridLayout();
            layout->addWidget(new QLabel(QObject::tr("Deploy file:")),1,1);
            m_pLineEdit_modelfile = new QLineEdit();
            layout->addWidget(m_pLineEdit_modelfile,1,2,1,3);
            pPushButton_openFileDlg_modelfile = new QPushButton(QObject::tr("..."));
            layout->addWidget(pPushButton_openFileDlg_modelfile,1,6,1,2);

            layout->addWidget(new QLabel(QObject::tr("Trained model file:")),2,1);
            m_pLineEdit_trainedfile = new QLineEdit();
            layout->addWidget(m_pLineEdit_trainedfile,2,2,1,3);
            pPushButton_openFileDlg_trainedfile = new QPushButton(QObject::tr("..."));
            layout->addWidget(pPushButton_openFileDlg_trainedfile,2,6,1,2);

            layout->addWidget(new QLabel(QObject::tr("Database mean file:")),3,1);
            m_pLineEdit_meanfile = new QLineEdit();
            layout->addWidget(m_pLineEdit_meanfile,3,2,1,3);
            pPushButton_openFileDlg_meanfile = new QPushButton(QObject::tr("..."));
            layout->addWidget(pPushButton_openFileDlg_meanfile,3,6,1,2);


            layout->addWidget(new QLabel(QObject::tr("Step size:")),4,1);
            stepsize_spinbox = new QSpinBox();
            stepsize_spinbox->setRange(1,20);
            stepsize_spinbox->setValue(10);
            layout->addWidget(stepsize_spinbox,4,2,1,3);


            layout->addWidget(new QLabel(QObject::tr("SWC file:")),5,1);
            m_pLineEdit_swcfile = new QLineEdit();
            layout->addWidget(m_pLineEdit_swcfile,5,2,1,3);
            pPushButton_openFileDlg_swcfile = new QPushButton(QObject::tr("..."));
            layout->addWidget(pPushButton_openFileDlg_swcfile,5,6,1,2);

            QPushButton * ok = new QPushButton("Ok");
            QPushButton * cancel = new QPushButton("Cancel");
            ok->setDefault(true);
            layout->addWidget(ok, 6, 0, 1, 2);
            layout->addWidget(cancel, 6, 6, 1, 2);
            this->setLayout(layout);

            QSettings settings("Allen Institute", "Deep Neuron");
            QString savedDeploy = settings.value("deploy file").toString();
            QString savedModel = settings.value("trained model file").toString();
            QString savedMean = settings.value("database mean file").toString();
            m_pLineEdit_modelfile->setText(savedDeploy);
            m_pLineEdit_trainedfile->setText(savedModel);
            m_pLineEdit_meanfile->setText(savedMean);

            connect(pPushButton_openFileDlg_modelfile, SIGNAL(clicked()), this, SLOT(_slots_openFileDlg_modelfile()));
            connect(pPushButton_openFileDlg_trainedfile, SIGNAL(clicked()), this, SLOT(_slots_openFileDlg_trainedfile()));
            connect(pPushButton_openFileDlg_meanfile, SIGNAL(clicked()), this, SLOT(_slots_openFileDlg_meanfile()));
            connect(pPushButton_openFileDlg_swcfile, SIGNAL(clicked()), this, SLOT(_slots_openFileDlg_swcfile()));

            connect(stepsize_spinbox, SIGNAL(valueChanged(int)), this, SLOT(update()));

            connect(ok, SIGNAL(clicked()), this, SLOT(_slot_start()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
        }

        ~DNInputDialog(){}

        public slots:
        void update()
        {
            model_file = m_pLineEdit_modelfile->text();
            trained_file = m_pLineEdit_trainedfile->text();
            mean_file = m_pLineEdit_meanfile->text();
            swc_file = m_pLineEdit_swcfile->text();
            step_size = stepsize_spinbox->text().toInt();
        }

        void _slots_openFileDlg_modelfile()
        {
            QFileDialog d(this);
            QString fileOpenName;
            fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Choose deploy file:"),
                                                        "",
                                                        QObject::tr("Supported file (*.prototxt)"
                                                                    ));
            if(!fileOpenName.isEmpty())
            {
                m_pLineEdit_modelfile->setText(fileOpenName);
            }
        }

        void _slots_openFileDlg_trainedfile()
        {
            QFileDialog d(this);
            QString fileOpenName;
            fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Choose trained model file:"),
                                                        "",
                                                        QObject::tr("Supported file (*.caffemodel)"
                                                                    ));
            if(!fileOpenName.isEmpty())
            {
                m_pLineEdit_trainedfile->setText(fileOpenName);
            }
        }

        void _slots_openFileDlg_meanfile()
        {
            QFileDialog d(this);
            QString fileOpenName;
            fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Choose databse mean file:"),
                                                        "",
                                                        QObject::tr("Supported file (*.binaryproto)"
                                                                    ));
            if(!fileOpenName.isEmpty())
            {
                m_pLineEdit_meanfile->setText(fileOpenName);
            }
        }

        void _slots_openFileDlg_swcfile()
        {
            QFileDialog d(this);
            QString fileOpenName;
            fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Choose SWC file:"),
                                                        "",
                                                        QObject::tr("Supported file (*.swc)"
                                                                    ));
            if(!fileOpenName.isEmpty())
            {
                m_pLineEdit_swcfile->setText(fileOpenName);
            }
        }

        void _slot_start()
        {
            update();
            accept();
            QSettings settings("Allen Institute", "Deep Neuron");
            settings.setValue("deploy file", model_file);
            settings.setValue("trained model file", trained_file);
            settings.setValue("database mean file", mean_file);
        }

    public:
        QString model_file;
        QString trained_file;
        QString mean_file;
        QString swc_file;
        int step_size;

        QLineEdit *m_pLineEdit_modelfile;
        QLineEdit *m_pLineEdit_trainedfile;
        QLineEdit *m_pLineEdit_meanfile;
        QLineEdit *m_pLineEdit_swcfile;

        QPushButton *pPushButton_openFileDlg_modelfile;
        QPushButton *pPushButton_openFileDlg_trainedfile;
        QPushButton *pPushButton_openFileDlg_meanfile;
        QPushButton *pPushButton_openFileDlg_swcfile;

        QSpinBox * stepsize_spinbox;


    };

#endif
