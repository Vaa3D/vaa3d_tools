/*
 *  sort_func.cpp
 *  core functions for sort neuron swc 
 *
 *  Created by Wan, Yinan, on 06/20/11.
 *  Changed by  Wan, Yinan, on 06/23/11.
 *  Enable processing of .ano file, add threshold parameter by Yinan Wan, on 01/31/12
 */
#ifndef __SORT_SWC_H_
#define __SORT_SWC_H_

#include <QtGlobal>
#include <math.h>
//#include <unistd.h> //remove the unnecessary include file. //by PHC 20131228
#include "basic_surf_objs.h"
#include <string.h>
#include <vector>
#include <iostream>
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include "../../../released_plugins/v3d_plugins/bigneuron_zz_neurontracing_TReMAP/smooth_curve.h"
#include "../../../released_plugins/v3d_plugins/neuron_radius/hierarchy_prune.h"

using namespace std;

#ifndef VOID
#define VOID 1000000000
#endif

#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
#define NTDOT(a,b) ((a).x*(b).x+(a).y*(b).y+(a).z*(b).z)
#ifndef MAX_DOUBLE
#define MAX_DOUBLE 1.79768e+308        //actual: 1.79769e+308
#endif


template <class T> T pow2(T a)
{
    return a*a;

}

QHash<V3DLONG, V3DLONG> ChildParent(QList<NeuronSWC> &neurons, const QList<V3DLONG> & idlist, const QHash<V3DLONG,V3DLONG> & LUT)
{
    QHash<V3DLONG, V3DLONG> cp;
    for (V3DLONG i=0;i<neurons.size(); i++)
        if (neurons.at(i).pn==-1) cp.insertMulti(idlist.indexOf(LUT.value(neurons.at(i).n)), -1);
        else if(idlist.indexOf(LUT.value(neurons.at(i).pn)) == 0 && neurons.at(i).pn != neurons.at(0).n)
            cp.insertMulti(idlist.indexOf(LUT.value(neurons.at(i).n)), -1);
        else
            cp.insertMulti(idlist.indexOf(LUT.value(neurons.at(i).n)), idlist.indexOf(LUT.value(neurons.at(i).pn)));
    return cp;
};


QHash<V3DLONG, V3DLONG> getUniqueLUT(QList<NeuronSWC> &neurons)
{
	QHash<V3DLONG,V3DLONG> LUT;

	for (V3DLONG i=0;i<neurons.size();i++)
	{
		V3DLONG j;
		for (j=0;j<i;j++)
		{
            if (neurons.at(i).x==neurons.at(j).x && neurons.at(i).y==neurons.at(j).y && neurons.at(i).z==neurons.at(j).z)	break;
		}
		LUT.insertMulti(neurons.at(i).n,j);
	}
	return (LUT);
};


void DFS(bool** matrix, V3DLONG* neworder, V3DLONG node, V3DLONG* id, V3DLONG siz, int* numbered, int *group)
{
    if (!numbered[node]){
        numbered[node] = *group;
        neworder[*id] = node;
        (*id)++;
        for (V3DLONG v=0;v<siz;v++)
            if (!numbered[v] && matrix[v][node])
            {
                DFS(matrix, neworder, v, id, siz,numbered,group);
            }
    }
};

double computeDist2(const NeuronSWC & s1, const NeuronSWC & s2)
{
	double xx = s1.x-s2.x;
	double yy = s1.y-s2.y;
	double zz = s1.z-s2.z;
	return (xx*xx+yy*yy+zz*zz);
};

bool combine_linker(vector<QList<NeuronSWC> > & linker, QList<NeuronSWC> & combined)
{
	V3DLONG neuronNum = linker.size();
	if (neuronNum<=0)
	{
		cout<<"the linker file is empty, please check your data."<<endl;
		return false;
	}
	V3DLONG offset = 0;
	combined = linker[0];
	for (V3DLONG i=1;i<neuronNum;i++)
	{
		V3DLONG maxid = -1;
		for (V3DLONG j=0;j<linker[i-1].size();j++)
			if (linker[i-1][j].n>maxid) maxid = linker[i-1][j].n;
		offset += maxid+1;
		for (V3DLONG j=0;j<linker[i].size();j++)
		{
			NeuronSWC S = linker[i][j];
			S.n = S.n+offset;
			if (S.pn>=0) S.pn = S.pn+offset;
			combined.append(S);
		}
	}
};

