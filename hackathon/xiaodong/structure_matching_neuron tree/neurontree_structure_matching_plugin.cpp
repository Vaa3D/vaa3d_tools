/* neurontree_structure_matching_plugin.cpp
 * neurontree structure matching
 * 2017-6-23 : by Xiaodong Yue
 */
 
#define FNUM 22

#include <vector>
#include <v3d_interface.h>
#include "v3d_message.h"
#include "basic_surf_objs.h"
#include "neuron_dist_func.h"
#include "neuron_match_gui.h"
#include "neuron_sim_scores.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"
#include "../swc_to_maskimage/filter_dialog.h"
#include "sort_swc.h"
#include "compute.h"
#include "global_neuron_feature.h"

#include "neurontree_structure_matching_plugin.h"
Q_EXPORT_PLUGIN2(neurontree_structure_matching, NeurontreeStructureMatching);

using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
    NeuronTree nt;
    LandmarkList listLandmarks;
};

void ml_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
 
QStringList NeurontreeStructureMatching::menulist() const
{
	return QStringList() 
		<<tr("neurontree structure matching")
		<<tr("about");
}

QStringList NeurontreeStructureMatching::funclist() const
{
	return QStringList()
		<<tr("neurontree structure matching")
		<<tr("help");
}

void NeurontreeStructureMatching::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("neurontree structure matching"))
	{
        //bool bmenu = true;
        //input_PARA PARA;
        //ml_func(callback,parent,PARA,bmenu);

		// QOBJECT bug
		//SelectNeuronDlg * selectDlg = new SelectNeuronDlg(parent);
	    //selectDlg->exec();

	    //build up target neuron tree nt1 from swc file
		QString fileOpenName;
		fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open target neuron tree"),
			"",
			QObject::tr("Supported file (*.swc *.eswc)"
			";;Neuron structure	(*.swc)"
			";;Extended neuron structure (*.eswc)"
			));
		if(fileOpenName.isEmpty())
			return;

		//
		NeuronTree nt1;
		if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
		{
			nt1 = readSWC_file(fileOpenName);
		}

		//build up searching neuron tree nt2 from swc file
		fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open searching neuron tree"),
			"",
			QObject::tr("Supported file (*.swc *.eswc)"
			";;Neuron structure	(*.swc)"
			";;Extended neuron structure (*.eswc)"
			));
		if(fileOpenName.isEmpty())
			return;
		//
		NeuronTree nt2;
		if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
		{
			nt2 = readSWC_file(fileOpenName);
		}

		FindSimilarStructures( nt1, nt2);


		//NeuronDistSimple tmp_score = neuron_score_rounding_nearest_neighbor(&(selectDlg->nt1), &(selectDlg->nt2),1);
		//NeuronDistSimple tmp_score = neuron_score_rounding_nearest_neighbor(& nt1, & nt2,1);
		////QString message = QString("Distance between neuron 1:\n%1\n and neuron 2:\n%2\n").arg(selectDlg->name_nt1).arg(selectDlg->name_nt2);
		//QString message = QString("entire-structure-average:from neuron 1 to 2 = %1\n").arg(tmp_score.dist_12_allnodes);
		//message += QString("entire-structure-average:from neuron 2 to 1 = %1\n").arg(tmp_score.dist_21_allnodes);
		//message += QString("average of bi-directional entire-structure-averages = %1\n").arg(tmp_score.dist_allnodes);
		//message += QString("differen-structure-average = %1\n").arg(tmp_score.dist_apartnodes);
		//message += QString("percent of different-structure = %1\n").arg(tmp_score.percent_apartnodes);

		//v3d_msg(message);



		/*
		//load swc file
		QString fileOpenName;
		fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
			"",
			QObject::tr("Supported file (*.swc *.eswc)"
			";;Neuron structure	(*.swc)"
			";;Extended neuron structure (*.eswc)"
			));
		if(fileOpenName.isEmpty())
			return;

		//build up a neuron tree
		NeuronTree nt;
		if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
		{
			nt = readSWC_file(fileOpenName);
		}
		*/

		/* for debug
		QList<NeuronSWC> & list = nt.listNeuron;
	
		printf( "%d \n", list.size());
		for(int i=0;i< list.size();i++) 
			printf( "%f", list[i].r);
		*/
		
		/*
		//generate global features of neuron tree
		double * features = new double[FNUM];
		QString featureName[FNUM]=
		{"N_node", "Soma_surface", "N_stem", "N_bifs", "N_branch", "N_tips", "Width", "Height", "Depth","Average Diameter","Diameter,Length",
		"Surface","Volume", "Max_Eux", "Max_Path", "Max_Order", "Contraction", "Fragmentation", "Pd_ratio", "BifA_local", "BifA_remote", "Hausdorff"};

		computeFeature(nt, features);

		
		//for debug
		for(int i=0;i<FNUM;i++) 
			printf( "%s %d  %f \n", "features",i,features[i]);
		
		
		if (!features) {
			delete []features; 
			features = NULL;
		}
		*/

	}
	else
	{
		v3d_msg(tr("neurontree structure matching. "
			"Developed by Xiaodong Yue, 2017-6-23"));
	}
}

