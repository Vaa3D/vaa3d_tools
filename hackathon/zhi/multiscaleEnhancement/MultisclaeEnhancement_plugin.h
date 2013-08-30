/* MultisclaeEnhancement_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2013-08-29 : by Zhi Zhou
 */
 
#ifndef __MULTISCLAEENHANCEMENT_PLUGIN_H__
#define __MULTISCLAEENHANCEMENT_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class selectiveEnhancement : public QObject, public V3DPluginInterface2_1
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

//define a simple dialog for choose Multiscale Enhancement parameters
class MultiscaleEnhancementDialog : public QDialog
    {
        Q_OBJECT

    public:
        MultiscaleEnhancementDialog(V3DPluginCallback2 &cb, QWidget *parent)
        {
            //initialization of variables
            image = 0;
            gridLayout = 0;

            //

            v3dhandleList win_list = cb.getImageWindowList();

            if(win_list.size()<1)
            {
                QMessageBox::information(0, "Gaussian Filter", QObject::tr("No image is open."));
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

            minD_editor = new QLineEdit(QString("").setNum(4));
            maxD_editor = new QLineEdit(QString("").setNum(10));
            scale_editor = new QLineEdit(QString("").setNum(5));
            channel_spinbox = new QSpinBox();
            channel_spinbox->setRange(1,chn_num);


            gridLayout->addWidget(new QLabel("Minimal Diameter"),0,0);
            gridLayout->addWidget(minD_editor, 0,1,1,5);
            gridLayout->addWidget(new QLabel("Maximal Diameter"),1,0);
            gridLayout->addWidget(maxD_editor, 1,1,1,5);
            gridLayout->addWidget(new QLabel("Scale value"),2,0);
            gridLayout->addWidget(scale_editor, 2,1,1,5);
            gridLayout->addWidget(new QLabel("Channel"),4,0);
            gridLayout->addWidget(channel_spinbox, 4,1,1,5);

            ok     = new QPushButton("OK");
            cancel = new QPushButton("Cancel");
            gridLayout->addWidget(cancel, 5,0);
            gridLayout->addWidget(ok,     5,1,1,5);;

            setLayout(gridLayout);
            setWindowTitle(QString("Multiscale Enhancement"));

            //slot interface
            connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
            update();
        }

        ~MultiscaleEnhancementDialog(){}

        public slots:
        void update()
        {
            d0 = atof(minD_editor->text().toStdString().c_str());
            d1 = atof(maxD_editor->text().toStdString().c_str());
            range = atof(scale_editor->text().toStdString().c_str());
            ch = channel_spinbox->text().toInt();
        }

    public:
        double d0,d1,range,ch;
        Image4DSimple* image;
        ROIList pRoiList;
        QGridLayout *gridLayout;
        QPushButton* ok;
        QPushButton* cancel;
        QLineEdit * minD_editor;
        QLineEdit * maxD_editor;
        QLineEdit * scale_editor;
        QSpinBox * channel_spinbox;

    };


#endif



