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
	
	freopen("D:/soamdata/somaDetection/raw.txt","w",stdout);
	for(int i=0;i<in_sz[0]*in_sz[1]*in_sz[2];i++){
		cout << (int)data1d[i] << ' ';
	}
	cout << endl;
	freopen("CON","w",stdout);

	simple_saveimage_wrapper(callback,outfiles[0],(unsigned char*)data1d,in_sz,1);


	return true;
}

bool conv(unsigned char * data1d,V3DLONG in_sz[4],V3DLONG root[4],V3DLONG xL=50,V3DLONG yL=50,V3DLONG zL=5){
	return true;
}

//bool test(){
//	else if (func_name == tr("cropTerafly"))
//	{
//		vector<char*> infiles, inparas, outfiles;
//		if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
//		if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
//		if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
//		QString length1 = inparas.at(0);
//		QString length2 = inparas.at(1);
//		QString length3 = inparas.at(2);
//		QString m_InputfolderName = infiles.at(0);
//		QString inputListFile = infiles.at(1);
//		QString savePath = infiles.at(2);
//
//
//		V3DLONG *in_zz;
//		if(!callback.getDimTeraFly(m_InputfolderName.toStdString(), in_zz))
//		{
//			v3d_msg("Cannot load terafly images.",0);
//			return false;
//		}
//
//		long cubeSideLength1 = length1.toLong();
//		long cubeSideLength2 = length2.toLong();
//		long cubeSideLength3 = length3.toLong();
//		V3DLONG in_sz[4];
//		in_sz[0] = cubeSideLength1;
//		in_sz[1] = cubeSideLength2;
//		in_sz[2] = cubeSideLength3;
//		in_sz[3] = in_zz[3];
//
//		QFile inputFile(inputListFile);
//		QTextStream in(&inputFile);
//		int count = 0;
//		if (inputFile.open(QIODevice::ReadOnly))
//		{
//			while (!in.atEnd())
//			{
//				QString line = in.readLine();
//				QList<QString> coords = line.split(",");
//				if (coords[0] == "##n") continue;
//				coords[4] = coords[4].remove(0, 1);
//				qDebug() << coords[4] << " " << coords[5] << " " << coords[6];
//
//				QList<QString> zC = coords[4].split(".");
//				QList<QString> xC = coords[5].split(".");
//				QList<QString> yC = coords[6].split(".");
//
//				long zcenter = zC[0].toLong() - 1;
//				long xcenter = xC[0].toLong() - 1;
//				long ycenter = yC[0].toLong() - 1;
//				cout << zcenter << " " << xcenter << " " << ycenter << endl;
//
//				unsigned char * cropped_image = 0;
//				cropped_image = callback.getSubVolumeTeraFly(m_InputfolderName.toStdString(),
//					(xcenter-cubeSideLength2/2), (xcenter+cubeSideLength2/2),
//					(ycenter-cubeSideLength1/2), (ycenter+cubeSideLength1/2),
//					(zcenter-cubeSideLength3/2), (zcenter+cubeSideLength3/2));
//				Image4DSimple* new4DImage = new Image4DSimple();
//				new4DImage->createImage(in_sz[0], in_sz[1], in_sz[2], in_sz[3], V3D_UINT8);
//				QString saveName = savePath + "/" + coords[5] + "_" + coords[6] + "_" + coords[4] + ".v3draw";
//				const char* fileName = saveName.toAscii();
//
//				simple_saveimage_wrapper(callback, fileName, cropped_image, in_sz, 1);
//			}
//		}
//		inputFile.close();
//
//		return true;
//	}
//}