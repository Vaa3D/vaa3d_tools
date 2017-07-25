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
vector<V3DLONG> leaf_node;
vector<V3DLONG> leaf_node_false;
vector<V3DLONG> temp_length;
vector<V3DLONG> temp_length_up;
//vector<V3DLONG> temp_length_c;
vector<V3DLONG> node_length;
void judgeparent(int parent_judge,const NeuronTree &nt_sorted, vector<V3DLONG> &temp_length_p);
void judgechild(int child_judge,const NeuronTree &nt_sorted, vector<V3DLONG> &temp_length_c);
int getsum(int num);
//void getChildNum(const NeuronTree &nt, vector<vector<V3DLONG> > &childs);
int length_down =0;
int length_up = 0;
int jump_num = 0;
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
    cout<< "******************into ml_func*********************"<<endl;
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
    vector<V3DLONG> selected_trees;





//    pattern_match(sub_trees,pt_consensus,selected_trees,search_step);

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
    cout<< "******************into Pattern Search*********************"<<endl;
    V3DLONG boundary_size = boundary.listNeuron.size();
    V3DLONG nt_size= nt.listNeuron.size();
//    QHash<V3DLONG, V3DLONG> LUT = getUniqueLUT(nt.listNeuron);

//    //create a new id list to give every different neuron a new id
//    QList<V3DLONG> idlist = ((QSet<V3DLONG>)LUT.values().toSet()).toList();

//    //create a child-parent table, both child and parent id refers to the index of idlist
//    QHash<V3DLONG, V3DLONG> cp = ChildParent(nt.listNeuron,idlist,LUT);

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
    cout<< "******************boundry done*********************"<<endl;
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

    cout<< "******************begin to calculate boundry_length*********************"<<endl;
   // calculate boundary_length of each of v_boundary
   vector<NeuronTree> v_area_sorted(v_boundary.size());

   vector<double> length(v_boundary.size(),0);
   int sum_dist = 0;
   int sum_node = 0;

   int sum_length=0;
   cout<<"v_boundary.size()="<<v_boundary.size()<<endl;

   for(V3DLONG i=0;i<v_boundary.size();i++)
   {
        if(v_area[i].listNeuron.size()==0)
        {
            jump_num++;
            continue;
        }
        cout<<"v_area[i].listNeuron.size = "<<v_area[i].listNeuron.size()<<endl;
        if(!SortSWC(v_area[i].listNeuron,v_area_sorted[i].listNeuron,VOID,VOID))
        {
            cout<<"sort failed"<<endl;
            return false;
        }
        v_area_sorted[i].hashNeuron.clear();
        cout<<"v_area_sorted.listNeuron.size() = "<<v_area_sorted[i].listNeuron.size()<<endl;
        for(V3DLONG j=0; j< v_area_sorted[i].listNeuron.size();j++)
        {
            v_area_sorted[i].hashNeuron.insert(v_area_sorted[i].listNeuron[j].n, j);
        }
        cout<< "******************        sort done           *********************"<<endl;
        V3DLONG neuronNum = v_area_sorted[i].listNeuron.size();
        cout<<"neuronnum = "<<neuronNum<<endl;

        childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
        for (V3DLONG j=0;j<neuronNum;j++)
        {
            V3DLONG par = v_area_sorted[i].listNeuron[j].pn;
            if (par<0) continue;
            childs[v_area_sorted[i].hashNeuron.value(par)].push_back(j);
        }

        cout<< "******************        generate childs done           *********************"<<endl;
        int rootidx=0;
      //  int size_rchild = getRemoteChild(rootidx).size();
        temp_length.clear();
        node_length.clear();
        length_up = 0;
        length_down = 0;
        judgechild(rootidx,v_area_sorted[i],temp_length);
        for(int l = 0;l<node_length.size();l++)
        {
            sum_dist = sum_dist + node_length[i];
        }
        sum_dist = sum_dist + getsum(length_up-1);
        sum_node = node_length.size()+length_down-1;
        cout<<"length_up_from_root = "<<length_up<<endl;
        cout<<"sum_node_from_root = "<<sum_node<<endl;
        cout<<"length_up_upper = "<<length_up<<endl;
        cout<<"length_down_upper = "<<length_down<<endl;

        length_up = 0;
        length_down = 0;
        temp_length.clear();
        node_length.clear();
        int size_rchild = leaf_node.size();




        for(int j=0;j<size_rchild;j++)
        {
            cout<<leaf_node[j]<<endl;
        }
        cout<<"size_rchild = "<<size_rchild<<endl;
        for(V3DLONG j=0;j<size_rchild;j++)
        {
            int rchild = leaf_node[j];
//            cout<<"rchild = "<<rchild<<endl;
            int parent = rchild;
            judgeparent(parent,v_area_sorted[i],temp_length);
            for(int l = 0;l<node_length.size();l++)
            {
                sum_dist = sum_dist + node_length[i];
            }
            sum_dist = sum_dist + getsum(length_up-1);
            cout<<"sum_dist = "<<sum_dist<<endl;
            sum_node = sum_node + node_length.size()+length_up-1;
            cout<<"length_up = "<<length_up<<endl;
            cout<<"sum_node = "<<sum_node<<endl;
            cout<<"node_length.size = "<<node_length.size()<<endl;
        }
        leaf_node.clear();
        leaf_node_false.clear();
//        length[i] = sum_dist/sum_node;
        int hahaha = v_area_sorted[i].listNeuron.size()*(size_rchild + 1);
        cout<<"hahaha=="<<hahaha<<endl;
        length[i] = sum_dist/hahaha;
        cout<<"length[i] = "<<length[i]<<endl;
        hahaha = 0;
        sum_dist = 0;
        sum_node = 0;
   }
   cout<< "******************        3           *********************"<<endl;
   for(V3DLONG i=0;i<v_boundary.size();i++)
   {
       sum_length=sum_length + length[i];
   }
   cout<<"sum_length = "<<sum_length<<endl;
   double boundary_length_inner = sum_length/(v_boundary.size()-jump_num);
   cout<<"boundary_length_inner = "<<boundary_length_inner<<endl;
   boundary_length = (int)boundary_length_inner;
   cout<<"boundary_length = "<<boundary_length<<endl;
   return true;

}

