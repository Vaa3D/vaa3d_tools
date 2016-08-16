/* neuron_dist_func.cpp
 * The plugin to calculate distance between two neurons. Distance is defined as the average distance among all nearest pairs in two neurons.
 * 2012-05-04 : by Yinan Wan
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "basic_surf_objs.h"
#include "neuron_dist_func.h"
#include "neuron_dist_gui.h"
#include "neuron_sim_scores.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"
#include "../swc_to_maskimage/filter_dialog.h"
#include <vector>
#include <iostream>

#include <fstream>

using namespace std;

const QString title = QObject::tr("Neuron Distantce");

int neuron_dist_io(V3DPluginCallback2 &callback, QWidget *parent)
{
	SelectNeuronDlg * selectDlg = new SelectNeuronDlg(parent);
	selectDlg->exec();

    NeuronDistSimple tmp_score = neuron_score_rounding_nearest_neighbor(&(selectDlg->nt1), &(selectDlg->nt2),1);
	QString message = QString("Distance between neuron 1:\n%1\n and neuron 2:\n%2\n").arg(selectDlg->name_nt1).arg(selectDlg->name_nt2);
    message += QString("entire-structure-average:from neuron 1 to 2 = %1\n").arg(tmp_score.dist_12_allnodes);
    message += QString("entire-structure-average:from neuron 2 to 1 = %1\n").arg(tmp_score.dist_21_allnodes);
    message += QString("average of bi-directional entire-structure-averages = %1\n").arg(tmp_score.dist_allnodes);
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
	NeuronTree nt1 = readSWC_file(name_nt1);
	NeuronTree nt2 = readSWC_file(name_nt2);
    NeuronDistSimple tmp_score = neuron_score_rounding_nearest_neighbor(&nt1, &nt2,bmenu);

	cout<<"\nDistance between neuron 1 "<<qPrintable(name_nt1)<<" and neuron 2 "<<qPrintable(name_nt2)<<" is: "<<endl;
    cout<<"entire-structure-average (from neuron 1 to 2) = "<<tmp_score.dist_12_allnodes <<endl;
    cout<<"entire-structure-average (from neuron 2 to 1)= "<<tmp_score.dist_21_allnodes <<endl;
    cout<<"average of bi-directional entire-structure-averages = "<<tmp_score.dist_allnodes <<endl;
    cout<<"differen-structure-average = "<<tmp_score.dist_apartnodes<<endl;
    cout<<"percent of different-structure (from neuron 1 to 2) = "<<tmp_score.percent_12_apartnodes<<endl<<endl;
    cout<<"percent of different-structure (from neuron 2 to 1) = "<<tmp_score.percent_21_apartnodes<<endl<<endl;
    cout<<"percent of different-structure (average) = "<<tmp_score.percent_apartnodes<<endl<<endl;

    if (output.size() == 1)
    {
        char *outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

        ofstream myfile;
        myfile.open (outimg_file);
        myfile << "input1 = ";
        myfile << name_nt1.toStdString().c_str()  ;
        myfile << "\nintput2 = ";
        myfile << name_nt2.toStdString().c_str();
        myfile << "\nentire-structure-average (from neuron 1 to 2) = ";
        myfile << tmp_score.dist_12_allnodes;
        myfile << "\nentire-structure-average (from neuron 2 to 1) = ";
        myfile << tmp_score.dist_21_allnodes;
        myfile << "\naverage of bi-directional entire-structure-averages = ";
        myfile << tmp_score.dist_allnodes;
        myfile << "\ndifferen-structure-average = ";
        myfile << tmp_score.dist_apartnodes;
        myfile << "\npercent of different-structure (from neuron 1 to 2) = ";
        myfile << tmp_score.percent_12_apartnodes;
        myfile << "\npercent of different-structure (from neuron 2 to 1) = ";
        myfile << tmp_score.percent_21_apartnodes;
        myfile << "\npercent of different-structure (average)= ";
        myfile << tmp_score.percent_apartnodes;
        myfile << "\n";
        myfile.close();
    }
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

	QString message;
	int cur_idx = 0;

	for (V3DLONG i=0;i<nt_list->size();i++)
	{
		NeuronTree curr_nt = nt_list->at(i);
		if (curr_nt.file == nt.file) {cur_idx = i; continue;}
        NeuronDistSimple tmp_score = neuron_score_rounding_nearest_neighbor(&nt, &curr_nt,1);
        message += QString("\nneuron #%1:\n%2\n").arg(i+1).arg(curr_nt.file);
        message += QString("entire-structure-average (from neuron 1 to 2)= %1\n").arg(tmp_score.dist_12_allnodes);
        message += QString("entire-structure-average (from neuron 2 to 1)= %1\n").arg(tmp_score.dist_21_allnodes);
        message += QString("average of bi-directional entire-structure-averages = %1\n").arg(tmp_score.dist_allnodes);
		message += QString("differen-structure-average = %1\n").arg(tmp_score.dist_apartnodes);
		message += QString("percent of different-structure = %1\n").arg(tmp_score.percent_apartnodes);
	}
	message = QString("Distance between current neuron #%1 and\n").arg(cur_idx+1) + message;


	v3d_msg(message);

	return true;

}

int neuron_dist_mask(V3DPluginCallback2 &callback, QWidget *parent)
{
    SelectNeuronDlg * selectDlg = new SelectNeuronDlg(parent);
    selectDlg->exec();

    NeuronTree nt1 = selectDlg->nt1;
    NeuronTree nt2 = selectDlg->nt2;

    float dilate_ratio = QInputDialog::getDouble(parent, "dilate_ratio",
                                 "Enter dialate ratio:",
                                 3.0, 1.0, 100.0);
    for(V3DLONG i = 0; i <nt1.listNeuron.size(); i++)
        nt1.listNeuron[i].r = dilate_ratio;
    for(V3DLONG i = 0; i <nt2.listNeuron.size(); i++)
        nt2.listNeuron[i].r = dilate_ratio;

    double x_min,x_max,y_min,y_max,z_min,z_max;
    x_min=x_max=y_min=y_max=z_min=z_max=0;
    V3DLONG sx,sy,sz;
    unsigned char *pImMask_nt1 = 0;
    BoundNeuronCoordinates(nt1,x_min,x_max,y_min,y_max,z_min,z_max);
    sx=x_max;
    sy=y_max;
    sz=z_max;
    V3DLONG stacksz = sx*sy*sz;
    pImMask_nt1 = new unsigned char [stacksz];
    memset(pImMask_nt1,0,stacksz*sizeof(unsigned char));
    ComputemaskImage(nt1, pImMask_nt1, sx, sy, sz);

    double x_min_2,x_max_2,y_min_2,y_max_2,z_min_2,z_max_2;
    x_min_2=x_max_2=y_min_2=y_max_2=z_min_2=z_max_2=0;
    V3DLONG sx_2,sy_2,sz_2;

    unsigned char *pImMask_nt2 = 0;
    BoundNeuronCoordinates(nt2,x_min_2,x_max_2,y_min_2,y_max_2,z_min_2,z_max_2);
    sx_2=x_max_2;
    sy_2=y_max_2;
    sz_2=z_max_2;
    V3DLONG stacksz_2 = sx_2*sy_2*sz_2;
    pImMask_nt2 = new unsigned char [stacksz_2];
    memset(pImMask_nt2,0,stacksz_2*sizeof(unsigned char));
    ComputemaskImage(nt2, pImMask_nt2, sx_2, sy_2, sz_2);

    unsigned int nx=sx, ny=sy, nz=sz;
    if(sx_2 > nx) nx = sx_2;
    if(sy_2 > ny) ny = sy_2;
    if(sz_2 > nz) nz = sz_2;


    unsigned char *pData = new unsigned char[nx*ny*nz];
    memset(pData,0,nx*ny*nz*sizeof(unsigned char));

    for (V3DLONG k1 = 0; k1 < sz; k1++){
       for(V3DLONG j1 = 0; j1 < sy; j1++){
           for(V3DLONG i1 = 0; i1 < sx; i1++){
               if(pImMask_nt1[k1*sx*sy + j1*sx +i1] == 255)
                    pData[k1 * nx*ny + j1*nx + i1] = 127;
           }
       }
    }

    for (V3DLONG k1 = 0; k1 < sz_2; k1++){
       for(V3DLONG j1 = 0; j1 < sy_2; j1++){
           for(V3DLONG i1 = 0; i1 < sx_2; i1++){
               if(pImMask_nt2[k1*sx_2*sy_2 + j1*sx_2 +i1] == 255)
                    pData[k1 * nx*ny + j1*nx + i1] += 127;
           }
       }
    }


    V3DLONG AandB = 0, AorB = 0;
    for(V3DLONG i = 0; i < nx*ny*nz; i++)
    {
        if(pData[i] > 0) AorB++;
        if(pData[i] == 254) AandB++;

    }

    if(pImMask_nt1) {delete []pImMask_nt1; pImMask_nt1 = 0;}
    if(pImMask_nt2) {delete []pImMask_nt2; pImMask_nt2 = 0;}


    v3d_msg(QString("score is %1, %2").arg(AandB).arg(AorB));
    Image4DSimple tmp;
    tmp.setData(pData, nx, ny, nz, 1, V3D_UINT8);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, &tmp);
    callback.setImageName(newwin, QString("Output_swc_mask"));
    callback.updateImageWindow(newwin);
    callback.open3DWindow(newwin);


    return 1;
}

void printHelp()
{
	cout<<"\nNeuron Distance: compute the distance between two neurons. distance is defined as the average distance among all nearest point pairs. 2012-05-04 by Yinan Wan"<<endl;
    cout<<"Usage: v3d -x neuron_distance -f neuron_distance -i <input_filename1> <input_filename2> -o <output_file>"<<endl;
	cout<<"Parameters:"<<endl;
	cout<<"\t-i <input_filename1> <input_filename2>: input neuron structure file (*.swc *.eswc)"<<endl;
	cout<<"Distance result will be printed on the screen\n"<<endl;
}


