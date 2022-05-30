/* standardize_image_plugin.cpp
 * This is a standardize plugin, you can use it as a demo.
 * 2022-5-3 : by zll
 */
 
#include "v3d_message.h"

#include <vector>
#include <iostream>
#include "standardize_image_plugin.h"
using namespace std;
//Q_EXPORT_PLUGIN2(standardize_image, standardizePlugin);

bool standardize(unsigned char* pdata, V3DLONG* sz);

QStringList standardizePlugin::menulist() const
{
	return QStringList() 
		<<tr("standardize")
		<<tr("menu2")
		<<tr("about");
}

QStringList standardizePlugin::funclist() const
{
	return QStringList()
		<<tr("standardize")
		<<tr("func2")
		<<tr("help");
}

void standardizePlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("standardize"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a standardize plugin, you can use it as a demo.. "
			"Developed by zll, 2022-5-3"));
	}
}

bool standardizePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    vector<char*> infiles, inparas, outfiles;
    Image4DSimple * inputimg,* outputimg;
    int isdatafile=1,isout=1;
    unsigned char* pdata = 0;
    //if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) {
        inparas = *((vector<char*> *)input.at(1).p);
        isdatafile=atoi(inparas.at(0));
        isout=atoi(inparas.at(1));
    }
    if(isdatafile==1){
        infiles = *((vector<char*> *)input.at(0).p);
        qDebug()<<"Input is file!";
    }else{
        inputimg=((vector<Image4DSimple*> *)(input.at(0).p))->at(0);
        qDebug()<<"Input is img!";
    }

    if(isout==1){
        outfiles = *((vector<char*> *)output.at(0).p);
        qDebug()<<"Output is file!";
    }else{
        outputimg=((vector<Image4DSimple*> *)(output.at(0).p))->at(0);
        qDebug()<<"Output is img!";
    }
	if (func_name == tr("standardize"))
	{
        QString imagePath,outImagePath;


        V3DLONG sz[4] = {0,0,0,0};
        int dataType = 1;

        if(isdatafile==1){
            imagePath = infiles.size() >= 1 ? infiles[0] : "";
            outImagePath = outfiles.size()>=1 ? outfiles[0] : "";
            simple_loadimage_wrapper(callback,imagePath.toStdString().c_str(), pdata, sz, dataType);
        }else{
            V3DLONG totalbytes = inputimg->getTotalBytes();
            pdata = new unsigned char [totalbytes];
            memcpy(pdata, inputimg->getRawData(), totalbytes);
            dataType = inputimg->getUnitBytes();
            sz[0] = inputimg->getXDim();
            sz[1] = inputimg->getYDim();
            sz[2] = inputimg->getZDim();
            sz[3] = inputimg->getCDim();
        }
        standardize(pdata,sz);
        if(isout==1){
            simple_saveimage_wrapper(callback,outImagePath.toStdString().c_str(),pdata,sz,dataType);
        }else{
            ImagePixelType dt;
            if (dataType==1)
                dt = V3D_UINT8;
            else if (dataType==2)
                dt = V3D_UINT16;
            else if (dataType==4)
                dt = V3D_FLOAT32;
            outputimg->setData(pdata,sz[0],sz[1],sz[2],sz[3],dt);
        }
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
        cout<<"Usage : v3d -x standardize_image -f standardize -i <inimg_file> -o <outimg_file> "<<endl;
        cout<<endl;
        return true;
	}
	else return false;

	return true;
}