QVector<V3DLONG> getRemoteChild(int t)
{
    cout<<"************************this is getremotechild***********************"<<endl;
    QVector<V3DLONG> rchildlist;
    rchildlist.clear();
    int tmp;
    cout<<"childs_size = "<<childs[t].size()<<endl;
    for (int i=0;i<childs[t].size();i++)
    {
        tmp = childs[t].at(i);
        while (childs[tmp].size()==1)
            tmp = childs[tmp].at(0);
        rchildlist.append(tmp);
    }
    return rchildlist;
}

void judgeparent(int parent_judge,const NeuronTree &nt_sorted, vector<V3DLONG> &temp_length_p)
{
//    cout<< "******************this is judgeparent*********************"<<endl;
    length_up++;
    int temp_parent = parent_judge;
    parent_judge = getParent(parent_judge,nt_sorted);
    if(parent_judge!=VOID)
    {
        if(childs[parent_judge].size()>1)
        {
            temp_length_p.push_back(length_up);
  //          temp_length_up.push_back(length_up);
            for(V3DLONG i = 0;i<childs[parent_judge].size();i++)
            {
                if(childs[parent_judge][i]!= temp_parent)
                {

                    length_down++;
                    judgechild(childs[parent_judge][i],nt_sorted,temp_length_p);
                }

            }
            length_down = 0;
            judgeparent(parent_judge,nt_sorted,temp_length_p);
        }
        else if(childs[parent_judge].size()==1)
            judgeparent(parent_judge,nt_sorted,temp_length_p);
    }
    else
      return;

}
void judgechild(int child_judge,const NeuronTree &nt_sorted, vector<V3DLONG> &temp_length_c)
{
   //     cout<< "******************this is judgechild*********************"<<endl;
    length_down++;
    int node_num = 0;
    if(temp_length_c.size()!=0)
    {
        int k = temp_length_c.size()-1;
        node_num = temp_length_c[k]+length_down-1;
        node_length.push_back(node_num);
    }
    if(childs[child_judge].size() > 1)
    {
        temp_length_c.push_back(node_num);
        for(V3DLONG i = 0;i<childs[child_judge].size();i++)
        {
            cout<<"*************meet branches**************"<<endl;
            length_down++;
            judgechild(childs[child_judge][i],nt_sorted,temp_length_c);
            leaf_node.push_back(leaf_node_false[0]);
            leaf_node_false.clear();
        }
    }
    else if(childs[child_judge].size()==1)
    {
        judgechild(childs[child_judge][0],nt_sorted,temp_length_c);
    }
    else
        leaf_node_false.push_back(child_judge);
        return;
}
int getsum(int num)
{
    int sum_num=0;
    for(int i=0;i<num;i++)
    {
        sum_num+=i;
    }
    cout<<"sum_num = "<<sum_num<<endl;
    return sum_num;
}





















//void getChildNum(const NeuronTree &nt, vector<vector<V3DLONG> > &childs)
//{
//    V3DLONG nt_size=nt.listNeuron.size();
//    childs=vector<vector<V3DLONG> > (nt_size,vector<V3DLONG>() );
//    for (V3DLONG i=0; i<nt_size;i++)
//    {
//        V3DLONG par = nt.listNeuron[i].pn;
//        if (par<0) continue;
//        childs[nt.hashNeuron.value(par)].push_back(i);
//    }
//}
//QHash<V3DLONG, V3DLONG> ChildParent(QList<NeuronSWC> &neurons, const QList<V3DLONG> & idlist, const QHash<V3DLONG,V3DLONG> & LUT)
//{
//    QHash<V3DLONG, V3DLONG> cp;
//    for (V3DLONG i=0;i<neurons.size(); i++)
//    {
//        if (neurons.at(i).pn==-1)
//            cp.insertMulti(idlist.indexOf(LUT.value(neurons.at(i).n)), -1);
//        else if(idlist.indexOf(LUT.value(neurons.at(i).pn)) == 0 && neurons.at(i).pn != neurons.at(0).n)
//            cp.insertMulti(idlist.indexOf(LUT.value(neurons.at(i).n)), -1);
//        else
//            cp.insertMulti(idlist.indexOf(LUT.value(neurons.at(i).n)), idlist.indexOf(LUT.value(neurons.at(i).pn)));
//    }
//        return cp;
//};



//QHash<V3DLONG, V3DLONG> getUniqueLUT(QList<NeuronSWC> &neurons)
//{
//    QHash<V3DLONG,V3DLONG> LUT;

//    for (V3DLONG i=0;i<neurons.size();i++)
//    {
//        V3DLONG j;
//        for (j=0;j<i;j++)
//        {
//            if (neurons.at(i).x==neurons.at(j).x && neurons.at(i).y==neurons.at(j).y && neurons.at(i).z==neurons.at(j).z)	break;
//        }
//        LUT.insertMulti(neurons.at(i).n,j);
//    }
//    return (LUT);
//};

bool get_subtrees(const NeuronTree &nt, vector<NeuronTree> &sub_trees, int boundary_length, vector<vector<V3DLONG> >p_to_tree)
{

}