bool NeurontreeStructureMatching::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("neurontree structure matching"))
	{
        bool bmenu = false;
        input_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        if(infiles.empty())
        {
            fprintf (stderr, "Need input image. \n");
            return false;
        }
        else
            PARA.inimg_file = infiles[0];
        int k=0;
        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        QString inneuron_file = (paras.size() >= k+1) ? paras[k] : "";k++;
        if(!inneuron_file.isEmpty())
            PARA.nt = readSWC_file(inneuron_file);
        QString inmarker_file = paras.empty() ? "" : paras[k]; if(inmarker_file == "NULL") inmarker_file = ""; k++;
        QList<ImageMarker> file_inmarkers;
        if(!inmarker_file.isEmpty())
            file_inmarkers = readMarker_file(inmarker_file);

        LocationSimple t;
        for(int i = 0; i < file_inmarkers.size(); i++)
        {
            t.x = file_inmarkers[i].x+1;
            t.y = file_inmarkers[i].y+1;
            t.z = file_inmarkers[i].z+1;
            PARA.listLandmarks.push_back(t);
        }

        ml_func(callback,parent,PARA,bmenu);
	}
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


		printf("**** Usage of neurontree_structure_matching tracing **** \n");
		printf("vaa3d -x neurontree_structure_matching -f tracing_func -i <inimg_file> -p <channel> <swc_file> <marker_file> <other parameters>\n");
        printf("inimg_file       The input image\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");
        printf("swc_file         SWC file path.\n");
        printf("marker_file      Marker file path.\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n");
        printf("outmarker_file   Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

	}
	else return false;

	return true;
}

void ml_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
    unsigned char* data1d = 0;
    V3DLONG N,M,P,sc,c;
    V3DLONG in_sz[4];
    if(bmenu)
    {
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return;
        }

        Image4DSimple* p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }


        data1d = p4DImage->getRawData();
        N = p4DImage->getXDim();
        M = p4DImage->getYDim();
        P = p4DImage->getZDim();
        sc = p4DImage->getCDim();

        bool ok1;

        if(sc==1)
        {
            c=1;
            ok1=true;
        }
        else
        {
            c = QInputDialog::getInteger(parent, "Channel",
                                             "Enter channel NO:",
                                             1, 1, sc, 1, &ok1);
        }

        if(!ok1)
            return;

        PARA.listLandmarks = callback.getLandmark(curwin);
        PARA.nt = callback.getSWC(curwin);


        in_sz[0] = N;
        in_sz[1] = M;
        in_sz[2] = P;
        in_sz[3] = sc;


        PARA.inimg_file = p4DImage->getFileName();
    }
    else
    {
        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,PARA.inimg_file.toStdString().c_str(), data1d, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
            return;
        }
        if(PARA.channel < 1 || PARA.channel > in_sz[3])
        {
            fprintf (stderr, "Invalid channel number. \n");
            return;
        }
        N = in_sz[0];
        M = in_sz[1];
        P = in_sz[2];
        sc = in_sz[3];
        c = PARA.channel;
    }

    //main neuron machine learning code

    //// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON MACHINE LEARNING CODE

    //Output
    NeuronTree nt_output;
    QList<ImageMarker> marker_output;

	QString swc_name = PARA.inimg_file + "_neurontree_structure_matching.swc";
	nt_output.name = "neurontree_structure_matching";
	QString marker_name = PARA.inimg_file + "_neurontree_structure_matching.marker";
    writeSWC_file(swc_name.toStdString().c_str(),nt_output);
    writeMarker_file(marker_name,marker_output);


    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    if(nt_output.listNeuron.size()>0) v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);
    if(marker_output.size()>0) v3d_msg(QString("Now you can drag and drop the generated marker fle [%1] into Vaa3D.").arg(marker_name.toStdString().c_str()),bmenu);


    return;
}

