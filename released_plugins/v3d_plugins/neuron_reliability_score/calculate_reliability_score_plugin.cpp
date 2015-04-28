/* calculate_reliability_score_plugin.cpp
 * This plugin will generate the reliability score of neuron reconstruction.
 * 2015-4-27 : by Hanbo Chen, Hang Xiao, Hanchuan Peng
 */
 
#include "v3d_message.h"
#include <vector>
#include "calculate_reliability_score_plugin.h"
#include "src/topology_analysis.h"
#include <fstream>
using namespace std;
Q_EXPORT_PLUGIN2(calculate_reliability_score, neuronScore);
 
QStringList neuronScore::menulist() const
{
	return QStringList() 
		<<tr("calculate_score")
		<<tr("about");
}

QStringList neuronScore::funclist() const
{
	return QStringList()
		<<tr("calculate_score")
		<<tr("help");
}

void neuronScore::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("calculate_score"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This plugin will generate the reliability score of neuron reconstruction.. "
			"Developed by Hanbo Chen, Hang Xiao, Hanchuan Peng, 2015-4-27"));
	}
}

bool neuronScore::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("calculate_score"))
	{
        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();
        vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();

        if(infiles.size()<2)
        {
            fprintf (stderr, "Need input image and swc. \n");
            printHelp();
            return false;
        }

        if(outfiles.empty())
        {
            fprintf (stderr, "Need output file name. \n");
            printHelp();
            return false;
        }

        int k=0;
        int scoreType = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        float radiusFactor = (paras.size() >= k+1) ? atof(paras[k]) : 2;  k++;

        doCalculateScore(callback, infiles[0],infiles[1],outfiles[0],scoreType,radiusFactor,0);
	}
	else if (func_name == tr("help"))
	{
        printHelp();
	}
	else return false;

	return true;
}


void doCalculateScore(V3DPluginCallback2 &callback, QString fname_img, QString fname_swc, QString fname_output, int score_type=1, float radius_factor=2, bool is_gui=0)
{
    //load image
    unsigned char * p_img1d;
    int type_img;
    V3DLONG sz_img[4];
    if(!simple_loadimage_wrapper(callback, fname_img.toStdString().c_str(), p_img1d, sz_img, type_img)){
        if(is_gui){
            v3d_msg("ERROR: failed to load image file "+fname_img);
        }else{
            qDebug()<<"ERROR: failed to load image file "<<fname_img;
        }
        exit(1);
    }
    if(sz_img[3]>1){
        if(is_gui){
            v3d_msg("WARNING: image has more than 1 color channel. Only the first channel will be used.");
        }else{
            qDebug()<<"WARNING: image has more than 1 color channel. Only the first channel will be used.";
        }
    }

    //load swc
    vector<MyMarker *> neuronTree = readSWC_file(fname_swc.toStdString());
    if(neuronTree.size()==0){
        if(is_gui){
            v3d_msg("ERROR: failed to load swc file "+fname_swc);
        }else{
            qDebug()<<"ERROR: failed to load swc file "<<fname_swc;
        }
        exit(1);
    }

    //calcluate
    map<MyMarker*, double> score_map;
    if(type_img==1)
        topology_analysis_perturb_intense(p_img1d, neuronTree, score_map, radius_factor, sz_img[0], sz_img[1], sz_img[2], score_type);
    else if(type_img==2)
        topology_analysis_perturb_intense((unsigned short *)p_img1d, neuronTree, score_map, radius_factor, sz_img[0], sz_img[1], sz_img[2], score_type);
    else if(type_img==4)
        topology_analysis_perturb_intense((float *)p_img1d, neuronTree, score_map, radius_factor, sz_img[0], sz_img[1], sz_img[2], score_type);
    else{
        if(is_gui){
            v3d_msg("ERROR: image formate not supported. Only support: UINT8, UINT16, and FLOAT32.");
        }else{
            qDebug()<<"ERROR: image formate not supported. Only support: UINT8, UINT16, and FLOAT32";
        }
        exit(1);
    }

    //output
    QString fname_outscore = fname_output + "_score.txt";
    ofstream fp(fname_outscore.toStdString().c_str());
    if(!fp.is_open()){
        if(is_gui){
            v3d_msg("ERROR: cannot open file to save: "+fname_outscore);
        }else{
            qDebug()<<"ERROR: cannot open file to save: "<<fname_outscore;
        }
        exit(1);
    }
    for(long i=0; i<neuronTree.size(); i++){
        fp<<i+1<<"\t"<<score_map[neuronTree[i]]<<endl;
    }
    fp.close();
    for(V3DLONG i = 0; i<neuronTree.size(); i++){
        MyMarker * marker = neuronTree[i];
        double tmp = score_map[marker] * 80 +19;
        marker->type = tmp > 255 ? 255 : tmp;
    }
    QString fname_outswc = fname_output+"_scored.swc";
    saveSWC_file(fname_outswc.toStdString(), neuronTree);

    if(is_gui){
        v3d_msg("Done!");
    }else{
        qDebug()<<"Done!";
    }
}

void printHelp()
{
    qDebug()<<"vaa3d -x libname -f calculate_score -i <input_image> <input_swc> -o <output> -p [<score type> [<radius_factor>]]";
    qDebug()<<"score type (default 1): 0: node wise comparison; 1: segment wise comparison.";
    qDebug()<<"radius factor (RF) (default 2): the area around the reconstruction within distance RF*radius will be masked before searching for alternation pathways.";
}
