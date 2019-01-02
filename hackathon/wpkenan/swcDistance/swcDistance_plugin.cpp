/*
swcDistance_plugin.cpp
Wed Jan  2 18:11:10 2019 by wp
*/

#include "v3d_message.h"
#include <vector>
#include "swcDistance_plugin.h"
#include<iostream>
#include <QString>
#include <QtCore/QFile>
//#include "my_surf_objs.h"
using namespace std;

Q_EXPORT_PLUGIN2(swcDistance,swcDistancePlugin);
void function(V3DPluginCallback2 &callback,QWidget *parent);
bool function(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent);
const QString title = QObject::tr("swcDistance");

QStringList swcDistancePlugin::menulist() const{
   return QStringList()
       << tr("function")
       << tr("about");
}

QStringList swcDistancePlugin::funclist() const{
   return QStringList()
       << tr("function")
       << tr("help");
}

void swcDistancePlugin::domenu(const QString &menu_name,V3DPluginCallback2 &callback,QWidget *parent){
   if(menu_name==tr("function")){
       function(callback,parent);
   }
   else{
       v3d_msg(tr("developing...Wed Jan  2 18:11:10 2019 by wp"));
   }
}

bool swcDistancePlugin::dofunc(const QString &func_name,const V3DPluginArgList &input,V3DPluginArgList &output,V3DPluginCallback2 &callback,QWidget *parent){
   if(func_name==tr("function")){
       return function(callback,input,output,parent);
   }
   else if(func_name==tr("help")){
       return true;
   }
   return false;
}

void function(V3DPluginCallback2 &callback,QWidget *parent){

}

bool function(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent){
	cout << "welcome to swc_dist_io" << endl;
	
	vector<char*> *pinfiles=(input.size()>=1)?(vector<char*> *) input[0].p : 0;
	vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
	vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
	vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
	vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
	vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

	//cout << (infiles.size()) << endl;
	if(infiles.size()!=2||((string)infiles[0]).substr(((string)infiles[0]).length()-4,((string)infiles[0]).length()-1)!=".swc"||((string)infiles[1]).substr(((string)infiles[1]).length()-4,((string)infiles[1]).length()-1)!=".swc"){
		cout << "Input files must be 2 *.swc Files!" << endl;
		cout << "Your input files error" << endl;
		return false;
	}
	string name_nt1(infiles[0]);
	string name_nt2(infiles[1]);
	//NeuronTree nt1=readSWC_file(name_nt1);
	//NeuronTree nt2=readSWC_file(name_nt2);
	
	vector<MyMarker*> nt1=readSWC_file(name_nt1);


	
   

	return true;
}

