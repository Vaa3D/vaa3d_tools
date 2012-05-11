#include <iostream>
#include <string>
#include <fstream>

using namespace std;

void produce_simplest_plugin(string dir)
{
	ofstream ofs;
	string filename;

	filename = dir + "/test.pro";
	cout<<"output "<<filename<<endl;

	ofs.open((char*)filename.c_str());
	ofs<<""<<endl;
	ofs<<"TEMPLATE	= lib"<<endl;
	ofs<<"CONFIG	+= qt plugin warn_off"<<endl;
	ofs<<"#CONFIG	+= x86_64"<<endl;
	ofs<<"VAA3D_DIR = /Users/xiaoh10/work/v3d_external"<<endl;
	ofs<<"INCLUDEPATH	+= $$VAA3D_DIR/v3d_main/basic_c_fun"<<endl;
	ofs<<""<<endl;
	ofs<<"HEADERS	= test_plugin.h"<<endl;
	ofs<<"SOURCES	+= $$VAA3D_DIR/v3d_main/basic_c_fun/v3d_message.cpp"<<endl;
	ofs<<""<<endl;
	ofs<<"TARGET	= $$qtLibraryTarget(test)"<<endl;
	ofs<<"DESTDIR	= $$VAA3D_DIR/bin/plugins/test/"<<endl;
	ofs.close();

	filename = dir + "/test_plugin.h";
	cout<<"output "<<filename<<endl;

	ofs.open((char*)filename.c_str());
	ofs<<"#ifndef __TEST_PLUGIN_H__"<<endl;
	ofs<<"#define __TEST_PLUGIN_H__"<<endl;
	ofs<<""<<endl;
	ofs<<"#include <QtGui>"<<endl;
	ofs<<"#include <v3d_interface.h>"<<endl;
	ofs<<""<<endl;
	ofs<<"class TestPlugin : public QObject, public V3DPluginInterface2_1"<<endl;
	ofs<<"{"<<endl;
	ofs<<"	Q_OBJECT"<<endl;
	ofs<<"	Q_INTERFACES(V3DPluginInterface2_1);"<<endl;
	ofs<<""<<endl;
	ofs<<"public:"<<endl;
	ofs<<"	float getPluginVersion() const {return 1.1f;}"<<endl;
	ofs<<""<<endl;
	ofs<<"	QStringList menulist() const"<<endl;
	ofs<<"	{"<<endl;
	ofs<<"		return QStringList()"<<endl;
	ofs<<"			<< tr(\"test\")"<<endl;
	ofs<<"			<< tr(\"about\");"<<endl;
	ofs<<"	}"<<endl;
	ofs<<""<<endl;
	ofs<<"	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)"<<endl;
	ofs<<"	{"<<endl;
	ofs<<"		if(menu_name == \"test\")"<<endl;
	ofs<<"		{"<<endl;
	ofs<<"		}"<<endl;
	ofs<<"		else if(menu_name == \"about\")"<<endl;
	ofs<<"		{"<<endl;
	ofs<<"			QMessageBox::information(0, \"Raw_split Plugin About\", \\"<<endl;
	ofs<<"					QObject::tr(\"This is test plugin\"));"<<endl;
	ofs<<"		}"<<endl;
	ofs<<"	}"<<endl;
	ofs<<""<<endl;
	ofs<<"	QStringList funclist() const "<<endl;
	ofs<<"	{"<<endl;
	ofs<<"		return QStringList();"<<endl;
	ofs<<"	}"<<endl;
	ofs<<""<<endl;
	ofs<<"	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)"<<endl;
	ofs<<"	{"<<endl;
	ofs<<"		return false;"<<endl;
	ofs<<"	}"<<endl;
	ofs<<"};"<<endl;
	ofs<<""<<endl;
	ofs<<"Q_EXPORT_PLUGIN2(test, TestPlugin);"<<endl;
	ofs<<""<<endl;
	ofs<<"#endif"<<endl;
	ofs<<""<<endl;
	ofs.close();
}
