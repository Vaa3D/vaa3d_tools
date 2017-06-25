/* neurontree_structure_matching_plugin.cpp
 * neurontree structure matching
 * 2017-6-23 : by Xiaodong Yue
 */
 
//#define FNUM 22

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
#include "tree_retrieve.h"
//#include "global_neuron_feature.h"

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
	    
		//
		QString fileOpenName_target;
		fileOpenName_target = QFileDialog::getOpenFileName(0, QObject::tr("Open target neuron tree"),
			"",
			QObject::tr("Supported file (*.swc *.eswc)"
			";;Neuron structure	(*.swc)"
			";;Extended neuron structure (*.eswc)"
			));
		if(fileOpenName_target.isEmpty())
			return;

		//build up target neuron tree nt1 from swc file
		NeuronTree nt_target;
		if (fileOpenName_target.toUpper().endsWith(".SWC") || fileOpenName_target.toUpper().endsWith(".ESWC"))
		{
			nt_target = readSWC_file(fileOpenName_target);
		}

		//
		QString fileOpenName_search;
		fileOpenName_search = QFileDialog::getOpenFileName(0, QObject::tr("Open searching neuron tree"),
			"",
			QObject::tr("Supported file (*.swc *.eswc)"
			";;Neuron structure	(*.swc)"
			";;Extended neuron structure (*.eswc)"
			));
		if(fileOpenName_search.isEmpty())
			return;
		//
		
		//build up searching neuron tree nt2 from swc file
		NeuronTree nt_search;
		if (fileOpenName_search.toUpper().endsWith(".SWC") || fileOpenName_search.toUpper().endsWith(".ESWC"))
		{
			nt_search = readSWC_file(fileOpenName_search);
		}
		QString filepath = tr("F:\\mouse brain neuron data\\subtrees\\");
		int window_size = nt_target.listNeuron.size();
		int step = window_size/2;
		QList<NeuronTree> nt_list;
		GetSubTreesSWC(nt_search, window_size, step, filepath, nt_list);
		//find similar trees
		vector<V3DLONG>	retrieved_id;
		tree_retrieval( nt_target, nt_list, retrieved_id);

		//update neural file, for co-debug
		for(int k=0; k< retrieved_id.size();k++)
		{
			printf("%s %d \n","retrieved_id:",retrieved_id[k]);
			//
			int index_nt= retrieved_id[k];
			QString filename = tr("F:\\mouse brain neuron data\\subtrees\\subnttree_");
			QString index_subtree = QString::number(index_nt);
			filename = filename + index_subtree;
			filename = filename + tr(".swc");
			NeuronTree nt_index = readSWC_file(filename);
			//get position
			QString beginend = nt_index.comment;
			QStringList splitted = beginend.split(",");
			int begin = splitted[0].toInt();
			int end = splitted[1].toInt();
			//for debug
			printf("%s %d %s %d \n",tr("positin:"), begin, tr(","),end);

			for(int j=begin; j<=end; j++)
			{
				nt_search.listNeuron[j].type = 50; //change the node type in the finded tree
			}
			
		}//end for k
		//save the updated swc file of searching tree
		QString updatefilepath = tr("F:\\mouse brain neuron data\\subtrees\\updated_vr_neuron.swc");
		writeSWC_file(updatefilepath,nt_search);
		//global_retrieve_main(const V3DPluginArgList & input, V3DPluginArgList & output)
		
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

