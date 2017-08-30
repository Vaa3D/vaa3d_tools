/* pattern_search_v2_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-8-23 : by Yishan He
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
#include "pattern_analysis.h"
#include "get_subtrees.h"
#include "../pattern_search/trees_retrieve.h"
#include "pattern_search_v2_plugin.h"
#include <iostream>
Q_EXPORT_PLUGIN2(pattern_search_v2, pattern_search_v2);
#ifndef dist(a,b)
#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#endif
using namespace std;


struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
    NeuronTree nt_search;
    NeuronTree nt_pattern;
};

void ml_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
 
QStringList pattern_search_v2::menulist() const
{
	return QStringList() 
		<<tr("tracing_menu")
		<<tr("about");
}

QStringList pattern_search_v2::funclist() const
{
	return QStringList()
		<<tr("tracing_func")
		<<tr("help");
}

void pattern_search_v2::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("tracing_menu"))
	{
        bool bmenu = true;
        input_PARA PARA;
        ml_func(callback,parent,PARA,bmenu);

	}
	else
	{
        v3d_msg(tr("This is pattern_search version 2 "
			"Developed by Yishan He, 2017-8-23"));
	}
}

bool pattern_search_v2::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("tracing_func"))
	{
        bool bmenu = false;
        input_PARA PARA;
        vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
        vector<char*>* outlist = NULL;
        vector<char*>* paralist = NULL;
        QString searchName = QString(inlist->at(0));
        QString patternName = QString(inlist->at(1));
        PARA.nt_search = readSWC_file(searchName);
        PARA.nt_pattern = readSWC_file(patternName);

        ml_func(callback,parent,PARA,bmenu);
	}
    else if (func_name == tr("help"))
    {
        printf("**** Usage of pattern_search tracing **** \n");
        printf("vaa3d -x pattern_search -f tracing_func -i <search_file> <pattern_file>\n");
        printf("search_file       The input search neuron swc\n");
        printf("pattern_file      The input pattern boundary swc\n");
        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n");
        //printf("outmarker_file   Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");
	}
	else return false;

	return true;
}

void ml_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
    if(bmenu)
    {
        PARA.nt_search = readSWC_file("original_vr_neuron.swc");
        PARA.nt_pattern = readSWC_file("areaofinterest.swc");
    }
    else
    {
        printf("In to dofunc.\n");
    }
    cout<< "******************Welcome To Pattern Search_V2*********************"<<endl;

    vector<double> pt_lens;
    vector<int> pt_nums;
    vector<NeuronTree> pt_list;
    vector<V3DLONG> result_points;
    if(!pattern_analysis(PARA.nt_search,PARA.nt_pattern,pt_list,pt_lens,pt_nums,callback))
    {
        cout<<"something wrong is in pattern_analysis"<<endl;
        return;
    }
    if(pt_list.size()!=pt_lens.size()) {cout<<"list size is not equal to lens size."<<endl; return;}
    cout<<"pt_list.size="<<pt_list.size()<<endl;
    for(int v=0;v<pt_list.size();v++)
    {
        NeuronTree pt = pt_list[v];
        writeSWC_file("pt.swc",pt);
        double area_len = pt_lens[v];
        int pt_num = pt_nums[v];
        vector<NeuronTree> sub_trees;
        vector<vector<V3DLONG> > p_to_tree;
        get_subtrees(PARA.nt_search,sub_trees,area_len,pt_num,p_to_tree);
        vector<V3DLONG> selected_trees;
        trees_retrieve(sub_trees,pt,selected_trees);

        cout<<"selected_tree_size="<<selected_trees.size()<<endl;
        cout<<p_to_tree.size()<<endl;

//        for(int i=0; i<selected_trees.size();i++)
//        {
//            V3DLONG id_tree=selected_trees[i];
//            vector<V3DLONG> ps_tree=p_to_tree[id_tree];
//            // save subtree for test
//            NeuronTree tree_temp;
//            for(int j=0;j<ps_tree.size();j++)
//            {
//                V3DLONG p_id  = ps_tree[j];
//                tree_temp.listNeuron.push_back(PARA.nt_search.listNeuron[p_id]);
//                // put selected points into result
//                result_points.push_back(ps_tree[j]);
//            }
//            QString subtreeName = "subtree_"+ QString::number(v+1) +"_" + QString::number(i+1)+".swc";
//            writeSWC_file(subtreeName,tree_temp);
//            tree_temp.listNeuron.clear();
//        }

        // remove small part from p_to_tree
        for(V3DLONG i =0; i<selected_trees.size(); i++)
        {
            V3DLONG id_tree = selected_trees[i];
            vector<V3DLONG> ps_tree = p_to_tree[id_tree];
            NeuronTree sub_res = sub_trees[id_tree];
            for(V3DLONG j=0; j<sub_res.listNeuron.size(); j++)
            {
                NeuronSWC cur_sub = sub_res.listNeuron[j];
                for(V3DLONG k=0; k<ps_tree.size(); k++)
                {
                    NeuronSWC cur_nt = PARA.nt_search.listNeuron[ps_tree[k]];
                    if(dist(cur_sub,cur_nt) < 0.1)
                    {
                        result_points.push_back(ps_tree[k]);
                        break;
                    }
                }
            }
            QString subtreeName = "subtree_"+ QString::number(v+1) +"_" + QString::number(i+1)+".swc";
            writeSWC_file(subtreeName,sub_trees[id_tree]);
        }
        sub_trees.clear();
        p_to_tree.clear();
        selected_trees.clear();
    }
    //Output
    cout<<"result_points.size="<<result_points.size()<<endl;
    NeuronTree list_search = PARA.nt_search;

    for(V3DLONG i =0; i<result_points.size();i++)
    {
        V3DLONG id=result_points[i];
        list_search.listNeuron[id].type =7;    // type=7 means the color is green
    }
    writeSWC_file("updated_vr_neuron.swc",list_search);
    return;
}












