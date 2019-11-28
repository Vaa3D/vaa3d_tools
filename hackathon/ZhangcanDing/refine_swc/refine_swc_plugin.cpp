/* refine_swc_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-12-6 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "refine_swc_plugin.h"
#include "refine_swc_funcs.h"

#include "neuron_format_converter.h"
#include "../../zhi/AllenNeuron_postprocessing/sort_swc_IVSCC.h"


using namespace std;
Q_EXPORT_PLUGIN2(refine_swc, refine_swc);

QStringList refine_swc::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList refine_swc::funclist() const
{
	return QStringList()
		<<tr("refine")
        <<tr("initial_4ds")
        <<tr("refine_v2")
        <<tr("smooth_evaluate")
		<<tr("help");
}

void refine_swc::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2018-12-6"));
	}
}

bool refine_swc::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
    vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
    vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
    vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
    vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();
    vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
	if (func_name == tr("refine"))
	{
        if(infiles.size()<2)
        {
            fprintf (stderr, "Need input terafly image path and swc. \n");
            return false;
        }

        if(outfiles.empty())
        {
            fprintf (stderr, "Need output file name. \n");
            return false;
        }

        int k=0;
        int max_length = (paras.size() >= k+1) ? atof(paras[k]) : 200;  k++;

        QList<ImageMarker> break_points;
        NeuronTree nt = readSWC_file(QString(infiles[1]));
        NeuronTree nt2 = SortSWC_pipeline(nt.listNeuron,VOID, 0);
        NeuronTree nt2_broken = breakSWC(nt2,max_length, break_points);
        V_NeuronSWC_list nt2_decomposed = NeuronTree__2__V_NeuronSWC_list(nt2_broken);
        NeuronTree nt2_new = V_NeuronSWC_list__2__NeuronTree(nt2_decomposed);
        NeuronTree nt2_refined = refineSWCTerafly(callback,infiles[0],nt2_new);
        NeuronTree nt2_sorted = SortSWC_pipeline(nt2_refined.listNeuron,VOID, 0);
        writeESWC_file(QString(outfiles[0]),nt2_sorted);

        NeuronTree nt2_smoothed = smoothSWCTerafly(callback,infiles[0],nt2_sorted,break_points);
        NeuronTree nt2_smoothed_sorted = SortSWC_pipeline(nt2_smoothed.listNeuron,VOID, 0);
        writeESWC_file(QString(outfiles[0]),nt2_smoothed_sorted);
	}
    else if(func_name==tr("smooth_evaluate"))
    {
        if(infiles.size()<1)
        {
            fprintf(stderr, "Please input a refined swc/eswc file\n");
            return false;
        }

        NeuronTree nt= readSWC_file(QString(infiles[0]));

        QList<CellAPO> pin_points;
        QList<float> curvature=evalute_smooth(nt,pin_points);

        QString apofilename=QString(outfiles[0]);
        writeAPO_file(apofilename,pin_points);

//        QList<ImageMarker> sample_points;
//        QList<CellAPO> pinpoints;
//        NeuronTree nt_broken= breakSWC(nt,20,sample_points);
//        V_NeuronSWC_list nt_decomposed=NeuronTree__2__V_NeuronSWC_list(nt_broken);
//        NeuronTree nt_new=V_NeuronSWC_list__2__NeuronTree(nt_decomposed);

//        QList<float> curvature= evalute_smooth(nt,pin_poibts,sample_points);






        //QString markerfilename="/home/penglab/PBserver/tmp/ding/smoothness_eval/break_10.marker";
        //writeMarker_file(markerfilename,sample_points);
        //QString swcfilename="/home/penglab/PBserver/tmp/ding/smoothness_eval/break_10.eswc";
        //writeESWC_file(swcfilename,nt_broken);






    }

    else if( func_name==tr("refine_v2")) //added by DZC 14Mar2019
    {
        if(infiles.size()<2)
        {
            fprintf (stderr, "Need input terafly image path and swc. \n");
            return false;
        }

        if(outfiles.empty())
        {
            fprintf (stderr, "Need output file name. \n");
            return false;
        }

        int k=0;
        int max_length = (paras.size() >= k+1) ? atof(paras[k]) : 200;  k++;

        QList<ImageMarker> break_points;
        NeuronTree nt = readSWC_file(QString(infiles[1]));
        NeuronTree nt2 = SortSWC_pipeline(nt.listNeuron,VOID, 0);
        NeuronTree nt2_broken = breakSWC(nt2,max_length, break_points);

        //output the first break points to apo and marker file.
        bool save_inter_result=false;
        if(save_inter_result)
        {
            QString markerfile;
            markerfile=QString(outfiles[0])+"_1st_break_points.marker";
            QList<CellAPO> break_points_apo;
            for(int i=0; i< break_points.size();i++)
            {
                CellAPO break_node;
                break_node.x=break_points.at(i).x;
                break_node.y=break_points.at(i).y;
                break_node.z=break_points.at(i).z;
                break_node.volsize=50;
                break_node.color.r=0;
                break_node.color.g=153;//green
                break_node.color.b=68;
                break_points_apo.push_back(break_node);
            }
            QString apofile;
            apofile=QString(outfiles[0])+"_1st_break_points.apo";
            writeAPO_file(apofile,break_points_apo);
            writeMarker_file(markerfile, break_points);
        }

        V_NeuronSWC_list nt2_decomposed = NeuronTree__2__V_NeuronSWC_list(nt2_broken);
        NeuronTree nt2_new = V_NeuronSWC_list__2__NeuronTree(nt2_decomposed);
        NeuronTree nt2_refined = refineSWCTerafly(callback,infiles[0],nt2_new);
        NeuronTree nt2_sorted = SortSWC_pipeline(nt2_refined.listNeuron,VOID, 0);
        if(save_inter_result)
        {
            QString prerefinename= QString(outfiles[0])+"_1st_refine.eswc";
            writeESWC_file(prerefinename,nt2_sorted);
        }


        //run the 2nd refine according to the new break points and save to apo and marker file

        //QList<ImageMarker> break_points=readMarker_file(QString(infiles[2]));
        //NeuronTree nt2_sorted= readSWC_file(QString(infiles[1]));
        QList<ImageMarker> new_break_points=break_points_reselect(break_points,nt2_sorted);
        if(save_inter_result)
        {
            QString new_markerfile;
            new_markerfile=QString(outfiles[0])+"_2nd_break_points.marker";
            QList<CellAPO> new_break_points_apo;
            for(int i=0; i< new_break_points.size();i++)
            {
                CellAPO break_node;
                break_node.x=new_break_points.at(i).x;
                break_node.y=new_break_points.at(i).y;
                break_node.z=new_break_points.at(i).z;
                break_node.volsize=50;
                break_node.color.r=246;//orange
                break_node.color.g=171;
                break_node.color.b=0;
                new_break_points_apo.push_back(break_node);
            }
            QString apofile_new;
            apofile_new=QString(outfiles[0])+"_2nd_break_points.apo";
            writeAPO_file(apofile_new,new_break_points_apo);
            writeMarker_file(new_markerfile, new_break_points);
        }


        //break the swc with new break points and run the refine pipeline
        NeuronTree nt3_broken=breakSWC_with_points(nt2_sorted,new_break_points);
        V_NeuronSWC_list nt3_decomposed = NeuronTree__2__V_NeuronSWC_list(nt3_broken);
        NeuronTree nt3_new = V_NeuronSWC_list__2__NeuronTree(nt3_decomposed);
        NeuronTree nt3_refined = refineSWCTerafly(callback,infiles[0],nt3_new);
        NeuronTree nt3_sorted = SortSWC_pipeline(nt3_refined.listNeuron,VOID, 100);
        writeESWC_file(QString(outfiles[0]),nt3_sorted);

   }
    else if (func_name == tr("initial_4ds"))
	{
        if(infiles.size()<2)
        {
            fprintf (stderr, "Need input terafly image path and swc. \n");
            return false;
        }

        if(outfiles.empty())
        {
            fprintf (stderr, "Need output file name. \n");
            return false;
        }

        int k=0;
        int ds_rate = (paras.size() >= k+1) ? atof(paras[k]) : 4;  k++;

        QList<ImageMarker> break_points;
        NeuronTree nt = readSWC_file(QString(infiles[1]));
        NeuronTree nt2 = SortSWC_pipeline(nt.listNeuron,VOID, 0);

        NeuronTree nt2_broken = breakSWC(nt2,2000, break_points);
        V_NeuronSWC_list nt2_decomposed = NeuronTree__2__V_NeuronSWC_list(nt2_broken);
        NeuronTree nt2_new = V_NeuronSWC_list__2__NeuronTree(nt2_decomposed);
        NeuronTree nt2_checked = initialSWCTerafly(callback,infiles[0],nt2_new,ds_rate);
        writeESWC_file(QString(outfiles[0]),nt2_checked);
    }else if (func_name == tr("break_detection"))
    {
        if(infiles.size()<2)
        {
            cerr<<"Need two input swc files"<<endl;
            return false;
        }

        if(outfiles.empty())
        {
            cerr<<"Need output file name"<<endl;
            return false;
        }

        NeuronTree nt_original = readSWC_file(QString(infiles[0]));
        V_NeuronSWC_list nt2_decomposed = NeuronTree__2__V_NeuronSWC_list(nt_original);

        V3DLONG n_segs = nt2_decomposed.nsegs();
        if (n_segs<=1) return true;

        V3DLONG *seg_id_array = 0;
        try{ seg_id_array = new V3DLONG [n_segs];}
        catch (...) {v3d_msg("fail to allocate memory in proj_trace_joinAllNeuronSegs()");return false;}
        for (V3DLONG i=0;i<n_segs;i++) seg_id_array[i]=i;

        V_NeuronSWC m_neuron = join_segs_in_V_NeuronSWC_list(nt2_decomposed, seg_id_array, n_segs);
        nt_original = V_NeuronSWC__2__NeuronTree(m_neuron);

        NeuronTree nt_refined = readSWC_file(QString(infiles[1]));
        QVector<QVector<V3DLONG> > childs_original;
        V3DLONG neuronNum = nt_original.listNeuron.size();
        childs_original = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
        for (V3DLONG i=0;i<neuronNum;i++)
        {
            V3DLONG par = nt_original.listNeuron[i].pn;
            if (par<0) continue;
            childs_original[nt_original.hashNeuron.value(par)].push_back(i);
        }
        QList<ImageMarker> tips_original;
        for (V3DLONG i=0;i<neuronNum;i++)
        {
            ImageMarker p;
            if(childs_original[i].size()==0)
            {
                p.x = nt_original.listNeuron[i].x;
                p.y = nt_original.listNeuron[i].y;
                p.z = nt_original.listNeuron[i].z;
                tips_original.push_back(p);
            }
        }
//        QString tip1 = QString(infiles[0]) +".marker";
//        writeMarker_file(tip1,tips_original);

        QVector<QVector<V3DLONG> > childs_refined;
        neuronNum = nt_refined.listNeuron.size();
        childs_refined = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
        for (V3DLONG i=0;i<neuronNum;i++)
        {
            V3DLONG par = nt_refined.listNeuron[i].pn;
            if (par<0) continue;
            childs_refined[nt_refined.hashNeuron.value(par)].push_back(i);
        }

        QList<ImageMarker> tips_refined;
        for (V3DLONG i=0;i<neuronNum;i++)
        {
            ImageMarker p;
            if(childs_refined[i].size()==0)
            {
                p.x = nt_refined.listNeuron[i].x;
                p.y = nt_refined.listNeuron[i].y;
                p.z = nt_refined.listNeuron[i].z;
                bool flag = true;
                for(V3DLONG j=0; j<tips_original.size();j++)
                {
                    if(NTDIS(p,tips_original.at(j))<5)
                    {
                        flag = false;
                        break;
                    }
                }
                if(flag)
                {
                    nt_refined.listNeuron[i].level=255;
                    int parent_tip = getParent(i,nt_refined);
                    while(childs_refined[parent_tip].size()<2)
                    {
                        nt_refined.listNeuron[parent_tip].level=255;
                        parent_tip = getParent(parent_tip,nt_refined);
                        if(parent_tip == 1000000000)
                            break;
                    }
//                    tips_refined.push_back(p);
                }
            }
        }
        writeESWC_file(QString(outfiles[0]),nt_refined);
//        QString tip2 = QString(infiles[1]) +".marker";
//        writeMarker_file(tip2,tips_refined);
    }else if (func_name == tr("help"))
	{
        printf("This is a plugin for refinement \n");
        printf("Usage£º vaa3d -x refine_swc -f refine_v2 -i <terafly image path> <corresponding swc> -o <outputswc> ");
	}
	else return false;

	return true;
}