NeuronTree SortSWC_pipeline(QList<NeuronSWC> & neurons, V3DLONG newrootid, double thres, bool flag = false)
{
    NeuronTree nt_sorted;

	//create a LUT, from the original id to the position in the listNeuron, different neurons with the same x,y,z & r are merged into one position
	QHash<V3DLONG, V3DLONG> LUT = getUniqueLUT(neurons);

	//create a new id list to give every different neuron a new id		
	QList<V3DLONG> idlist = ((QSet<V3DLONG>)LUT.values().toSet()).toList();

	//create a child-parent table, both child and parent id refers to the index of idlist
    QHash<V3DLONG, V3DLONG> cp = ChildParent(neurons,idlist,LUT);


	V3DLONG siz = idlist.size();

	bool** matrix = new bool*[siz];
	for (V3DLONG i = 0;i<siz;i++)
	{
		matrix[i] = new bool[siz];
		for (V3DLONG j = 0;j<siz;j++) matrix[i][j] = false;
	}


	//generate the adjacent matrix for undirected matrix
	for (V3DLONG i = 0;i<siz;i++)
	{
		QList<V3DLONG> parentSet = cp.values(i); //id of the ith node's parents
        for (V3DLONG j=0;j<parentSet.size();j++)
		{
			V3DLONG v2 = (V3DLONG) (parentSet.at(j));
            if (v2==-1) continue;
			matrix[i][v2] = true;
			matrix[v2][i] = true;
		}
	}


	//do a DFS for the the matrix and re-allocate ids for all the nodes
    V3DLONG root = 0;
	if (newrootid==VOID)
	{
		for (V3DLONG i=0;i<neurons.size();i++)
			if (neurons.at(i).pn==-1){
				root = idlist.indexOf(LUT.value(neurons.at(i).n));
				break;
			}
	}
	else{
		root = idlist.indexOf(LUT.value(newrootid));

		if (LUT.keys().indexOf(newrootid)==-1)
		{
			v3d_msg("The new root id you have chosen does not exist in the SWC file.");
            return nt_sorted;
        }
	}


	V3DLONG* neworder = new V3DLONG[siz];
	int* numbered = new int[siz];
	for (V3DLONG i=0;i<siz;i++) numbered[i] = 0;

	V3DLONG id[] = {0};

	int group[] = {1};
	DFS(matrix,neworder,root,id,siz,numbered,group);

	while (*id<siz)
	{
		V3DLONG iter;
		(*group)++;
		for (iter=0;iter<siz;iter++)
			if (numbered[iter]==0) break;
		DFS(matrix,neworder,iter,id,siz,numbered,group);
	}


	//find the point in non-group 1 that is nearest to group 1, 
	//include the nearest point as well as its neighbors into group 1, until all the nodes are connected
	while((*group)>1)
	{
		double min = VOID;
		double dist2 = 0;
		int mingroup = 1;
		V3DLONG m1,m2;
		for (V3DLONG ii=0;ii<siz;ii++){
			if (numbered[ii]==1)
				for (V3DLONG jj=0;jj<siz;jj++)
					if (numbered[jj]!=1)
					{
						dist2 = computeDist2(neurons.at(idlist.at(ii)),neurons.at(idlist.at(jj)));
                        if (dist2<min)
						{
                            if(!flag)
                            {
                                min = dist2;
                                mingroup = numbered[jj];
                                m1 = ii;
                                m2 = jj;
                            }else if(neurons.at(idlist.at(ii)).type != neurons.at(idlist.at(jj)).type)
                            {
                                min = dist2;
                                mingroup = numbered[jj];
                                m1 = ii;
                                m2 = jj;
                            }
						}
					}
		}
		for (V3DLONG i=0;i<siz;i++)
			if (numbered[i]==mingroup)
				numbered[i] = 1;
        if (min<=thres*thres)
        {

            matrix[m1][m2] = true;
            matrix[m2][m1] = true;
		}
		(*group)--;
	}

    id[0] = 0;
	for (int i=0;i<siz;i++)
	{
		numbered[i] = 0;
		neworder[i]= VOID;
	}
	
	*group = 1;

	V3DLONG new_root=root;
	V3DLONG offset=0;
    V3DLONG last_id = 0;

    //NeutronTree structure

    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();

	while (*id<siz)
	{
		V3DLONG cnt = 0;
		DFS(matrix,neworder,new_root,id,siz,numbered,group);
		(*group)++;
		NeuronSWC S;
		S.n = offset+1;
		S.pn = -1;
		V3DLONG oripos = idlist.at(new_root);
		S.x = neurons.at(oripos).x;
		S.y = neurons.at(oripos).y;
		S.z = neurons.at(oripos).z;
		S.r = neurons.at(oripos).r;
		S.type = neurons.at(oripos).type;
        listNeuron.append(S);
        hashNeuron.insert(S.n, listNeuron.size()-1);
        last_id = S.n;

		cnt++;

		for (V3DLONG ii=offset+1;ii<(*id);ii++)
		{
			for (V3DLONG jj=offset;jj<ii;jj++) //after DFS the id of parent must be less than child's
			{
				if (neworder[ii]!=VOID && neworder[jj]!=VOID && matrix[neworder[ii]][neworder[jj]] ) 
				{
                    if(last_id != ii+1)
                    {
                        NeuronSWC S;
                        S.n = ii+1;
                        S.pn = jj+1;
                        V3DLONG oripos = idlist.at(neworder[ii]);
                        S.x = neurons.at(oripos).x;
                        S.y = neurons.at(oripos).y;
                        S.z = neurons.at(oripos).z;
                        S.r = neurons.at(oripos).r;
                        S.type = neurons.at(oripos).type;
                        listNeuron.append(S);
                        hashNeuron.insert(S.n, listNeuron.size()-1);
                        last_id = ii+1;
                        cnt++;
                    }

                        break; //added by CHB to avoid problem caused by loops in swc, 20150313
				}
			}
		}

        nt_sorted.n = -1;
        nt_sorted.on = true;
        nt_sorted.listNeuron = listNeuron;
        nt_sorted.hashNeuron = hashNeuron;
		for (new_root=0;new_root<siz;new_root++)
			if (numbered[new_root]==0) break;
		offset += cnt;
	}



	if ((*id)<siz) {
		v3d_msg("Error!");
        return nt_sorted ;
	}
	
	//free space by Yinan Wan 12-02-02
	if (neworder) {delete []neworder; neworder=NULL;}
	if (numbered) {delete []numbered; numbered=NULL;}
	if (matrix){
		for (V3DLONG i=0;i<siz;i++) {delete matrix[i]; matrix[i]=NULL;}
		if (matrix) {delete []matrix; matrix=NULL;}
	}


    return nt_sorted;
};

