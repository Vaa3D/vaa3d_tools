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
using namespace std;

#ifndef VOID
#define VOID 1000000000
#endif

#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))


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

NeuronTree prunswc(NeuronTree nt, double length)
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
