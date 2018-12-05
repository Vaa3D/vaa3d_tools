/* ImageBgDetect_plugin.cpp
* This is a ImageBgDetect filter plugin, you can use it as a demo.
* 2018-10-13 : by Peng Wang
*/

#include "v3d_message.h"
#include <vector>
#include "ImageBgDetect_plugin.h"
#include<iostream>
#include <QString>
#include <QtCore/QFile>
using namespace std;

Q_EXPORT_PLUGIN2(ImageBgDetect,ImageBgDetectPlugin);
void gmm(V3DPluginCallback2 &callback,QWidget *parent);
bool gmm(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent);
template <class T> bool gmmFilter(T* data1d,V3DLONG *in_sz,V3DLONG c,float* &outimg);
const QString title = QObject::tr("ImageBgDetect");

QStringList ImageBgDetectPlugin::menulist() const{
	return QStringList()
		<< tr("gmmAlgorithm")
		<< tr("about");
}

QStringList ImageBgDetectPlugin::funclist() const{
	return QStringList()
		<< tr("gmmAlgorithm")
		<< tr("help");
}

void ImageBgDetectPlugin::domenu(const QString &menu_name,V3DPluginCallback2 &callback,QWidget *parent){
	if(menu_name==tr("gmmAlgorithm")){
		gmm(callback,parent);
	}
	else{
		v3d_msg(tr("developing...2018-10-12 by wp"));
	}
}

bool ImageBgDetectPlugin::dofunc(const QString &func_name,const V3DPluginArgList &input,V3DPluginArgList &output,V3DPluginCallback2 &callback,QWidget *parent){
	if(func_name==tr("gmmAlgorithm")){
		return gmm(callback,input,output,parent);
	}
	else if(func_name==tr("help")){
		return true;
	}
	return false;
}

void gmm(V3DPluginCallback2 &callback,QWidget *parent){
	v3dhandle curwin=callback.currentImageWindow();
	if(!curwin){
		v3d_msg("You don't have any image open in the main window.");
		return;
	}


	Image4DSimple *p4DImage=callback.getImage(curwin);

	if (!p4DImage)
	{
		v3d_msg("The image pointer is invalid. Ensure your data is valid and try again!");
		return;
	}
	unsigned char* data1d = p4DImage->getRawData();

	QString fileName=QString(QLatin1String(p4DImage->getFileName()));
	v3d_msg(fileName,1);

	//input
	bool ok1;
	V3DLONG c=1;
	V3DLONG in_sz[4];

	in_sz[0] = p4DImage->getXDim();
	in_sz[1] = p4DImage->getYDim();
	in_sz[2] = p4DImage->getZDim();
	in_sz[3] = p4DImage->getCDim();

	float * outimg = 0;
	bool isOk;

	v3d_msg(QString("datatype=%1").arg(int(p4DImage->getDatatype())), 1);

	switch(p4DImage->getDatatype())
	{
	case V3D_UINT8:
		isOk = gmmFilter(data1d, in_sz, c-1,outimg);
		break;
	case V3D_UINT16:
		isOk = gmmFilter((unsigned short int *)data1d, in_sz, c-1, outimg);
		break;
	case V3D_FLOAT32:
		isOk = gmmFilter((float *)data1d, in_sz, c-1, outimg);
		break;
	default:
		isOk = false;
		v3d_msg("Unsupported data type. Do nothing.");
		return;
	}

	if(!isOk){
		v3d_msg("Fail to gmm. Do nothing.");
		return;
	}

	// display
	Image4DSimple * new4DImage = new Image4DSimple();
	new4DImage->setData((unsigned char *)outimg, in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);
	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, new4DImage);
	callback.setImageName(newwin, title);
	callback.updateImageWindow(newwin);
}

template <class T> bool gmmFilter(T* data1d,V3DLONG *in_sz,V3DLONG c,float* &outimg){
	if(!data1d||!in_sz||c<0||outimg){
		return false;
	}
	V3DLONG N = in_sz[0];
	V3DLONG M = in_sz[1];
	V3DLONG P = in_sz[2];
	V3DLONG sc = in_sz[3];
	V3DLONG pagesz = N*M;
	V3DLONG channelsz = pagesz*P;

	V3DLONG offset_init = c*channelsz;

	try
	{
		outimg = new float [channelsz];
	}
	catch (...)
	{
		printf("Fail to allocate memory.\n");
		return false;
	}

	T *curdata1d = data1d + offset_init;
	/*
	for(V3DLONG i=0;i<N;i++){
	for(V3DLONG j=0;j<M;j++){
	for(V3DLONG k=0;k<P;k++){
	outimg[i*pagesz+j*N+k]=data1d[i*pagesz+j*N+k]/2;
	}
	}
	}
	*/
	QFile data("C:/Users/admin/Desktop/ImageBgDetect.txt");
	if(!data.open(QFile::WriteOnly | QIODevice::Text)){
		return -1;
	}
	
	QTextStream out(&data);

	float _min=100000000;
	float _max=-1;

	for(V3DLONG i=0;i<N*M*P;i++){
		if(data1d[i]>_max) _max=data1d[i];
		if(data1d[i]<_min) _min=data1d[i];
		out<<data1d[i]<<"\n";
	}

	v3d_msg(QString("_min=%1 _max=%2").arg(_min).arg(_max), 1);


	return true;
}

bool gmm(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent){
	return true;
}