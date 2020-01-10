/* getTeraflyBlock_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-12-5 : by wp
 */
 
#include "v3d_message.h"
#include <vector>
#include "getTeraflyBlock_plugin.h"
#include "iostream"
using namespace std;
Q_EXPORT_PLUGIN2(getTeraflyBlock, getTeraflyBlockPlugin);
 
QStringList getTeraflyBlockPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList getTeraflyBlockPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void getTeraflyBlockPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by wp, 2019-12-5"));
	}
}

bool getTeraflyBlockPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("getSubBlock"))
	{

		
		QString tmp("Z:\\wpkenan\\10000\\result_multimarkers_unet\\17052\\marker\\10060_2703.119_8825.112_6017.92.v3draw.marker_tmp_APP2");
		//QDir(tmp);
		cout << "helloo" << endl;
		cout << tmp.toStdString().data() << endl;
		


		QString str1,str2,str3;
		str1 = tmp.section("\\", -1, -1);
		str2 = tmp.section("\\", -2, -2);
		str3 = tmp.section("\\", -3, -3);
		cout << str1.toStdString().data() << endl;
		cout << str2.toStdString().data() << endl;
		cout << str3.toStdString().data() << endl;
		
		//callback.getSubVolumeTeraFly(inimg_file,)
		////v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

