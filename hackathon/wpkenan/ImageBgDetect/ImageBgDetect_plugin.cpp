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
template <class T> bool gmmFilter(QString fileName,T* data1d,V3DLONG *in_sz,V3DLONG c,float* &outimg,QWidget *parent);

void adptThreshold(V3DPluginCallback2 &callback,QWidget *parent);
bool adptThreshold(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent);
template <class T> bool adptThresholdFilter(QString fileName,T* data1d,V3DLONG *in_sz,V3DLONG c,float* &outimg,QWidget *parent);
const QString title = QObject::tr("ImageBgDetect");

QStringList ImageBgDetectPlugin::menulist() const{
	return QStringList()
		<< tr("gmmAlgorithm")
		<< tr("adptThreshold")
		<< tr("exportGrayValueToTxt")
		<< tr("about");
}

QStringList ImageBgDetectPlugin::funclist() const{
	return QStringList()
		<< tr("gmmAlgorithm")
		<< tr("adptThreshold")
		<< tr("exportGrayValueToTxt")
		<< tr("help");
}

void ImageBgDetectPlugin::domenu(const QString &menu_name,V3DPluginCallback2 &callback,QWidget *parent){
	if(menu_name==tr("gmmAlgorithm")){
		gmm(callback,parent);
	}
	else if(menu_name==tr("adptThreshold")){
		adptThreshold(callback,parent);
	}
	else{
		v3d_msg(tr("developing...2018-10-12 by wp"));
	}
}

bool ImageBgDetectPlugin::dofunc(const QString &func_name,const V3DPluginArgList &input,V3DPluginArgList &output,V3DPluginCallback2 &callback,QWidget *parent){
	if(func_name==tr("gmmAlgorithm")){
		return gmm(callback,input,output,parent);
	}
	else if(func_name==tr("adptThreshold")){
		return adptThreshold(callback,input,output,parent);
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
	v3d_msg(fileName,0);
	v3d_msg(QString("datatype=%1").arg(int(p4DImage->getDatatype())), 0);
	
	//input
	V3DLONG c=1;
	V3DLONG in_sz[4];
	in_sz[0] = p4DImage->getXDim();
	in_sz[1] = p4DImage->getYDim();
	in_sz[2] = p4DImage->getZDim();
	in_sz[3] = p4DImage->getCDim();

	float * outimg = 0;
	bool isOk;

	switch(p4DImage->getDatatype())
	{
	case V3D_UINT8:
		isOk = gmmFilter(fileName,data1d, in_sz, c-1,outimg,parent);
		break;
	case V3D_UINT16:
		isOk = gmmFilter(fileName,(unsigned short int *)data1d, in_sz, c-1, outimg,parent);
		break;
	case V3D_FLOAT32:
		isOk = gmmFilter(fileName,(float *)data1d, in_sz, c-1, outimg,parent);
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
	new4DImage->setFileName("C:/Users/admin/Desktop/wp_data..v3draw.v3dpbd");
	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, new4DImage);
	callback.setImageName(newwin, title);
	callback.updateImageWindow(newwin);
}

template <class T> bool gmmFilter(QString fileName,T* data1d,V3DLONG *in_sz,V3DLONG c,float* &outimg,QWidget *parent){
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

	//exportGrayValueToTxt
	QString rawStr=fileName.split(".")[0];
	QFile data(rawStr+"_gray.txt");
	if(!data.open(QFile::WriteOnly | QIODevice::Text)){
		return -1;
	}
	QTextStream out(&data);
	float _min=100000000;
	float _max=-1;
	for(V3DLONG i=0;i<N*M*P;i++){
		//if(data1d[i]>_max) _max=data1d[i];
		//if(data1d[i]<_min) _min=data1d[i];
		out<<data1d[i]<<"\n";
	}
	data.close();
	//v3d_msg(QString("_min=%1 _max=%2").arg(_min).arg(_max), 0);


	//QFile classByGmmFile("C:/Users/admin/Desktop/ImageBgDetect_gmm.txt");
	

	/*QStringList args;
	args.append("D:/wpkenan/Project/Pycharm/20180924/gmmSklearn.py");
	args.append(rawStr+"_gray.txt");
	QProcess::execute(QString("Python.exe"), args);*/

	
	V3DLONG threshold = 0;
	threshold=QInputDialog::getInteger(parent, "Channel",
		"Enter threshold # (starts from 0):",
		0, 0, 100, 1);

	QFile inputFile(rawStr+"_gray_gmm.txt");
	if (inputFile.open(QIODevice::ReadOnly|QIODevice::Text))
	{
		QTextStream in(&inputFile);
		int i=0;
		while (!in.atEnd())
		{
			QString line = in.readLine();

			if(line.toInt()>=threshold){
				outimg[i++]=line.toInt()*20;
			}else{
				outimg[i++]=0;
			}
			
		}
		inputFile.close();
	}


	return true;
}

bool gmm(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent){
	return true;
}


void adptThreshold(V3DPluginCallback2 &callback,QWidget *parent){
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
	v3d_msg(fileName,0);
	v3d_msg(QString("datatype=%1").arg(int(p4DImage->getDatatype())), 0);
	
	//input
	V3DLONG c=1;
	V3DLONG in_sz[4];
	in_sz[0] = p4DImage->getXDim();
	in_sz[1] = p4DImage->getYDim();
	in_sz[2] = p4DImage->getZDim();
	in_sz[3] = p4DImage->getCDim();

	float * outimg = 0;
	bool isOk;

	switch(p4DImage->getDatatype())
	{
	case V3D_UINT8:
		isOk = adptThresholdFilter(fileName,data1d, in_sz, c-1,outimg,parent);
		break;
	case V3D_UINT16:
		isOk = adptThresholdFilter(fileName,(unsigned short int *)data1d, in_sz, c-1, outimg,parent);
		break;
	case V3D_FLOAT32:
		isOk = adptThresholdFilter(fileName,(float *)data1d, in_sz, c-1, outimg,parent);
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
	new4DImage->setFileName("C:/Users/admin/Desktop/wp_data..v3draw.v3dpbd");
	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, new4DImage);
	callback.setImageName(newwin, title);
	callback.updateImageWindow(newwin);
}
bool adptThreshold(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent){
	return true;
}
template <class T> bool adptThresholdFilter(QString fileName,T* data1d,V3DLONG *in_sz,V3DLONG c,float* &outimg,QWidget *parent){
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

	map<T,V3DLONG> grayCounts;
	for(V3DLONG i=0;i<N*M*P;i++){
		grayCounts[int(data1d[i])]++;
	}
	QString strTmp=QString("%1 grayLevel in all\n").arg(grayCounts.size());
	map<T,V3DLONG>::iterator iter=grayCounts.begin();
	V3DLONG tmp=iter->second;
	for(;iter!=grayCounts.end();iter++){
		//cout << iter->first << " " << iter->second << endl;
		strTmp+=QString("%1      %2      %3\n").arg(iter->first,10).arg(iter->second,20).arg(iter->second*1.0/tmp,10);
		tmp=iter->second;
	}
	v3d_msg(strTmp);



	return true;
}