/* neuron_dist_func.cpp
 * The plugin to calculate distance between two neurons. Distance is defined as the average distance among all nearest pairs in two neurons.
 * 2012-05-04 : by Yinan Wan
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "basic_surf_objs.h"
#include "neuron_dist_func.h"
#include "neuron_dist_gui.h"
#include "neuron_distance.h""
#include <vector>
#include <iostream>
using namespace std;

const QString title = QObject::tr("Neuron Distantce");

int neuron_dist_io(V3DPluginCallback2 &callback, QWidget *parent)
{
	SelectNeuronDlg * selectDlg = new SelectNeuronDlg(parent);
	selectDlg->exec();

	double dist = neuron_distance(selectDlg->nt1, selectDlg->nt2);

	v3d_msg(QString("Distance of\nneuron 1:\n%1\nneuron 2:\n%2\nis:\n\t%3").arg(selectDlg->name_nt1).arg(selectDlg->name_nt2).arg(dist));

	return 1;
}

bool neuron_dist_io(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to neuron_dist_io"<<endl;
	if(input.size() != 2 || output.size() != 1) return true;
	char * paras = 0;
	if(((vector<char*> *)(input.at(1).p))->empty()){paras = new char[1]; paras[0]='\0';}
	else paras = (*(vector<char*> *)(input.at(1).p)).at(0);
	cout<<"paras : "<<paras<<endl;

	for(int i = 0; i < strlen(paras); i++)
	{
		if(paras[i] == '#') paras[i] = '-';
	}
	cout<<"paras : "<<paras<<endl;

	return true;
}

int help(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3d_msg("Invoke function : help");
	return 1;
}

bool help(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to help"<<endl;
	if(input.size() != 2 || output.size() != 1) return true;
	char * paras = 0;
	if(((vector<char*> *)(input.at(1).p))->empty()){paras = new char[1]; paras[0]='\0';}
	else paras = (*(vector<char*> *)(input.at(1).p)).at(0);
	cout<<"paras : "<<paras<<endl;

	for(int i = 0; i < strlen(paras); i++)
	{
		if(paras[i] == '#') paras[i] = '-';
	}
	cout<<"paras : "<<paras<<endl;

	return true;
}


