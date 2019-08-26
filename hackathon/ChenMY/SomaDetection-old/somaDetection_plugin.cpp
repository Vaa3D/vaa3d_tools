/*
somaDetection_plugin.cpp
Sun Dec  9 21:48:00 2018 by wp
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
       v3d_msg(tr("developing...Sun Dec  9 21:48:00 2018 by wp"));
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

}

bool function(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent){
return true;
}

