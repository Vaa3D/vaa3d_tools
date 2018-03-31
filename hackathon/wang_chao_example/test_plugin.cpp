/* test_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-3-16 : by YourName
 */
#include<iostream>
#include <v3d_interface.h>
#include "v3d_message.h"
#include <vector>
#include "test_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(test, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("segment")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
        <<tr("segment")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("segment"))
	{

       image_threshold(callback, parent);
       v3d_msg("to be implemented.");
	}
    else if (menu_name == tr("about"))
	{
        void couthelp();
        v3d_msg("not to be implemented.");

	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2018-3-16"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{

	}
	else return false;

	return true;
}

int image_threshold(V3DPluginCallback2 &callback, QWidget *parent)
{

     v3dhandle graph=callback.currentImageWindow();
     if(!graph)
     {
         QMessageBox::information(0,"title",QObject::tr("no image"));
         return -1;

     }

     Image4DSimple *pp=callback.getImage(graph);

    int c=-1;
    bool ok;
    c = QInputDialog::getInteger(parent, "Channel Number", "Set the subject channel number:", 0, 0, pp->getCDim()-1, 1, &ok);
    if (!ok) return -1;
    // asking for threshold
    int thres = -1;
    thres = QInputDialog::getInteger(parent, "Threshold", "Set threshold:", 0, 0, 255, 1, &ok);
    if (!ok) return -1;
    // QSpinBox * bkg_thresh_spinbox = new QSpinBox();
    // bkg_thresh_spinbox->setRange(-1, 255);
    // bkg_thresh_spinbox->setValue(bkg_thresh);

    // QGridLayout * layout = new QGridLayout();

    // QDialog * dialog = new QDialog();
    // layout->addWidget(new QLabel("bkg_thresh"),0,0);
    // layout->addWidget(bkg_thresh_spinbox, 0,1,1,5);

    // bkg_thresh = bkg_thresh_spinbox->value();






    V3DLONG  sz[3];
    sz[0]=pp->getXDim();
    sz[1]=pp->getYDim();
    sz[2]=pp->getZDim();

   // printf("%ld,%ld,%ld/n",sz[0],sz[1],sz[2]);
    V3DLONG tb=sz[0]*sz[1]*sz[2]; //tb是图像矩阵的大小

    unsigned char * inimgld=pp->getRawDataAtChannel(c);
    unsigned char * nm= new unsigned char [tb];

    for(V3DLONG i=0;i<tb;i++)
    {
       // v3d_msg(QString("intensity is %1, index is %2").arg(inimgld[i]).arg(i));

        if(inimgld[i]>=thres)
           nm[i]=255;
        else
            nm[i]=0;

    }

 //   Image4DSimple * new4DImage = new Image4DSimple();
 //   new4DImage->setData((unsigned char *)nm,sz[0],sz[1],sz[2],1,pp->getDatatype());
//
//   v3dhandle newwin = callback.newImageWindow();
 //   callback.setImage(newwin, new4DImage);
 //   callback.setImageName(newwin,QObject::tr("segment"));
  //  callback.updateImageWindow(newwin);


    v3d_msg("start set new Image");
    v3dhandle newwin=callback.newImageWindow();

    v3d_msg("get the pointer of newImageWindow");
    Image4DSimple *pp_out=callback.getImage(newwin);

    v3d_msg("get the pointer of new Image");
    pp_out->setData((unsigned char *)nm,sz[0],sz[1],sz[2],1,pp_out->getDatatype());
    v3d_msg("start set new Image two");
    callback.setImage(newwin,pp_out);

    callback.setImageName(newwin,QObject::tr("segment"));

    callback.updateImageWindow(newwin);

    return 1 ;



}