NeuronTree pruneswc(NeuronTree nt, double length)
{
    QVector<QVector<V3DLONG> > childs;

    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    V3DLONG *flag = new V3DLONG[neuronNum];

    for (V3DLONG i=0;i<neuronNum;i++)
    {
        flag[i] = 1;

        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    QList<NeuronSWC> list = nt.listNeuron;

    for (int i=0;i<list.size();i++)
    {
        if(i>0 && getParent(i,nt) == 1000000000)
            break;
        if (childs[i].size()==0)
        {
            int index_tip = 0;
            int parent_tip = getParent(i,nt);
            while(childs[parent_tip].size()<2)
            {
                parent_tip = getParent(parent_tip,nt);
                index_tip++;
                if(parent_tip == 1000000000)
                    break;
            }
            if(index_tip < length)
            {
                flag[i] = -1;

                int parent_tip = getParent(i,nt);
                while(childs[parent_tip].size()<2)
               {
                    flag[parent_tip] = -1;
                    parent_tip = getParent(parent_tip,nt);
                    if(parent_tip == 1000000000)
                        break;
               }
            }

        }

    }

   //NeutronTree structure
   NeuronTree nt_prunned;
   QList <NeuronSWC> listNeuron;
   QHash <int, int>  hashNeuron;
   listNeuron.clear();
   hashNeuron.clear();

   //set node

   NeuronSWC S;
   for (int i=0;i<list.size();i++)
   {
       if(flag[i] == 1)
       {
            NeuronSWC curr = list.at(i);
            S.n 	= curr.n;
            S.type 	= curr.type;
            S.x 	= curr.x;
            S.y 	= curr.y;
            S.z 	= curr.z;
            S.r 	= curr.r;
            S.pn 	= curr.pn;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);
       }

  }
   nt_prunned.n = -1;
   nt_prunned.on = true;
   nt_prunned.listNeuron = listNeuron;
   nt_prunned.hashNeuron = hashNeuron;

   if(flag) {delete[] flag; flag = 0;}
   return nt_prunned;
}

