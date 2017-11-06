/* Dataset_Generator_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-9-26 : by MK
 */
 
#include "v3d_message.h"
#include <vector>
#include "Dataset_Generator_plugin.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "datasetGeneratorUI.h"

using namespace std;
Q_EXPORT_PLUGIN2(Dataset_Generator, DatasetGenerator);
 
QStringList DatasetGenerator::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList DatasetGenerator::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void DatasetGenerator::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		DatasetGeneratorUI* inputForm = new DatasetGeneratorUI(0, &callback);
		inputForm->exec();
		inputForm->~DatasetGeneratorUI(); // Needs to force freeing up memory for Win7..? (Win10 doesn't need this.)

		return;
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by MK, 2017-9-26"));
	}
}

bool DatasetGenerator::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("pickup"))
	{
		this->inputSWCdir = infiles.at(0);
		this->inputImagedir = infiles.at(1);
		//this->inputBkgDir = infiles.at(2);
		//this->outputImagedir = outfiles.at(0);

		getImageFolders();
		//pick_save();
		//createList();
		//createList2();

		shapeFilter_line_dot();
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

