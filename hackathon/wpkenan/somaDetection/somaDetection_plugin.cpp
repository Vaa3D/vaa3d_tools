/*
somaDetection_plugin.cpp
Mon Dec 24 20:19:29 2018 by wp
*/

#include "v3d_message.h"
#include <vector>
#include "somaDetection_plugin.h"
#include<iostream>
#include <QString>
#include <QtCore/QFile>
using namespace std;

Q_EXPORT_PLUGIN2(somaDetection,somaDetectionPlugin);
void function(V3DPluginCallback2 &callback,QWidget *parent);
bool somadetect(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent);
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
       v3d_msg(tr("developing...Mon Dec 24 20:19:29 2018 by wp"));
   }
}

bool somaDetectionPlugin::dofunc(const QString &func_name,const V3DPluginArgList &input,V3DPluginArgList &output,V3DPluginCallback2 &callback,QWidget *parent){
   if(func_name==tr("somadetect")){
	   cout << "*****in*********" << endl;
       return somadetect(callback,input,output,parent);
   }
   else if(func_name==tr("help")){
       return true;
   }
   return false;
}

void function(V3DPluginCallback2 &callback,QWidget *parent){

}

bool somadetect(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent){
	//cout << input[0].type.toStdString() << endl;
	//cout << input[1].type.toStdString() << endl;
	vector<char*> *pinfiles=(input.size()>=1)?(vector<char*> *) input[0].p : 0;
	vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
	vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
	vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
	vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
	vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();
	//cout << __FUNCTION__ << endl;
	//cout <<  "infiles: " << infiles[0] << endl;
	//cout << "outfiles: " << outfiles[0] << endl;
	//char *a="dsadas";
	//QString tmp = a;
	//cout << tmp.toStdString() << endl;
	if(infiles.empty()){
		cerr<<"Need input image"<<endl;
		return false;
	}
	QString inimg_file=infiles[0];

	V3DLONG c=1;
	V3DLONG in_sz[4];
	
	Image4DSimple *p4dImage=callback.loadImage((char *)(qPrintable(inimg_file)));
	if(!p4dImage||!p4dImage->valid()){
		return false;
	}

	in_sz[0]=p4dImage->getXDim();
	in_sz[1]=p4dImage->getYDim();
	in_sz[2]=p4dImage->getZDim();
	in_sz[3]=p4dImage->getCDim();
	
	unsigned char* data1d=p4dImage->getRawData();
	
	simple_saveimage_wrapper(callback,outfiles[0],(unsigned char*)data1d,in_sz,1);

	
	

	return true;
}

