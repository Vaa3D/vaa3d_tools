/* pattern_search_plugin.cpp
 * finding substructure in a whole neuron with certain morpological pattern.
 * 2017-6-29 : by He Yishan
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
#include <iostream>
#include "pattern_search_plugin.h"
//#include "pattern_match.h"
#include "sort_swc.h"

#define VOID 1000000000
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
QVector<QVector<V3DLONG> > childs;
QVector<V3DLONG> getRemoteChild(int t);

Q_EXPORT_PLUGIN2(pattern_search, pattern_search);

using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
    NeuronTree nt_search;
    NeuronTree nt_pattern;
};

//struct Boundary
//{
//    double minx;
//    double miny;
//    double minz;
//    double maxx;
//    double maxy;
//    double maxz;
//};

struct Boundary
{
    float minx;
    float miny;
    float minz;
    float maxx;
    float maxy;
    float maxz;
};



void ml_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
bool pattern_analysis(const NeuronTree &nt,const NeuronTree &boundary,NeuronTree & consensus, int & boundary_length);
bool get_subtrees(const NeuronTree &nt, vector<NeuronTree> &sub_trees, int boundary_length, vector<vector<V3DLONG> >p_to_tree);
 
QStringList pattern_search::menulist() const
{
    return QStringList()
		<<tr("tracing_menu")
		<<tr("about");
}

QStringList pattern_search::funclist() const
{
	return QStringList()
		<<tr("tracing_func")
		<<tr("help");
}

void pattern_search::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("tracing_menu"))
	{
        bool bmenu = true;
        input_PARA PARA;
        ml_func(callback,parent,PARA,bmenu);

	}
	else
	{
		v3d_msg(tr("finding substructure in a whole neuron with certain morpological pattern.. "
			"Developed by He Yishan, 2017-6-29"));
	}
}

bool pattern_search::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("tracing_func"))
	{
        bool bmenu = false;
        printf("dofunc can't' work yet.\n");
        return true;
	}
    else if (func_name == tr("help"))
    {
		printf("**** Usage of pattern_search tracing **** \n");
		printf("vaa3d -x pattern_search -f tracing_func -i <inimg_file> -p <channel> <swc_file> <marker_file> <other parameters>\n");
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
    if(bmenu)
    {
        PARA.nt_search = readSWC_file("original_vr_neuron.swc");
        PARA.nt_pattern = readSWC_file("areaofinterest.swc");
    }
    else
    {
        printf("In to dofunc.\n");
    }
    cout<< "******************Welcome To Pattern Search*********************"<<endl;

    int boundary_length;
    NeuronTree pt_consensus;
    if(!pattern_analysis(PARA.nt_search,PARA.nt_pattern,pt_consensus,boundary_length))
    {
        cout<<"something wrong is in pattern_analysis"<<endl;
    }
    vector<NeuronTree> sub_trees;
    vector<vector<V3DLONG> > p_to_tree;
    get_subtrees(PARA.nt_search,sub_trees,boundary_length,p_to_tree);
    double step_radio= 0.25;
    int search_step = boundary_length * step_radio;
    vector<NeuronTree> selected_trees;
    pattern_match(sub_trees,pt_consensus,selected_trees,search_step);

    //Output
    cout<<"selected_tree_size="<<selected_trees.size()<<endl;
    cout<<p_to_tree.size()<<endl;
    vector<V3DLONG> result_points;
    for(int i=0; i<selected_trees.size();i++)
    {
        V3DLONG id_tree=selected_trees[i];
        cout<<"id_tree="<<id_tree<<endl;
        vector<V3DLONG> ps_tree=p_to_tree[id_tree];
        cout<<"ps_tree_size="<<ps_tree.size()<<endl;
        for(int j=0;j<ps_tree.size();j++)
        {
            result_points.push_back(ps_tree[j]);
        }
    }
    cout<<"result_points.size="<<result_points.size()<<endl;
    vector<V3DLONG> result_points_set = result_points; // Could be better

    //QList <NeuronSWC> list_pattern = PARA.nt_boundary.listNeuron;
    QList <NeuronSWC> list_search = PARA.nt_search.listNeuron;

    for(V3DLONG i =0; i<result_points_set.size();i++)
    {
        V3DLONG id=result_points_set[i];
        list_search[id].type =7;    // type=7 means the color is green
    }
    export_list2file(list_search,"updated_vr_neuron.swc","updated_vr_neuron.swc");

    return;
}

bool pattern_analysis(const NeuronTree &nt, const NeuronTree &boundary,NeuronTree & consensus, int & boundary_length)
{
    V3DLONG boundary_size = boundary.listNeuron.size();
    V3DLONG nt_size= nt.listNeuron.size();
    if(boundary_size==0 || nt_size==0)
    {
        v3d_msg("Input is empty, please retry");
        return false;
    }

    // find min max boundary of each area of interest
    Boundary temp;
    vector<Boundary> v_boundary;
    for(V3DLONG i=0;i<boundary_size;i++)
    {

       if(boundary.listNeuron[i].pn<0)
       {
            temp.minx=VOID; temp.miny=VOID;temp.minz=VOID;
            temp.maxx=0;temp.maxy=0;temp.maxz=0;
        }
       temp.minx=min(boundary.listNeuron[i].x,temp.minx);
       temp.miny=min(boundary.listNeuron[i].y,temp.miny);
       temp.minz=min(boundary.listNeuron[i].z,temp.minz);
       temp.maxx=max(boundary.listNeuron[i].x,temp.maxx);
       temp.maxy=max(boundary.listNeuron[i].y,temp.maxy);
       temp.maxz=max(boundary.listNeuron[i].z,temp.maxz);

       if(i==boundary_size-2)
       {
           temp.minx=min(boundary.listNeuron[i+1].x,temp.minx);
           temp.miny=min(boundary.listNeuron[i+1].y,temp.miny);
           temp.minz=min(boundary.listNeuron[i+1].z,temp.minz);
           temp.maxx=max(boundary.listNeuron[i+1].x,temp.maxx);
           temp.maxy=max(boundary.listNeuron[i+1].y,temp.maxy);
           temp.maxz=max(boundary.listNeuron[i+1].z,temp.maxz);
           v_boundary.push_back(temp);
           i++;
       }
       else if(boundary.listNeuron[i+1].pn==-1)
       {
           v_boundary.push_back(temp);
       }
    }

    // push points in v_boundary into v_area;
    vector<NeuronTree> v_area(v_boundary.size());

   for(V3DLONG i=0;i<nt.listNeuron.size();i++)
   {
       NeuronSWC curr = nt.listNeuron[i];
       for(int j=0; j<v_boundary.size();j++)
       {           Boundary b=v_boundary[j];
           if(curr.x>b.minx && curr.y>b.miny && curr.z>b.minz &&curr.x<b.maxx && curr.y<b.maxy&&curr.z<b.maxz)
           {
               v_area[j].listNeuron.push_back(curr);
               break;
           }
       }
    }

   // calculate boundary_length of each of v_boundary
//   QList<NeuronSWC> result;
   NeuronTree v_area_sorted;

   vector<double> length(v_boundary.size(),0);
   int sum_dist=0;
   int sum_length=0;
   //int i=0;
   for(V3DLONG i=0;i<v_boundary.size();i++)
   {     
        if(!SortSWC(v_area[i].listNeuron,v_area_sorted.listNeuron,VOID,VOID))
        {
            cout<<"sort failed"<<endl;
            return false;
        }

        V3DLONG neuronNum = v_area_sorted.listNeuron.size();
        childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
        for (V3DLONG i=0;i<neuronNum;i++)
        {
            V3DLONG par = v_area_sorted.listNeuron[i].pn;
            if (par<0) continue;
            childs[v_area_sorted.hashNeuron.value(par)].push_back(i);
        }


        int rootidx=0;
        int size_rchild = getRemoteChild(rootidx).size();
        for(V3DLONG i=0;i<size_rchild;i++)
        {
            int rchild_1 = getRemoteChild(rootidx).at(i);
            int parent = getParent(rchild_1,nt);
            if(parent!=VOID)
            {
                i++;
                sum_dist=sum_dist+i;
            }
            else
                break;
        }
        length[i] = sum_dist/i;
   }
   for(V3DLONG i=0;i<v_boundary.size();i++)
   {
       sum_length=sum_length + length[i];
   }
   boundary_length = sum_length/v_boundary.size();


}

QVector<V3DLONG> getRemoteChild(int t)
{
    QVector<V3DLONG> rchildlist;
    rchildlist.clear();
    int tmp;
    for (int i=0;i<childs[t].size();i++)
    {
        tmp = childs[t].at(i);
        while (childs[tmp].size()==1)
            tmp = childs[tmp].at(0);
        rchildlist.append(tmp);
    }
    return rchildlist;
}

bool get_subtrees(const NeuronTree &nt, vector<NeuronTree> &sub_trees, int boundary_length, vector<vector<V3DLONG> >p_to_tree)
{

}










