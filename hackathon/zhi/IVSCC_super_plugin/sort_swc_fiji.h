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
#include "../neurontracing_mip/smooth_curve.h"
#include "../../../released_plugins/v3d_plugins/neuron_radius/hierarchy_prune.h"

using namespace std;

#ifndef VOID
#define VOID 1000000000
#endif

#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))

template <class T> T pow2(T a)
{
    return a*a;

}


QHash<V3DLONG, V3DLONG> ChildParent(QList<NeuronSWC> &neurons, const QList<V3DLONG> & idlist, const QHash<V3DLONG,V3DLONG> & LUT) 
{
	QHash<V3DLONG, V3DLONG> cp;
    for (V3DLONG i=0;i<neurons.size(); i++)
    {
        if (neurons.at(i).pn==-1)
            cp.insertMulti(idlist.indexOf(LUT.value(neurons.at(i).n)), -1);
        else if(idlist.indexOf(LUT.value(neurons.at(i).pn)) == 0 && neurons.at(i).pn != neurons.at(0).n)
            cp.insertMulti(idlist.indexOf(LUT.value(neurons.at(i).n)), -1);
        else
            cp.insertMulti(idlist.indexOf(LUT.value(neurons.at(i).n)), idlist.indexOf(LUT.value(neurons.at(i).pn)));
    }
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

NeuronTree SortSWC(QList<NeuronSWC> & neurons, V3DLONG newrootid, double thres)
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
	V3DLONG root;
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
							min = dist2;
							mingroup = numbered[jj];
							m1 = ii;
							m2 = jj;
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
#endif