bool NeurontreeStructureMatching::FindSimilarStructures(const NeuronTree & nt_target, const NeuronTree & nt_search)
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
	QString name_nt_base = tr("./subnttree_");
	
	//matching process
	//for debug
	//QString featureName[FNUM]=
		//{"N_node", "Soma_surface", "N_stem", "N_bifs", "N_branch", "N_tips", "Width", "Height", "Depth","Average Diameter","Diameter,Length",
		//"Surface","Volume", "Max_Eux", "Max_Path", "Max_Order", "Contraction", "Fragmentation", "Pd_ratio", "BifA_local", "BifA_remote", "Hausdorff"};
	
	//get feature of target neuron
	//double * feat_target = new double[FNUM];
	//computeFeature(nt_target, feat_target);
	
	//for ( int cur = 0; cur < numneu_search; cur += step )
	int num_subtree=0;
	for ( int cur = 0; cur < numneu_search; cur += step )
	{
		//construct a subtree in the window
		listneu_cur = listneu_search.mid(cur,window_size);
		SortSWC(listneu_cur, listneu_cursort, VOID, VOID);
		
		NeuronTree nt_cur;
		nt_cur.listNeuron = listneu_cursort;
		nt_cur.linemode = 1;
		//
		//double * feat_search = new double[FNUM];
		//computeFeature(nt_search, feat_search);
		num_subtree++;
		QString n = QString::number(num_subtree, 'g',6);
		QString name_nt_cur = name_nt_base + n + tr(".swc"); 
		
		writeSWC_file(name_nt_cur,nt_cur);


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
		/*if (!feat_search) {
			delete [] feat_search; 
			feat_search = NULL;
		}*/

	}//end for
	
	//if (!feat_target) {
	//		delete [] feat_target; 
	//		feat_target = NULL;}
	//
	return true;
}

bool NeurontreeStructureMatching::GetSubTreesSWC(const NeuronTree & nt, int window_size, int step, const QString & filepath, QList<NeuronTree> & nt_list)
{
	//
	QList <NeuronSWC> listneu;	
	listneu = nt.listNeuron;

	int numneu = listneu.size();

	//for debug
	printf("%s : %d\n", "neuron number of target structure:", numneu);
	printf("%s : %d\n", "window size:", window_size);
	printf("%s : %d\n", "step:", step);
	//
	QList <NeuronSWC> listneu_cur;
	QList <NeuronSWC> listneu_cursort;
	//QString name_nt_base = tr("F:\mouse brain neuron data\subtrees\");
	QString name_nt_base = filepath + tr("subnttree_");
	
	//for ( int cur = 0; cur < numneu_search; cur += step )
	int num_subtree=0;
	int cur = 0;
	for ( cur = 0; cur < (numneu - window_size) ; cur += step )
	{
		//construct a subtree in the window
		listneu_cur.clear();
		listneu_cursort.clear();
		listneu_cur = listneu.mid(cur,window_size);
		SortSWC(listneu_cur, listneu_cursort, VOID, VOID);
		
		NeuronTree nt_cur;
		nt_cur.listNeuron = listneu_cursort;
		nt_cur.linemode = 1;
		int pos = cur + window_size-1;
		QString cmtofcur;
		cmtofcur = QString::number(cur);
		cmtofcur = cmtofcur + tr(",");
		cmtofcur = cmtofcur + QString::number(pos); 
		nt_cur.comment = cmtofcur;
		nt_list.append(nt_cur);
		//
		num_subtree++;
		QString n = QString::number(num_subtree, 'g',6);
		QString name_nt_cur = name_nt_base + n + tr(".swc"); 
		//
		writeSWC_file(name_nt_cur,nt_cur);

	}//end for

	//for last subtree
	NeuronTree nt_cur;
	nt_cur.listNeuron = listneu_cursort;
	nt_cur.linemode = 1;
	int pos = numneu - 1;
	QString cmtofcur;
	cmtofcur = QString::number(cur);
	cmtofcur = cmtofcur + tr(",");
	cmtofcur = cmtofcur + QString::number(pos); //still exceeding problem
	nt_cur.comment = cmtofcur;
	nt_list.append(nt_cur);
	//
	num_subtree++;
	QString n = QString::number(num_subtree, 'g',6);
	QString name_nt_cur = name_nt_base + n + tr(".swc"); 
	//
	writeSWC_file(name_nt_cur,nt_cur);

	return true;

}



