/*
statisticalGrayValue_plugin.cpp
Sun Oct 14 19:43:57 2018 by wp
*/

#include "v3d_message.h"
#include <vector>
#include "statisticalGrayValue_plugin.h"
#include<iostream>
#include <QString>
#include <QtCore/QFile>
#include <map>
using namespace std;

Q_EXPORT_PLUGIN2(statisticalGrayValue,statisticalGrayValuePlugin);
void statistical(V3DPluginCallback2 &callback,QWidget *parent);
bool statistical(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent);
template <class T> bool statisticalProcess(T* data1d,V3DLONG *in_sz,V3DLONG c);
const QString title = QObject::tr("statisticalGrayValue");

QStringList statisticalGrayValuePlugin::menulist() const{
   return QStringList()
       << tr("statistical")
       << tr("about");
}

QStringList statisticalGrayValuePlugin::funclist() const{
   return QStringList()
       << tr("statistical")
       << tr("help");
}

void statisticalGrayValuePlugin::domenu(const QString &menu_name,V3DPluginCallback2 &callback,QWidget *parent){
   if(menu_name==tr("statistical")){
       statistical(callback,parent);
   }
   else{
       v3d_msg(tr("developing...Sun Oct 14 19:43:57 2018 by wp"));
   }
}

bool statisticalGrayValuePlugin::dofunc(const QString &func_name,const V3DPluginArgList &input,V3DPluginArgList &output,V3DPluginCallback2 &callback,QWidget *parent){
   if(func_name==tr("statistical")){
       return statistical(callback,input,output,parent);
   }
   else if(func_name==tr("help")){
       return true;
   }
   return false;
}

void statistical(V3DPluginCallback2 &callback,QWidget *parent){
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
	//v3d_msg(QString(QLatin1String(p4DImage->getFileName())),1);
	//v3d_msg(QString("datatype=%1").arg(int(p4DImage->getDatatype())), 1);

	//input
	V3DLONG c=1;
	V3DLONG in_sz[4];
	in_sz[0] = p4DImage->getXDim();
	in_sz[1] = p4DImage->getYDim();
	in_sz[2] = p4DImage->getZDim();
	in_sz[3] = p4DImage->getCDim();

	bool isOk;
	switch(p4DImage->getDatatype())
	{
	case V3D_UINT8:
		isOk = statisticalProcess(data1d, in_sz, c-1);
		break;
	case V3D_UINT16:
		isOk = statisticalProcess((unsigned short int *)data1d, in_sz, c-1);
		break;
	case V3D_FLOAT32:
		isOk = statisticalProcess((float *)data1d, in_sz, c-1);
		break;
	default:
		isOk = false;
		v3d_msg("Unsupported data type. Do nothing.");
		return;
	}

	if(!isOk){
		v3d_msg("Fail to statisticalProcess. Do nothing.");
		return;
	}
	
	//display

}

bool statistical(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent){
	return true;
}


template <class T> bool statisticalProcess(T* data1d,V3DLONG *in_sz,V3DLONG c){
	//v3d_msg("goInto statisticalProcess.");
	if(!data1d||!in_sz||c<0){
		return false;
	}
	V3DLONG N = in_sz[0];
	V3DLONG M = in_sz[1];
	V3DLONG P = in_sz[2];
	V3DLONG sc = in_sz[3];
	V3DLONG pagesz = N*M;
	V3DLONG channelsz = pagesz*P;
	V3DLONG offset_init = c*channelsz;

	map<T,V3DLONG> grayCounts;
	for(V3DLONG i=0;i<N*M*P;i++){
		grayCounts[data1d[i]]++;
	}
	QString strTmp=QString("%1 grayLevels in all\nGraylevel   Counts\n").arg(grayCounts.size());
	map<T,V3DLONG>::iterator iter=grayCounts.begin();
	int count=0;
	for(iter=grayCounts.begin();iter!=grayCounts.end();iter++){
		//cout << iter->first << " " << iter->second << endl;
		count++;
		strTmp+=QString("%1       %2\n").arg(iter->first,3).arg(iter->second,10);
		if(count>52){
			break;
		}
	}
	if(count!=0)v3d_msg(strTmp);

	count=0;
	strTmp.clear();
	strTmp=QString("%1 grayLevels in all\nGraylevel   Counts\n").arg(grayCounts.size());
	for(;iter!=grayCounts.end();iter++){
		//cout << iter->first << " " << iter->second << endl;
		count++;
		strTmp+=QString("%1       %2\n").arg(iter->first,3).arg(iter->second,10);
		if(count>52){
			break;
		}
	}
	if(count!=0)v3d_msg(strTmp);

	count=0;
	strTmp.clear();
	strTmp=QString("%1 grayLevels in all\nGraylevel   Counts\n").arg(grayCounts.size());
	for(;iter!=grayCounts.end();iter++){
		//cout << iter->first << " " << iter->second << endl;
		count++;
		strTmp+=QString("%1       %2\n").arg(iter->first,3).arg(iter->second,10);
		if(count>52){
			break;
		}
	}
	if(count!=0)v3d_msg(strTmp);
	

	count=0;
	strTmp.clear();
	strTmp=QString("%1 grayLevels in all\nGraylevel   Counts\n").arg(grayCounts.size());
	for(;iter!=grayCounts.end();iter++){
		//cout << iter->first << " " << iter->second << endl;
		count++;
		strTmp+=QString("%1       %2\n").arg(iter->first,3).arg(iter->second,10);
		if(count>52){
			break;
		}
	}
	if(count!=0)v3d_msg(strTmp);

	count=0;
	strTmp.clear();
	strTmp=QString("%1 grayLevels in all\nGraylevel   Counts\n").arg(grayCounts.size());
	for(;iter!=grayCounts.end();iter++){
		//cout << iter->first << " " << iter->second << endl;
		count++;
		strTmp+=QString("%1       %2\n").arg(iter->first,3).arg(iter->second,10);
		if(count>52){
			break;
		}
	}
	if(count!=0)v3d_msg(strTmp);


	return true;
}