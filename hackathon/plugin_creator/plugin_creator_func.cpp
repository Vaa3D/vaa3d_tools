/* plugin_creator_func.cpp
 * This plugin is used to produce v3d plugin project from a template file
 * 2012-01-27 : by Hang Xiao
 */

#include <vector>
#include <string>
#include <iostream>

#include <QDir>
#include <v3d_interface.h>
#include "v3d_message.h"
#include "plugin_creator_func.h"
#include "plugin_creator_gui.h"
#include "create_plugin.h"

using namespace std;
#define STRING2VECTSTRING(values, value) \
{ \
	if(value.find('"') != string::npos) \
	{\
		int pos1 = value.find('"'); \
		while(pos1 != string::npos)\
		{\
			int pos2 = value.find('"',pos1+1); \
			string str = value.substr(pos1+1, pos2 - pos1 - 1);\
			values.push_back(str);\
			pos1 = value.find('"', pos2+1);\
		}\
	}\
	else\
	{\
		int pos1 = value.find_first_not_of(' ');\
		while(pos1 != string::npos)\
		{\
			int pos2 = value.find(' ', pos1);\
			string str = value.substr(pos1, pos2-pos1);values.push_back(str);\
			pos1 = value.find_first_not_of(' ', pos2);\
		}\
	}\
} 

const QString title = QObject::tr("Plugin Creator Plugin");

int create_plugin(V3DPluginCallback2 &callback, QWidget *parent)
{
	GuidingDialog *dialog = new GuidingDialog(callback, parent);

	if (dialog->exec()!=QDialog::Accepted) return -1;
	dialog->update();

	PluginTemplate pt;
	pt.PLUGIN_NAME = dialog->plugin_name;
	pt.PLUGIN_CLASS = dialog->plugin_class;
	pt.WINDOW_TITLE = dialog->win_title;
	pt.PLUGIN_DESCRIPTION = dialog->plugin_desp;
	pt.PLUGIN_DATE = dialog->plugin_date;
	pt.PLUGIN_AUTHOR = dialog->plugin_author;
	pt.V3D_MAIN_PATH = dialog->v3dmain_path;
	STRING2VECTSTRING(pt.MENUS, dialog->menulst);
	STRING2VECTSTRING(pt.FUNCS, dialog->funclst);
	pt.DOFUNC = true;

	cout<<"menus.size() = "<<pt.MENUS.size()<<endl;
	cout<<"funcs.size() = "<<pt.FUNCS.size()<<endl;

	string save_folder = dialog->save_folder;
	if(save_folder[0] != '/' && save_folder[0] != '.') save_folder = "./" + save_folder; 

	pt.PLUGIN_HEADER = pt.PLUGIN_NAME + "_plugin.h";
	pt.PLUGIN_CPP =  pt.PLUGIN_NAME + "_plugin.cpp";
	pt.FUNC_HEADER = pt.PLUGIN_NAME + "_func.h";
	pt.FUNC_CPP =  pt.PLUGIN_NAME + "_func.cpp";
	pt.PRO_FILE =  pt.PLUGIN_NAME + ".pro";

	QDir dir(save_folder.c_str()); if(!dir.exists()){QMessageBox::warning(0, QObject::tr("Error"), QObject::tr("Un existing foler : %1").arg(save_folder.c_str())); return 0;}
	QString cur_path = QDir::current().dirName();
	cout<<"current path : "<<QDir::current().dirName().toStdString()<<endl;
	QDir::setCurrent(save_folder.c_str());
	cout<<"current path : "<<QDir::current().dirName().toStdString()<<endl;
	create_plugin_all(pt);
	QDir::setCurrent(cur_path);
	cout<<"current path : "<<QDir::current().dirName().toStdString()<<endl;

	return 1;
}


