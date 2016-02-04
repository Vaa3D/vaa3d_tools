/* neuron_dist_func.cpp
 * The plugin to calculate distance between two neurons. Distance is defined as the average distance among all nearest pairs in two neurons.
 * 2012-05-04 : by Yinan Wan
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "basic_surf_objs.h"
#include "neuron_dist_func.h"
#include "neuron_dist_gui.h"
#include "neuron_weighted_distance.h"
#include <vector>
#include <iostream>

#include <fstream>

using namespace std;

const QString title = QObject::tr("Neuron Distantce");

int neuron_dist_io(V3DPluginCallback2 &callback, QWidget *parent)
{
	SelectNeuronDlg * selectDlg = new SelectNeuronDlg(parent);
	selectDlg->exec();

    NeuronDistSimple tmp_score = weighted_neuron_score_rounding_nearest_neighbor(&(selectDlg->nt1), &(selectDlg->nt2),1);
    QString message = QString("Distance between neuron 1 (with feature) :\n%1\n and neuron 2:\n%2\n").arg(selectDlg->name_nt1).arg(selectDlg->name_nt2);

    message += QString("weighted entire-structure neuron 1 to neuron 2 = %1\n").arg(tmp_score.weighted_dist12_allnodes);
    message += QString("weighted entire-structure neuron 2 to neuron 1 = %1\n").arg(tmp_score.weighted_dist21_allnodes);
    message += QString("average of bidirectional entire-structure = %1\n").arg(tmp_score.weighted_dist_ave_allnodes);
	message += QString("differen-structure-average = %1\n").arg(tmp_score.dist_apartnodes);
	message += QString("percent of different-structure = %1\n").arg(tmp_score.percent_apartnodes);

	v3d_msg(message);
	return 1;
}

bool neuron_dist_io(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to neuron_dist_io"<<endl;
    if(input.size() != 1)
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

    bool bmenu = 0;
	QString name_nt1(inlist->at(0));
	QString name_nt2(inlist->at(1));
    NeuronTree nt1 = readSWC_file(name_nt1); //eswc file
    if ( !name_nt1.endsWith(".eswc"))
    {
         cout <<"Error: the fist input file is not an eswc file!" <<inlist->at(0)<<endl;
         return false;
    }
	NeuronTree nt2 = readSWC_file(name_nt2);
    NeuronDistSimple tmp_score = weighted_neuron_score_rounding_nearest_neighbor(&nt1, &nt2,bmenu);

    cout<<"\nDistance between neuron 1 (with feature) "<<qPrintable(name_nt1)<<" and neuron 2 "<<qPrintable(name_nt2)<<" is: "<<endl;
    cout<<"weighted entire-structure distance from neuron 1 to 2= "<<tmp_score.weighted_dist12_allnodes <<endl;
    cout<<"weighted entire-structure distance from neuron 2 to 1= "<<tmp_score.weighted_dist21_allnodes <<endl;
    cout<<"average bidirectional weighted entire-structure= "<<tmp_score.weighted_dist_ave_allnodes <<endl;
	cout<<"differen-structure-average = "<<tmp_score.dist_apartnodes<<endl;
	cout<<"percent of different-structure = "<<tmp_score.percent_apartnodes<<endl<<endl;
    cout<< "maximum distance = "<<tmp_score.dist_max<<endl<<endl;

    if (output.size() == 1)
    {
        char *outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

        ofstream myfile;
        myfile.open (outimg_file);
        myfile << "input1 = ";
        myfile << name_nt1.toStdString().c_str()  ;
        myfile << "\nintput2 = ";
        myfile << name_nt2.toStdString().c_str();
        myfile << "\nweighted entire-structure-average (from neuron 1 to 2) = ";
        myfile << tmp_score.weighted_dist12_allnodes;
        myfile << "\nnweighted entire-structure-average (from neuron 2 to 1) = ";
        myfile << tmp_score.weighted_dist21_allnodes;
        myfile << "\nanweighted verage of bi-directional entire-structure-averages =   ";
        myfile << tmp_score.weighted_dist_ave_allnodes;
        myfile << "\ndifferen-structure-average =   ";
        myfile << tmp_score.dist_apartnodes;
        myfile << "\npercent of different-structure  =   ";
        myfile << tmp_score.percent_apartnodes;
        myfile << "\nmaximum distance =   ";
        myfile << tmp_score.dist_max;
        myfile << "\n";
        myfile.close();
    }
	return true;
}

//bool neuron_dist_toolbox(const V3DPluginArgList & input, V3DPluginCallback2 & callback)
//{
//	vaa3d_neurontoolbox_paras * paras = (vaa3d_neurontoolbox_paras *)(input.at(0).p);
//	V3dR_MainWindow * win = paras->win;
//	QList<NeuronTree> * nt_list = callback.getHandleNeuronTrees_Any3DViewer(win);
//	NeuronTree nt = paras->nt;
//	if (nt_list->size()<=1)
//	{
//		v3d_msg("You should have at least 2 neurons in the current 3D Viewer");
//		return false;
//	}

//	QString message;
//	int cur_idx = 0;

//	for (V3DLONG i=0;i<nt_list->size();i++)
//	{
//		NeuronTree curr_nt = nt_list->at(i);
//		if (curr_nt.file == nt.file) {cur_idx = i; continue;}
//        NeuronDistSimple tmp_score = neuron_score_rounding_nearest_neighbor(&nt, &curr_nt,1);
//		message += QString("\nneuron #%1:\n%2\n").arg(i+1).arg(curr_nt.file);
//		message += QString("entire-structure-average = %1\n").arg(tmp_score.dist_allnodes);
//		message += QString("differen-structure-average = %1\n").arg(tmp_score.dist_apartnodes);
//		message += QString("percent of different-structure = %1\n").arg(tmp_score.percent_apartnodes);
//	}
//	message = QString("Distance between current neuron #%1 and\n").arg(cur_idx+1) + message;


//	v3d_msg(message);

//	return true;

//}

void printHelp()
{
    cout<<"\n Neuron  Weighted Distance: Compute the weighted distance between two neurons, input neuron 1 should provide feature value to be the weights. Distance is defined as the average distance among all nearest point pairs."<<endl;
    cout<<"Usage: v3d -x neuron_weighted_distance -f neuron_weighted_distance -i <input_filename1> <input_filename2, *.eswc> -o <output_file>"<<endl;
	cout<<"Parameters:"<<endl;
    cout<<"\t-i <input_filename1, .eswc> <input_filename2>: input neuron structure file"<<endl;
	cout<<"Distance result will be printed on the screen\n"<<endl;
}


