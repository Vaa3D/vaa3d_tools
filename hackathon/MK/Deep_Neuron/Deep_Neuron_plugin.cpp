/* Deep_Neuron_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-12-12 : by MK
 */
 
#include <vector>
#include <iostream>

#include "v3d_message.h"
#include "Deep_Neuron_plugin.h"
#include "DeepNeuronUI.h"
#include "tester.h"

using namespace std;

Q_EXPORT_PLUGIN2(Deep_Neuron, DeepNeuron_plugin);
 
QStringList DeepNeuron_plugin::menulist() const
{
	return QStringList() 
		<<tr("start Deep Neuron application")
		<<tr("about");
}

QStringList DeepNeuron_plugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void DeepNeuron_plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("start Deep Neuron application"))
	{
		DeepNeuronUI* inputForm = new DeepNeuronUI(0, &callback);
		connect(this, SIGNAL(callUpUI()), inputForm, SLOT(uiCall()));
		emit callUpUI();
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by MK, 2017-12-12"));
	}
}

bool DeepNeuron_plugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		Tester tester1;
		tester1.inputFileStrings.push_back(infiles[0]); // image
		tester1.inputFileStrings.push_back(infiles[1]); // marker
		tester1.inputStrings.push_back(inparas[0]);     // deploy
		tester1.inputStrings.push_back(inparas[1]);     // model
		tester1.inputStrings.push_back(inparas[2]);     // mean
		tester1.outputStrings.push_back(outfiles[0]);   // output swc

		tester1.theCallbackPtr = &callback;
		tester1.test1();
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