bool smoothswc(vector<MyMarker*> & inswc, double length)
{
    unsigned char* inimg1d = 0;
    vector<HierarchySegment*> topo_segs;
    swc2topo_segs(inswc, topo_segs, 1, inimg1d, 0, 0, 0);

    cout<<"Smooth the final curve"<<endl;
    for(int i = 0; i < topo_segs.size(); i++)
    {
        HierarchySegment * seg = topo_segs[i];
        MyMarker * leaf_marker = seg->leaf_marker;
        MyMarker * root_marker = seg->root_marker;
        vector<MyMarker*> seg_markers;
        MyMarker * p = leaf_marker;
        while(p != root_marker)
        {
            seg_markers.push_back(p);
            p = p->parent;
        }
        seg_markers.push_back(root_marker);
        smooth_curve_Z(seg_markers, length);
       // smooth_curve_XY(seg_markers, length);
    }
    inswc.clear();

    topo_segs2swc(topo_segs, inswc, 0);
    return true;
}

vector<MyMarker*> internodeprune(vector<MyMarker*> & final_out_swc, NeuronTree nt)
{
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }
    vector<MyMarker*> final_out_swc_updated;
    //final_out_swc_updated.push_back(final_out_swc[0]);


    for(int j = 0; j < final_out_swc.size(); j++)
    {
        if(final_out_swc[j]->parent != 0)
        {
            int flag_prun = 0;
            int par_x = final_out_swc[j]->parent->x;
            int par_y = final_out_swc[j]->parent->y;
            int par_z = final_out_swc[j]->parent->z;
            int par_r = final_out_swc[j]->parent->radius;

            int dis_prun = sqrt(pow2(final_out_swc[j]->x - par_x) + pow2(final_out_swc[j]->y - par_y) + pow2(final_out_swc[j]->z - par_z));
            if( (final_out_swc[j]->radius + par_r - dis_prun)/dis_prun > 0.3)
            {
                if(childs[j].size() > 0)
                {
                    for(int jj = 0; jj < childs[j].size(); jj++)
                        final_out_swc[childs[j].at(jj)]->parent = final_out_swc[j]->parent;
                }
                flag_prun = 1;
            }

            if(flag_prun == 0)
            {
                final_out_swc_updated.push_back(final_out_swc[j]);
            }
        }
        else
            final_out_swc_updated.push_back(final_out_swc[j]);

    }


    return final_out_swc_updated;
}

bool analysis_swc(QString fileOpenName ,bool bmenu)
{
    NeuronTree nt;
    if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
    {
         nt = readSWC_file(fileOpenName);
    }

    QVector<QVector<V3DLONG> > childs;


    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );

    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    QList<NeuronSWC> list = nt.listNeuron;
    vector<MyMarker*> swc_file = readSWC_file(fileOpenName.toStdString());

    int branchOrder_apical = 0;
    int branchOrder_basal = 0;

    for (int i=0;i<list.size();i++)
    {
        int parent_node = getParent(i,nt);
        if(parent_node == 1000000000 || 0)
            swc_file[i]->type = 0;
        else if (childs[parent_node].size()<=1)
            swc_file[i]->type =  swc_file[parent_node]->type;
        else if (childs[parent_node].size()>1)
        {
            swc_file[i]->type =  swc_file[parent_node]->type + 1;
        }
    }

    for (int i=0;i<list.size();i++)
    {
        if(list.at(i).type == 3 && swc_file[i]->type > branchOrder_basal)
            branchOrder_basal = swc_file[i]->type;
        else if (list.at(i).type == 4 && swc_file[i]->type > branchOrder_apical)
            branchOrder_apical = swc_file[i]->type;

    }

