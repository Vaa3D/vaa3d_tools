/* Dataset_Generator_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-9-26 : by MK
 */
 
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "v3d_message.h"

#include "datasetGeneratorUI.h"
#include "Dataset_Generator_plugin.h"
#include "Analyzer.h"

using namespace std;
Q_EXPORT_PLUGIN2(Dataset_Generator, DatasetGenerator);
 
QStringList DatasetGenerator::menulist() const
{
	return QStringList() 
		<<tr("Start Dataset Generator")
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
	if (menu_name == tr("Start Dataset Generator"))
	{
		DatasetGeneratorUI* inputForm = new DatasetGeneratorUI(0, &callback);
		inputForm->exec();
		inputForm->~DatasetGeneratorUI();

		return;
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

	if (func_name == tr("datasetHistProfile"))
	{
		QString QinputPath = infiles.at(0);
		string inputPath = QinputPath.toStdString();
		Analyzer analyzer1;
		analyzer1.analyzerCallback = &callback;
		map<size_t, vector<float>> test;
		analyzer1.imgSetHistProfile(inputPath, test);
		vector<float> means = analyzer1.patchMeans;
		vector<float> stds = analyzer1.patchVars;
		
		const char* fileName = "axonMeans.txt";
		const char* stdFileName = "axonStds.txt";
		ofstream outputFile(fileName);
		ofstream outputStd(stdFileName);
		int count = 1;
		vector<float>::iterator stdIt = stds.begin();
		for (vector<float>::iterator it = means.begin(); it != means.end(); ++it)
		{
			cout << count << " ";
			string meanValue = to_string(*it);
			string stdValue = to_string(*stdIt);
			outputFile << meanValue << " ";
			outputStd << stdValue << " ";
			++stdIt;
			++count;
		}

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

