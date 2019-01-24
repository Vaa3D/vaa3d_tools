/*
highlightAnormaly_plugin.cpp
Thu Jan 24 14:27:50 2019 by wp
*/

#include "v3d_message.h"
#include <vector>
#include "highlightAnormaly_plugin.h"
#include<iostream>
#include <QString>
#include <QtCore/QFile>
using namespace std;

Q_EXPORT_PLUGIN2(highlightAnormaly,highlightAnormalyPlugin);
void function(V3DPluginCallback2 &callback,QWidget *parent);
bool function(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent);
bool highlightAnormaly(NeuronTree &nt,double min_angle,double max_angle,char* outfile,int length);
double calculateAngle(NeuronTree nt,int par,int ch1,int ch2);
bool printHelp();
const QString title = QObject::tr("highlightAnormaly");

QStringList highlightAnormalyPlugin::menulist() const{
   return QStringList()
       << tr("function")
       << tr("about");
}

QStringList highlightAnormalyPlugin::funclist() const{
   return QStringList()
       << tr("function")
       << tr("help");
}

void highlightAnormalyPlugin::domenu(const QString &menu_name,V3DPluginCallback2 &callback,QWidget *parent){
   if(menu_name==tr("function")){
       function(callback,parent);
   }
   else{
       v3d_msg(tr("developing...Thu Jan 24 14:27:50 2019 by wp"));
   }
}

bool highlightAnormalyPlugin::dofunc(const QString &func_name,const V3DPluginArgList &input,V3DPluginArgList &output,V3DPluginCallback2 &callback,QWidget *parent){
   if(func_name==tr("function")){
       return function(callback,input,output,parent);
   }
   else if(func_name==tr("help")){
	   return printHelp();
   }
   return false;
}

void function(V3DPluginCallback2 &callback,QWidget *parent){

}

bool function(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent){
	vector<char*> *pinfiles=(input.size()>=1)?(vector<char*> *) input[0].p : 0;
	vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
	vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
	vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
	vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
	vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

	int rootid=1;
	double thres=1;
	double min_angle=30;
	double max_angle=150;
	int length=3;
	if(paras.size()>0){
		rootid=atoi(paras[0]);
	}
	if(paras.size()>1){
		min_angle=atoi(paras[1]);
	}
	if(paras.size()>2){
		max_angle=atoi(paras[2]);
	}
	if(paras.size()>3){
		length=atoi(paras[3]);
	}
	if(paras.size()>4){
		cout << "Error tool many paras!!!" << endl;
		return false;
	}

	QDir dir;

	//1. get a sorted, single-rooted NeuronTree
	QString cmd_sortSwc=QString("%1 /x sort_neuron_swc /f sort_swc /i %2 /o %3 /p %4 %5")
		.arg((dir.currentPath().toStdString()+"//..//..//bin//vaa3d_msvc.exe").c_str())
		.arg(infiles[0])
		.arg((QString(infiles[0])+"_sortedTmp.swc").toStdString().c_str())
		.arg(0)
		.arg(paras[0]);
	system(qPrintable(cmd_sortSwc));


	NeuronTree nt;
	nt=readSWC_file(QString(infiles[0])+"_sortedTmp.swc");

	//2. highlight abnormal angles
	highlightAnormaly(nt,30,150,outfiles[0],length);

	//3. replace original VR nt with this nt

	return true;
}

bool highlightAnormaly(NeuronTree &nt,double min_angle,double max_angle,char* outfile,int length){

	cout << nt.listNeuron[0].pn << endl;

	vector<vector<int>> children;
	children.resize(nt.listNeuron.size()+1);
	cout << nt.listNeuron.size() << endl;
	for(int i=0;i<nt.listNeuron.size();i++){
		if(nt.listNeuron[i].pn!=-1){
			children[nt.listNeuron[i].pn].push_back(i+1);
		}
	}

	for(int i=1;i<=nt.listNeuron.size();i++){
		if(children[i].size()==2){//it's a bifurcation
			//nt.listNeuron[i].radius=10;
			double angle=calculateAngle(nt,i-1,children[i][0]-1,children[i][1]-1);
			cout << angle << endl;
			cout << "*******" << endl;
			if(angle<min_angle){
				int tmpLength=length;
				int tmpIndex=children[i][0];
				//nt.listNeuron[i-1].type=5;
				//nt.listNeuron[children[i][0]].type=5;

				while(tmpLength--){
					cout << tmpIndex << endl;
					nt.listNeuron[tmpIndex-1].type=5;
					if(children[tmpIndex].size()==0) break;
					tmpIndex=children[tmpIndex][0];
					
				}

				//nt.listNeuron[children[i][1]].type=5;
				tmpLength=length;
				tmpIndex=children[i][1];

				while(tmpLength--){
					cout << tmpIndex << endl;
					nt.listNeuron[tmpIndex-1].type=5;
					if(children[tmpIndex].size()==0) break;
					tmpIndex=children[tmpIndex][0];
				}
				//nt.listNeuron[i].radius=4;
			}
			if(angle>max_angle){
				////nt.listNeuron[i-1].type=6;
				//nt.listNeuron[children[i][0]].type=6;
				//nt.listNeuron[children[i][1]].type=6;
				////nt.listNeuron[i].radius=8;

				int tmpLength=length;
				int tmpIndex=children[i][0];

				while(tmpLength--){
					cout << tmpIndex << endl;
					nt.listNeuron[tmpIndex-1].type=6;
					if(children[tmpIndex].size()==0) break;
					tmpIndex=children[tmpIndex][0];

				}

				tmpLength=length;
				tmpIndex=children[i][1];

				while(tmpLength--){
					cout << tmpIndex << endl;
					nt.listNeuron[tmpIndex-1].type=6;
					if(children[tmpIndex].size()==0) break;
					tmpIndex=children[tmpIndex][0];
				}
				
			}
		}
	}


	writeSWC_file(QString(outfile),nt);
	return true;
}

double calculateAngle(NeuronTree nt,int parIndex,int ch1Index,int ch2Index){
	NeuronSWC par=nt.listNeuron[parIndex];
	NeuronSWC ch1=nt.listNeuron[ch1Index];
	NeuronSWC ch2=nt.listNeuron[ch2Index];


	//set par point as (0,0,0)
	double x1=ch1.x-par.x;
	double y1=ch1.y-par.y;
	double z1=ch1.z-par.z;

	double x2=ch2.x-par.x;
	double y2=ch2.y-par.y;
	double z2=ch2.z-par.z;

	
	double theta=acos((x1*x2+y1*y2+z1*z2)/sqrt(x1*x1+y1*y1+z1*z1)/sqrt(x2*x2+y2*y2+z2*z2))/3.1415926*180;
	cout << parIndex+1 << ' ' << ch1Index+1 << ' ' << ch2Index+1 << endl;
	cout << x1 << ' ' << y1 << ' ' << z1 << endl;
	cout << x2 << ' ' << y2 << ' ' << z2 << endl;


	return theta;

}

bool printHelp(){
	
	cout << "\n*******************************************" << endl ;
	cout << "Demo:" << endl;
	cout << "vaa3d_msvc.exe -x highlightAnormaly.dll -f function -i 1.swc -o result.swc -p 221 30 150 3" << endl;
	return true;
}