//    v3d_msg(QString("apical is %1, basal is %2").arg(branchOrder_apical).arg(branchOrder_basal));
    double *branchapical_mean = new double[branchOrder_apical];
    double branchapical_totalmean = 0;
    double branchapical_x = 0;
    QString disp_text = fileOpenName + "\n\n";
    disp_text += "branch orders vs. diameters (apical)\n\n";
    int count_apical = 0;

    QString output_csv_apical_file = fileOpenName + ".csv";
    QFile file(output_csv_apical_file);
    if (!file.open(QFile::WriteOnly|QFile::Truncate))
    {
        cout <<"Error opening the file "<<output_csv_apical_file.toStdString().c_str() << endl;
    }

    QTextStream stream (&file);
    stream << "Branch order (apical) vs. diameters"<<"\n";

    for (int j = 0; j < branchOrder_apical; j++)
    {

        stream << j+1 <<",";
        int count = 0;
        double radius_sum = 0;
        for (int i = 0;i < list.size();i++)
        {
            if(list.at(i).type == 4 &&  swc_file[i]->type == j+1)
            {
                double radius_node = list.at(i).radius;
                radius_sum += radius_node;
                branchapical_totalmean += radius_node*2;
                branchapical_x+= j+1;
                count++;
                count_apical++;
                stream << radius_node      <<",";
            }
        }
        stream << "\n";
        branchapical_mean[j] = 2*radius_sum/count;
        disp_text += "Branch order = " + QString::number (j+1)  + ", average diameter = "    + QString::number(branchapical_mean[j]) + ";\n";
    }

    branchapical_totalmean = branchapical_totalmean/count_apical;
    branchapical_x = branchapical_x/count_apical;
    double Sxy_apical = 0;
    double Sxx_apical = 0;

    stream << "Branch order (apical) vs. average diameters"<<"\n";


    for (int j = 0; j < branchOrder_apical; j++)
    {
        stream << j+1   <<",";
        stream << branchapical_mean[j]  <<",\n";
        for (int i = 0;i < list.size();i++)
        {
            if(list.at(i).type == 4 &&  swc_file[i]->type == j+1)
            {
                Sxy_apical+= ((j+1)-branchapical_x)*(list.at(i).radius*2-branchapical_totalmean);
                Sxx_apical+= ((j+1)-branchapical_x)*((j+1)-branchapical_x);
            }
        }
    }

    double  branchapical_slope = Sxy_apical/Sxx_apical;
    disp_text += "Slope = " + QString::number (branchapical_slope) + ";\n";
    stream << "Slope"  <<"," << branchapical_slope <<"\n\n";


    double *branchbasal_mean = new double[branchOrder_basal];
    double branchbasal_totalmean = 0;
    double branchbasal_x = 0;
    disp_text += "\n\n\nbranch orders vs. diameters (basal)\n\n";
    int count_basal = 0;
    stream << "Branch order (basal) vs. diameters"<<"\n";

    for (int j = 0; j < branchOrder_basal; j++)
    {
        stream << j+1 <<",";
        int count = 0;
        double radius_sum = 0;
        for (int i = 0;i < list.size();i++)
        {
            if(list.at(i).type == 3 &&  swc_file[i]->type == j+1)
            {
                double radius_node = list.at(i).radius;
                radius_sum += radius_node;
                branchbasal_totalmean += radius_node*2;
                branchbasal_x+= j+1;
                count++;
                count_basal++;
                stream << radius_node      <<",";

            }
        }
        stream << "\n";

        branchbasal_mean[j] = 2*radius_sum/count;
        disp_text += "Branch order = " + QString::number (j+1)  + ", average diameter = "    + QString::number(branchbasal_mean[j]) + ";\n";
    }

    branchbasal_totalmean = branchbasal_totalmean/count_basal;
    branchbasal_x = branchbasal_x/count_basal;
    double Sxy_basal = 0;
    double Sxx_basal = 0;

    stream << "Branch order (basal) vs. average diameters"<<"\n";

    for (int j = 0; j < branchOrder_basal; j++)
    {
        stream << j+1   <<",";
        stream << branchbasal_mean[j]  <<",\n";
        for (int i = 0;i < list.size();i++)
        {
            if(list.at(i).type == 3 &&  swc_file[i]->type == j+1)
            {
                Sxy_basal+= ((j+1)-branchbasal_x)*(list.at(i).radius*2-branchbasal_totalmean);
                Sxx_basal+= ((j+1)-branchbasal_x)*((j+1)-branchbasal_x);
            }
        }
    }

    double  branchbasal_slope = Sxy_basal/Sxx_basal;
    disp_text += "Slope = " + QString::number (branchbasal_slope) + ";\n";

    stream << "Slope"  <<"," << branchbasal_slope <<"\n\n";

    file.close();

    v3d_msg(disp_text,bmenu);
    return true;
}

bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName)
{
	QFile file(fileSaveName);
	if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
		return false;
	QTextStream myfile(&file);
	myfile<<"# generated by Vaa3D Plugin sort_neuron_swc"<<endl;
	myfile<<"# source file(s): "<<fileOpenName<<endl;
	myfile<<"# id,type,x,y,z,r,pid"<<endl;
	for (V3DLONG i=0;i<lN.size();i++)
		myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";

	file.close();
	cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<lN.size()<<endl;
	return true;
};

