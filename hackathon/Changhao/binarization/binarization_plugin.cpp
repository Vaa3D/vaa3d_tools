/* binarization_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-10-12 : by Guochanghao
 */
 
#include "v3d_message.h"
#include <iostream>
#include <vector>
#include "binarization_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(binarization, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
		<<tr("binarization")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("binarize")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("binarization"))
    {
       int flag=binarization(callback,parent);
       if (flag==1)
       {
            cout<<endl;
            cout<<"************************"<<endl;
            cout<<"*binarization finished!*"<<endl;
            cout<<"************************"<<endl;
       }
    }
	else
	{
        v3d_msg(tr("You can get the binary image by this plugin. "
            "Developed by Guochanghao, 2018-10-12"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("binarization"))
	{
        v3d_msg("To be implemented1.");
	}
	else return false;

	return true;
}


int binarization(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if(!curwin)
    {
            v3d_msg("No image is open.");
            return -1;
    }

    Image4DSimple *p4DImage = callback.getImage(curwin);

    //*************the default threshold of binarization
    int threshold = 10;

    //************set update the dialog
    QDialog * dialog = new QDialog();

    if(p4DImage->getZDim() > 1)
            dialog->setWindowTitle("3D neuron image binarization");
    else
            dialog->setWindowTitle("2D neuron image binarization");

    QGridLayout * layout = new QGridLayout();

    //*************set spinbox
    QSpinBox * binarize_spinbox = new QSpinBox();
    binarize_spinbox->setRange(0,255);
    binarize_spinbox->setValue(threshold);

    layout->addWidget(new QLabel("threshold of binarization"),0,0);
    layout->addWidget(binarize_spinbox, 0,1,1,5);

    QHBoxLayout * hbox2 = new QHBoxLayout();
    QPushButton * ok = new QPushButton(" ok ");
    ok->setDefault(true);
    QPushButton * cancel = new QPushButton("cancel");
    hbox2->addWidget(cancel);
    hbox2->addWidget(ok);

    layout->addLayout(hbox2,6,0,1,6);
    dialog->setLayout(layout);
    QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

    //**************run the dialog
    if(dialog->exec() != QDialog::Accepted)
        {
                if (dialog)
                {
                        delete dialog;
                        dialog=0;
                        cout<<"delete dialog"<<endl;
                }
                return -1;
        }

    //***************get the dialog return values
    threshold = binarize_spinbox->value();
    if (dialog) {delete dialog; dialog=0;}

    V3DLONG sz[4];
    sz[0] = p4DImage->getXDim();
    sz[1] = p4DImage->getYDim();
    sz[2] = p4DImage->getZDim();
    sz[3] = p4DImage->getCDim();
    unsigned char* data1d=0;
    data1d = p4DImage->getRawData();

    //For image processing please do not use original data.you'd better use the copy.
    unsigned char *newdata1d;
    newdata1d= new unsigned char[sz[0]*sz[1]*sz[2]];
    V3DLONG num_size=0;
    for(V3DLONG i=0;i<sz[0];i++)
    {
        for(V3DLONG j=0;j<sz[1];j++)
        {
            for(V3DLONG k=0;k<sz[2];k++)
            {
                newdata1d[num_size]=data1d[num_size];
                num_size++;
            }
        }
    }

    V3DLONG nx = sz[0];
    V3DLONG ny = sz[1];
    V3DLONG nz = sz[2];
    V3DLONG xy_area = nx*ny;


    for (V3DLONG z = 0; z < nz; z++)
    {
        for(V3DLONG x = 0; x < nx; x++)
        {
            for(V3DLONG y = 0; y < ny; y++)
            {
                if(data1d[z*xy_area+x*ny+y] > threshold)
                {   newdata1d[z*xy_area+x*ny+y] = 255;
                }
                else
                {   newdata1d[z*xy_area+x*ny+y] = 0;
                }
            }
        }
    }

    Image4DSimple *new4DImage = new Image4DSimple();
    //Here we can not use new4DImage->getXDim(),nothing to point
    new4DImage->setData((unsigned char*)newdata1d, sz[0], sz[1], sz[2], sz[3], p4DImage->getDatatype());
    v3dhandle newwin = callback.newImageWindow();
    callback.setImageName(newwin,QString("binarization threshold: %1").arg(threshold));
    callback.setImage(newwin, new4DImage);
    callback.updateImageWindow(newwin);


    return 1;
}