bool NeurontreeStructureMatching::FindSimilarStructures(NeuronTree & nt_target, NeuronTree & nt_search)
{
	//
	QList <NeuronSWC> listneu_target; 
	QList <NeuronSWC> listneu_search;
	
	listneu_target = nt_target.listNeuron;
	listneu_search = nt_search.listNeuron;

	int numneu_target = listneu_target.size();
	int numneu_search = listneu_search.size();
	//size of searching window
	int window_size = numneu_target;
	//searching step 
	int step = window_size/2;


	//for debug
	printf("%s : %d\n", "neuron number of target structure:", numneu_target);
	printf("%s : %d\n", "neuron number of searching tree:", numneu_search);
	printf("%s : %d\n", "window size:", window_size);
	printf("%s : %d\n", "step:", step);
	//
	QList <NeuronSWC> listneu_cur;
	QList <NeuronSWC> listneu_cursort;
	
	//matching process
	//for debug
	QString featureName[FNUM]=
		{"N_node", "Soma_surface", "N_stem", "N_bifs", "N_branch", "N_tips", "Width", "Height", "Depth","Average Diameter","Diameter,Length",
		"Surface","Volume", "Max_Eux", "Max_Path", "Max_Order", "Contraction", "Fragmentation", "Pd_ratio", "BifA_local", "BifA_remote", "Hausdorff"};
	
	//get feature of target neuron
	double * feat_target = new double[FNUM];
	computeFeature(nt_target, feat_target);
	
	//for ( int cur = 0; cur < numneu_search; cur += step )
	for ( int cur = 0; cur < numneu_search; cur += step )
	{
		listneu_cur = listneu_search.mid(cur,window_size);
		SortSWC(listneu_cur, listneu_cursort, VOID, VOID);
		//
		NeuronTree nt_cur;
		nt_cur.listNeuron = listneu_cursort;
		nt_cur.linemode = 1;
		//
		double * feat_search = new double[FNUM];
		computeFeature(nt_search, feat_search);




		//spatical feature test
		//NeuronDistSimple tmp_score = neuron_score_rounding_nearest_neighbor(& nt_target, & nt_cur, 1);
		/*
		QString message = QString("entire-structure-average:from neuron 1 to 2 = %1\n").arg(tmp_score.dist_12_allnodes);
		message += QString("entire-structure-average:from neuron 2 to 1 = %1\n").arg(tmp_score.dist_21_allnodes);
		message += QString("average of bi-directional entire-structure-averages = %1\n").arg(tmp_score.dist_allnodes);
		message += QString("differen-structure-average = %1\n").arg(tmp_score.dist_apartnodes);
		message += QString("percent of different-structure = %1\n").arg(tmp_score.percent_apartnodes);

		v3d_msg(message);
	    */
		if (!feat_search) {
			delete [] feat_search; 
			feat_search = NULL;
		}

	}//end for
	
	if (!feat_target) {
			delete [] feat_target; 
			feat_target = NULL;}
	
	return true;
}