double getswcdiameter(NeuronTree* nt)
{
    if(nt->listNeuron.size()<=0)
        return 0;
    float x_min=nt->listNeuron.at(0).x;
    float x_max=nt->listNeuron.at(0).x;
    float y_min=nt->listNeuron.at(0).y;
    float y_max=nt->listNeuron.at(0).y;
    float z_min=nt->listNeuron.at(0).z;
    float z_max=nt->listNeuron.at(0).z;
    for(V3DLONG i=1; i<nt->listNeuron.size(); i++){
        x_min=MIN(nt->listNeuron.at(i).x, x_min);
        y_min=MIN(nt->listNeuron.at(i).y, y_min);
        z_min=MIN(nt->listNeuron.at(i).z, z_min);
        x_max=MAX(nt->listNeuron.at(i).x, x_max);
        y_max=MAX(nt->listNeuron.at(i).y, y_max);
        z_max=MAX(nt->listNeuron.at(i).z, z_max);
    }
    double tmp=(x_max-x_min)*(x_max-x_min);
    tmp=(y_max-y_min)*(y_max-y_min);
    tmp=(z_max-z_min)*(z_max-z_min);
    tmp=sqrt(tmp);
    return tmp;
}

void connectall(NeuronTree* nt, QList<NeuronSWC>& newNeuron, double xscale, double yscale, double zscale, double angThr, double disThr, int matchType, bool b_minusradius, int rootID=-1)
{
    newNeuron.clear();
    bool b_connectall = false;
    bool b_somaaxon = false;
    if(disThr<0){
        disThr=getswcdiameter(nt);
        b_connectall=true;
    }

    //rescale neurons
    QList<XYZ> scaledXYZ;
    for(V3DLONG i=0; i<nt->listNeuron.size(); i++){
        XYZ S;
        S.x = nt->listNeuron.at(i).x*xscale;
        S.y = nt->listNeuron.at(i).y*yscale;
        S.z = nt->listNeuron.at(i).z*zscale;
        scaledXYZ.append(S);
    }

    qDebug()<<"search for components and tips";
    //initialize tree components and get all tips
    QList<V3DLONG> cand;
    QList<XYZ> canddir;
    QVector<int> childNum(nt->listNeuron.size(), 0);
    QVector<int> connNum(nt->listNeuron.size(), 0);
    QList<V3DLONG> components;
    QList<V3DLONG> pList;
    V3DLONG curid=0;
    for(V3DLONG i=0; i<nt->listNeuron.size(); i++){
        if(nt->listNeuron.at(i).pn<0){
            connNum[i]--; //root that only have 1 child will also be a dead end
            components.append(curid); curid++;
            pList.append(-1);
        }else{
            V3DLONG pid = nt->hashNeuron.value(nt->listNeuron.at(i).pn);
            childNum[pid]++;
            connNum[pid]++;
            components.append(-1);
            pList.append(pid);

            //check if there is connection between soma and axon already
            if(!b_somaaxon){
                if(nt->listNeuron.at(i).type==2 && nt->listNeuron.at(pid).type==1)
                    b_somaaxon = true;
                if(nt->listNeuron.at(i).type==1 && nt->listNeuron.at(pid).type==2)
                    b_somaaxon = true;
            }
        }
    }
    qDebug()<<"components searching";
    //connected component
    for(V3DLONG cid=0; cid<curid; cid++){
        QStack<V3DLONG> pstack;
        V3DLONG chid;
        if(!components.contains(cid)) //should not happen, just in case
            continue;
        if(components.indexOf(cid)!=components.lastIndexOf(cid)) //should not happen
            qDebug("unexpected multiple tree root, please check the code: neuron_stitch_func.cpp");
        //recursively search for child and mark them as the same component
        pstack.push(components.indexOf(cid));
        while(!pstack.isEmpty()){
            V3DLONG pid=pstack.pop();
            chid = -1;
            chid = pList.indexOf(pid,chid+1);
            while(chid>=0){
                pstack.push(chid);
                components[chid]=cid;
                chid=pList.indexOf(pid,chid+1);
            }
        }
    }
    qDebug()<<"tips searching";
    //get tips
    for(V3DLONG i=0; i<childNum.size(); i++){
        if(connNum.at(i)<1){
            cand.append(i);
            //get direction
            V3DLONG id=i;
            XYZ tmpdir(0,0,0);
            if(childNum[id]==1){ //single child root
                V3DLONG sid = pList.indexOf(id);
                if(sid>=0){
                    tmpdir.x += scaledXYZ.at(id).x-scaledXYZ.at(sid).x;
                    tmpdir.y += scaledXYZ.at(id).y-scaledXYZ.at(sid).y;
                    tmpdir.z += scaledXYZ.at(id).z-scaledXYZ.at(sid).z;
                }
            }else{ //tips
                V3DLONG sid = pList[id];
                if(sid>=0){
                    tmpdir.x += scaledXYZ.at(id).x-scaledXYZ.at(sid).x;
                    tmpdir.y += scaledXYZ.at(id).y-scaledXYZ.at(sid).y;
                    tmpdir.z += scaledXYZ.at(id).z-scaledXYZ.at(sid).z;
                }
            }
            double tmpNorm = sqrt(tmpdir.x*tmpdir.x+tmpdir.y*tmpdir.y+tmpdir.z*tmpdir.z);
            if(tmpNorm>1e-16){
                tmpdir.x/=tmpNorm;
                tmpdir.y/=tmpNorm;
                tmpdir.z/=tmpNorm;
            }
            canddir.append(tmpdir);
        }
    }

    qDebug()<<connNum.size()<<":"<<childNum.size()<<":"<<cand.size();

    qDebug()<<"match tips";
    //match tips
    multimap<double, QVector<V3DLONG> > connMap;
    for(V3DLONG tid=0; tid<cand.size(); tid++){
        V3DLONG tidx=cand.at(tid);
        for(V3DLONG cid=0; cid<curid; cid++){
            if(cid==components.at(cand[tid])) continue;
            double mvdis=disThr, mtdis=disThr;
            V3DLONG mvid=-1, mtid=-1;
            V3DLONG id=components.indexOf(cid);
            while(id>=0){
                if(matchType==1){ //must be the same type to connect
                    if(nt->listNeuron.at(id).type!=nt->listNeuron.at(tidx).type){
                        id=components.indexOf(cid, id+1);
                        continue;
                    }
                }
                if(matchType==2){ //must be the same type except soma
                    if(nt->listNeuron.at(id).type!=nt->listNeuron.at(tidx).type &&
                            nt->listNeuron.at(id).type!=1 && nt->listNeuron.at(tidx).type!=1){
                        id=components.indexOf(cid, id+1);
                        continue;
                    }
                }
                if(matchType==3){ //must be the different type to connect
                    if(nt->listNeuron.at(id).type==nt->listNeuron.at(tidx).type){
                        id=components.indexOf(cid, id+1);
                        continue;
                    }
                }
                double dis=NTDIS(scaledXYZ.at(tidx),scaledXYZ.at(id));
                if(b_minusradius){
                    dis-=nt->listNeuron.at(id).radius;
                    dis-=nt->listNeuron.at(tidx).radius;
                }
                if(dis<mvdis){
                    mvdis=dis;
                    mvid=id;
                }
                if(dis<mtdis){
                    if(connNum.at(id)<1){//tips
                        V3DLONG tmpid=cand.indexOf(id);
                        if(tmpid<0){//should not happen, just in case
                            qDebug()<<"unexpected error: cannot locate dead end in candidate list, please check code."<<tid<<":"<<cid<<":"<<id;
                            id=components.indexOf(cid, id+1);
                            continue;
                        }
                        if(NTDOT(canddir.at(tid),canddir.at(tmpid))<angThr){
                            mtdis=dis;
                            mtid=id;
                        }
                    }
                }
                id=components.indexOf(cid, id+1);
            }
//            if(b_connectall){
//                if(mvid>=0){
//                    QVector<V3DLONG> tmp;
//                    tmp.append(tidx); tmp.append(mvid);
//                    connMap.insert(pair<double, QVector<V3DLONG> >(mvdis,tmp));
//                }
//            }else{
                if(mvid>=0){
                    QVector<V3DLONG> tmp;
                    tmp.append(tidx); tmp.append(mvid);
                    connMap.insert(pair<double, QVector<V3DLONG> >(mvdis+disThr,tmp));
                }
                if(mtid>=0){
                    QVector<V3DLONG> tmp;
                    tmp.append(tidx); tmp.append(mtid);
                    connMap.insert(pair<double, QVector<V3DLONG> >(mtdis,tmp));
                }
//            }
        }
    }

    qDebug()<<"connecting tips";
    //find the best solution for connecting tips
    QMap<V3DLONG, QVector<V3DLONG> > connectPairs;
    for(multimap<double, QVector<V3DLONG> >::iterator iter=connMap.begin(); iter!=connMap.end(); iter++){
        if(components.at(iter->second.at(0))==components.at(iter->second.at(1))) //already connected
            continue;
        if(matchType==2){ //check soma axon connection
            if(nt->listNeuron.at(iter->second.at(0)).type * nt->listNeuron.at(iter->second.at(1)).type == 2){ //is soma axon connection
                if(b_somaaxon) //already has soma axon connection
                    continue;
                else
                    b_somaaxon=true;
            }
        }
        if(connectPairs.contains(iter->second.at(0))){
            connectPairs[iter->second.at(0)].append(iter->second.at(1));
        }else{
            QVector<V3DLONG> tmp; tmp.append(iter->second.at(1));
            connectPairs.insert(iter->second.at(0),tmp);
        }
        if(connectPairs.contains(iter->second.at(1))){
            connectPairs[iter->second.at(1)].append(iter->second.at(0));
        }else{
            QVector<V3DLONG> tmp; tmp.append(iter->second.at(0));
            connectPairs.insert(iter->second.at(1),tmp);
        }
        V3DLONG cid_0=components.at(iter->second.at(0));
        V3DLONG cid_1=components.at(iter->second.at(1));
        V3DLONG tmpid=components.indexOf(cid_1);
        while(tmpid>=0){
            components[tmpid]=cid_0;
            tmpid=components.indexOf(cid_1,tmpid+1);
        }
    }

    qDebug()<<"reconstruct neuron tree";
    //reconstruct tree
    QVector<V3DLONG> newid(nt->listNeuron.size(), -1);
    QVector<V3DLONG> newpn(nt->listNeuron.size(), -1); //id starts from 1, -1: not touched, 0: touched but overlap with parent
    curid=1;
    int rootidx=nt->hashNeuron.value(rootID);
    if(nt->listNeuron[rootidx].n != rootID)
        rootidx=-1;
    QVector<V3DLONG> prinode;
    if(rootidx!=-1){
        prinode.push_back(rootidx);
    }
    for(V3DLONG i=0; i<nt->listNeuron.size(); i++){
        if(nt->listNeuron[i].parent==-1){
            prinode.push_back(i);
        }
    }
    V3DLONG i=0;
    V3DLONG priIdx=0;
    while(1){
        if(priIdx<prinode.size()){
            i=prinode[priIdx];
            priIdx++;
        }else if(priIdx==prinode.size()){
            i=0;
            priIdx++;
        }else{
            i++;
            if(i>=nt->listNeuron.size())
                break;
        }
        if(newid[i]>0) continue;
        QQueue<V3DLONG> pqueue; pqueue.clear();
        pqueue.enqueue(i);
        newid[i]=curid++;
        while(!pqueue.isEmpty()){
            //add current node to the listNeuron
            V3DLONG oid=pqueue.dequeue();

            if(newid[oid]>0){
                NeuronSWC tmpNeuron;
                tmpNeuron.n = newid[oid];
                tmpNeuron.x = nt->listNeuron.at(oid).x;
                tmpNeuron.y = nt->listNeuron.at(oid).y;
                tmpNeuron.z = nt->listNeuron.at(oid).z;
                tmpNeuron.type = nt->listNeuron.at(oid).type;
                tmpNeuron.r = nt->listNeuron.at(oid).r;
                tmpNeuron.fea_val = nt->listNeuron.at(oid).fea_val;
                tmpNeuron.pn = newpn.at(oid);
                newNeuron.append(tmpNeuron);
            }

            //add current node's children/parent/new-neighbor to the stack
            //parent
            if(nt->listNeuron.at(oid).pn>=0){
                V3DLONG opid = nt->hashNeuron.value(nt->listNeuron.at(oid).pn);
                if(newid.at(opid)<0){
                    pqueue.enqueue(opid);
                    newpn[opid]=newid[oid];
                    newid[opid]=curid++;
                }
            }
            //child
            V3DLONG tmpid=pList.indexOf(oid);
            while(tmpid>=0){
                if(newid.at(tmpid)<0){
                    pqueue.enqueue(tmpid);
                    newpn[tmpid]=newid[oid];
                    newid[tmpid]=curid++;
                }
                tmpid=pList.indexOf(oid,tmpid+1);
            }
            //new-neighbor
            if(connectPairs.contains(oid)){
                for(V3DLONG j=0; j<connectPairs[oid].size(); j++){
                    V3DLONG onid=connectPairs[oid].at(j);
                    if(newid.at(onid)<0){
                        pqueue.enqueue(onid);
                        newpn[onid]=newid[oid];
                        newid[onid]=curid++;
                    }
                }
            }
        }
    }
}
#endif
