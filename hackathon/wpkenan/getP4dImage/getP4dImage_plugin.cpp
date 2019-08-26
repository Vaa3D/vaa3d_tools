/*
getP4dImage_plugin.cpp
Sun Oct 14 11:47:56 2018 by wp
*/

#include "v3d_message.h"
#include <vector>
#include "getP4dImage_plugin.h"
#include<iostream>
#include <QString>
#include <QtCore/QFile>
using namespace std;

Q_EXPORT_PLUGIN2(getP4dImage,getP4dImagePlugin);
void function(V3DPluginCallback2 &callback,QWidget *parent);
bool function(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent);
const QString title = QObject::tr("getP4dImage");

QStringList getP4dImagePlugin::menulist() const{
   return QStringList()
       << tr("function")
       << tr("about");
}

QStringList getP4dImagePlugin::funclist() const{
   return QStringList()
       << tr("function")
       << tr("help");
}

void getP4dImagePlugin::domenu(const QString &menu_name,V3DPluginCallback2 &callback,QWidget *parent){
   if(menu_name==tr("function")){
       function(callback,parent);
   }
   else{
       v3d_msg(tr("developing...Sun Oct 14 11:47:56 2018 by wp"));
   }
}

bool getP4dImagePlugin::dofunc(const QString &func_name,const V3DPluginArgList &input,V3DPluginArgList &output,V3DPluginCallback2 &callback,QWidget *parent){
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

