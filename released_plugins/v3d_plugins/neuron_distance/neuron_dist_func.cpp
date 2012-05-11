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
#include "customary_structs/vaa3d_neurontoolbox_para.h"
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
	if(input.size() != 1 | output.size() != 0) 
	{
		cerr<<"unrecognized parameter"<<endl;
		return true;
	}

	vector<char*> * inlist = (vector<char*>*)(input.at(0).p);
	if (inlist->size()!=2)
	{
		cerr<<"plese specify only 2 input neurons for distance computing"<<endl;
		return false;
	}

	QString name_nt1(inlist->at(0));
	QString name_nt2(inlist->at(1));
	NeuronTree nt1 = readSWC_file(name_nt1);
	NeuronTree nt2 = readSWC_file(name_nt2);
	double dist = neuron_distance(nt1, nt2);

	cout<<"\nDistance between neuron 1 "<<qPrintable(name_nt1)<<" and neuron 2 "<<qPrintable(name_nt2)<<" is: "<<dist<<endl<<endl;

	return true;
}

bool neuron_dist_toolbox(const V3DPluginArgList & input, V3DPluginCallback2 & callback)
{
	vaa3d_neurontoolbox_paras * paras = (vaa3d_neurontoolbox_paras *)(input.at(0).p);
	V3dR_MainWindow * win = paras->win;
	QList<NeuronTree> * nt_list = callback.getHandleNeuronTrees_Any3DViewer(win);
	NeuronTree nt = paras->nt;
	if (nt_list->size()<=1)
	{
		v3d_msg("You should have at least 2 neurons in the current 3D Viewer");
		return false;
	}

	QString messages = QString("Distance of current neuron and\n");

	for (V3DLONG i=0;i<nt_list->size();i++)
	{
		NeuronTree curr_nt = nt_list->at(i);
		if (curr_nt.file == nt.file) continue;
		double dist = neuron_distance(nt, curr_nt);
		messages += QString("\nneuron %1:\n%2\nis: %3\n").arg(i).arg(curr_nt.file).arg(dist);
	}


	v3d_msg(messages);

	return true;

}

void printHelp()
{
	cout<<"\nNeuron Distance: compute the distance between two neurons. distance is defined as the average distance among all nearest point pairs. 2012-05-04 by Yinan Wan"<<endl;
	cout<<"Usage: v3d -x neuron_distance -f neuron_distance -i <input_filename1> <input_filename2>"<<endl;
	cout<<"Parameters:"<<endl;
	cout<<"\t-i <input_filename1> <input_filename2>: input neuron structure file (*.swc *.eswc)"<<endl;
	cout<<"Distance result will be printed on the screen\n"<<endl;
}


