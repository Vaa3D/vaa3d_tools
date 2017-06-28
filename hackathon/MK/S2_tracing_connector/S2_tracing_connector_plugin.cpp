/* S2_tracing_connector_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-6-26 : by MK
 */
 
#include "v3d_message.h"
#include <vector>
#include "S2_tracing_connector_plugin.h"
#include <iostream>
#include "S2_tracing_connector_func.h"

using namespace std;
Q_EXPORT_PLUGIN2(S2_tracing_connector, S2Connector);
 
QStringList S2Connector::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList S2Connector::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void S2Connector::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
			"Developed by MK, 2017-6-26"));
	}
}

bool S2Connector::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("combineSWC"))
	{
		QString inputFolder = infiles[0];
		QString outputFile = outfiles[0];
		
		//qDebug() << inputFolder;

		bool combine;
		combine = generatorcombined4FilesInDir(input, output);
		QString connectCommand = "C:\\Vaa3D_2010_Qt472\\v3d_external\\bin\\vaa3d_msvc.exe /x neuron_connector /f connect_neuron_SWC /i ";
		connectCommand = connectCommand + outputFile + " /o " + inputFolder + "\\combined_connected.swc /p 60 10 1 1 1 1 true -1";
		string c_command = connectCommand.toStdString().c_str();
		const char* command = c_command.c_str();
		cout << command << endl;
		system(command);

		return true;
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

