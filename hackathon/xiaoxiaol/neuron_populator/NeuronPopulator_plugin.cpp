/* NeuronPopulator_plugin.cpp
 * Use this plugin to simulate a group of neurons with desired density. This simulator also counts the contacts between axons and dendrites.
 * 2015-8-5 : by Xiaoxiao Liu
 */
 
#include "v3d_message.h"
#include <vector>
#include "NeuronPopulator_plugin.h"
#include "populate_neurons.h"
#include <iostream>

using namespace std;
Q_EXPORT_PLUGIN2(NeuronPopulator, NeuronPopulator);



 
QStringList NeuronPopulator::menulist() const
{
	return QStringList() 
		<<tr("populate")
		<<tr("detect")
		<<tr("about");
}

QStringList NeuronPopulator::funclist() const
{
	return QStringList()
		<<tr("populate")
		<<tr("detect")
		<<tr("help");
}

void NeuronPopulator::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("populate"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("detect"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
        v3d_msg(tr("Use this plugin to simulate a group of neurons with desired density. This simulator also detects and counts the contacts between axons and dendrites."
			"Developed by Xiaoxiao Liu, 2015-8-5"));
	}
}

bool NeuronPopulator::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("populate"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("detect"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
        cout <<"This plugin is used for 1) populate neurons with specified density ( number of neurons, and bounding box size). 2) detect axon and dendrite contacts.\n";
        cout<<"usage:\n";
        cout<<"v3d -x neuron_populator -f populate -i <inswc_file> -o <out_ano_file> -p max_z_rotation num_of_neurons size_x size_y size_z "<<endl;
        cout<<"inswc_file:\t\t input swc file\n";
        cout<<"out_ano_file:\t\t output ano file that lists simulated swc files\n";
        cout<<"num_of_neurons:\t\t output ano file that lists simulated swc files\n";
        cout<<"size_x:\t\t bounding box size in x\n";
        cout<<"size_y:\t\t bounding box size in y\n";
        cout<<"size_z:\t\t bounding box size in z\n";
        cout <<"\n\n\n";

        cout<<"v3d -x neuron_populator -f detect -i <in_swc_ano_file> -o <list_of_landmarks>" <<endl;
        cout<<"in_swc_ano_file:\t\t input ano file that lists simulated swc files\n";
        cout<<"list_of_landmarks:\t\t output axon and dendrite contacting landmarks into a file\n";

	}
	else return false;

	return true;
}

