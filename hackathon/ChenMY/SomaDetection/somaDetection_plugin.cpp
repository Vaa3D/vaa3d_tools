/*
somaDetection_plugin.cpp
Sun Dec  9 22:05:56 2018 by cmy
*/

#include "v3d_message.h"
#include <vector>
#include "somaDetection_plugin.h"
#include<iostream>
#include <QString>
#include <QtCore/QFile>
#include <fstream>
#include "head.h"
using namespace std;

Q_EXPORT_PLUGIN2(somaDetection,somaDetectionPlugin);
void function(V3DPluginCallback2 &callback,QWidget *parent);
bool function(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent);
const QString title = QObject::tr("somaDetection");

QStringList somaDetectionPlugin::menulist() const{
   return QStringList()
       << tr("function")
       << tr("about");
}

QStringList somaDetectionPlugin::funclist() const{
   return QStringList()
       << tr("function")
       << tr("help");
}

void somaDetectionPlugin::domenu(const QString &menu_name,V3DPluginCallback2 &callback,QWidget *parent){
   if(menu_name==tr("function")){
       function(callback,parent);
   }
   else{
       v3d_msg(tr("developing...Sun Dec  9 22:05:56 2018 by cmy"));
   }
}

bool somaDetectionPlugin::dofunc(const QString &func_name,const V3DPluginArgList &input,V3DPluginArgList &output,V3DPluginCallback2 &callback,QWidget *parent){
   if(func_name==tr("function")){
       return function(callback,input,output,parent);
   }
   else if(func_name==tr("help")){
       return true;
   }
   return false;
}

void function(V3DPluginCallback2 &callback,QWidget *parent){
	v3dhandle  curwin=callback.currentImageWindow();
	if(!curwin){
		v3d_msg("You don't have any image open in the main window.");
	return;
	}
	Image4DSimple *p4DImage=callback.getImage(curwin);
	if(!p4DImage){
		v3d_msg("The image pointer is invalid.Ensure your data  is valid and try again!");
		return;
	}
	unsigned char* data1d=p4DImage->getRawData();
	V3DLONG in_sz[3];
	in_sz[0]=p4DImage->getXDim();
	in_sz[1]=p4DImage->getYDim();
	in_sz[2]=p4DImage->getZDim();

	//FILE *p=fopen("F:/data.txt","wb");
	//fwrite(data1d,1,in_sz[0]*in_sz[1]*in_sz[2],p);
	//fclose(p);
	//	
	//cout<<in_sz[0]<<endl;
	//cout<<in_sz[1]<<endl;
	//cout<<in_sz[2]<<endl;
	//for(int i=0;i<10;i++)
	//{
	//	cout<<int(data1d[i])<<endl;
	//}
	/*for(int i=0;i<10;i++)
		cout<<double(data1d[i])<<"   ";
	double *img=(double*)data1d;*/
	//cout<<double(data1d[8389120])<<"   " << endl;
	cout << "__in__" << endl;
	/*int *outimg=ImageFilter((char*) data1d, 1, in_sz[0], in_sz[1], in_sz[2]);
	int *outimg1=ImageThresh(outimg, in_sz[0], in_sz[1], in_sz[2]);*/
	int *outimg=ImageBin((char*) data1d, 200, in_sz[0], in_sz[1], in_sz[2]);
	/*
	for(int i=0;i<10;i++)
		cout<<outimg[i]<<"   ";*/

	 // display

    Image4DSimple * new4DImage = new Image4DSimple();

    new4DImage->setData((unsigned char *)outimg, in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);

    v3dhandle newwin = callback.newImageWindow();

    callback.setImage(newwin, new4DImage);

    callback.setImageName(newwin, title);

    callback.updateImageWindow(newwin);


}



bool function(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent){
return true;
}

