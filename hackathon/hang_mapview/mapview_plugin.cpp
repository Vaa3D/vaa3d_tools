/* mapview_plugin.cpp
 * Used to load large data
 * 2012-03-16 : by Hang Xiao & Jianlong Zhou
 */
#include <iostream> 
#include <fstream>

#include "v3d_message.h"
#include "mapview_plugin.h"
#include "mapview_gui.h"
 
using namespace std;

Q_EXPORT_PLUGIN2(mapview, MapViewPlugin);

int load_hraw_data(V3DPluginCallback2 &callback, QWidget *parent);
int help(V3DPluginCallback2 &callback, QWidget *parent);
 
QStringList MapViewPlugin::menulist() const
{
	return QStringList() 
		<<tr("load hraw data")
		<<tr("create hraw data")
		<<tr("about");
}

QStringList MapViewPlugin::funclist() const
{
	return QStringList()
		<<tr("load_hraw_data")
		<<tr("create_hraw_data")
		<<tr("help");
}

void MapViewPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("load hraw data"))
	{
		load_hraw_data(callback,parent);
	}
	else if(menu_name == tr("create hraw data"))
	{
	}
	else
	{
		v3d_msg(tr("Used to load large data. "
			"Developed by Hang Xiao, 2012-03-16"));
	}
}

bool MapViewPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("load_hraw_data"))
	{
		//return load_hraw_data(input, output);
	}
	else if(func_name == tr("create_hraw_data"))
	{
	}
	else if (func_name == tr("help"))
	{
		//return help(input,output);
	}
	return false;
}

static string toupper_case(string instr)
{
	string outstr = instr;
	for(int i = 0; i < instr.size(); i++)
	{
		char c = instr[i];
		if(c >= 'a' && c <= 'z')
		{
			c = c - 'a' + 'A';
		}
		outstr[i] = c;
	}
	return outstr;
}

int load_hraw_data(V3DPluginCallback2 &callback, QWidget *parent)
{
	QString hraw_file = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                                                 ".",
                                                 QObject::tr("Images (*.hraw)"));
	cout<<"hraw_file = "<<hraw_file.toStdString().c_str()<<endl;
	ifstream ifs(hraw_file.toStdString().c_str());

	string dir;
	V3DLONG L = 0, M = 0, N = 0;
	V3DLONG l = 0, m = 0, n = 0;
	V3DLONG channel = 0;
	int level_num = 0;
	while(ifs.good())
	{
		string name;
		ifs >> name; name = toupper_case(name);
		if(name == "PATH") ifs >> dir;
		else if(name == "L0_X_BLOCKS") ifs >> L;
		else if(name == "L0_Y_BLOCKS") ifs >> M;
		else if(name == "L0_Z_BLOCKS") ifs >> N;
		else if(name == "LEVEL_NUM") ifs >> level_num;
	}
	string raw0000 = dir + "/L0/L0_X0_Y0_Z0.raw";
	cout<<"raw0000 = "<<raw0000<<endl;
	getRawImageSize(raw0000, l, m, n, channel);

	cout<<"dir = "<<dir<<endl;
	cout<<"L = "<<L<<" M = "<<M<<" N = "<<N<<endl;
	cout<<"l = "<<l<<" m = "<<m<<" n = "<<n<<endl;
	cout<<"channel = "<<channel<<endl;
	cout<<"level_num = "<<level_num<<endl;

	Mapview_Paras mapview_paras;
	mapview_paras.hraw_dir = dir.c_str();
	mapview_paras.L = L;
	mapview_paras.M = M;
	mapview_paras.N = N;
	mapview_paras.l = l;
	mapview_paras.m = m;
	mapview_paras.n = n;
	mapview_paras.channel = channel;
	mapview_paras.level_num = level_num;
	mapview_paras.outsz[0] = 128;
	mapview_paras.outsz[1] = 63;
	mapview_paras.outsz[2] = 32;

	MapViewWidget * mapview_widget = new MapViewWidget(&callback, mapview_paras, 0);
	mapview_widget->show();
	
	return 1;
}

int help(V3DPluginCallback2 &callback, QWidget *parent)
{
	return 1;
}
