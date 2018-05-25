/*
 *  sort_func.cpp
 *  core functions for sort neuron swc 
 *
 *  2018-05-24 : by Linus Manubens Gil
 *  adapted from 2012-02-01 : by Yinan Wan
 */

#ifndef __SORT_SWC_LMG_H_
#define __SORT_SWC_LMG_H_

#include <QtGlobal>
#include <math.h>
//#include <unistd.h> //remove the unnecessary include file. //by PHC 20131228
#include "basic_surf_objs.h"
#include <string.h>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <iostream>
using namespace std;

#ifndef VOID
#define VOID 1000000000
#endif

//#define PI 3.14159265359
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
#define NTDOT(a,b) ((a).x*(b).x+(a).y*(b).y+(a).z*(b).z)
#define angle(a,b,c) (acos((((b).x-(a).x)*((c).x-(a).x)+((b).y-(a).y)*((c).y-(a).y)+((b).z-(a).z)*((c).z-(a).z))/(NTDIS(a,b)*NTDIS(a,c)))*180.0/3.14159265359)

#ifndef MAX_DOUBLE
#define MAX_DOUBLE 1.79768e+308        //actual: 1.79769e+308
#endif

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

bool SortSWC(QList<NeuronSWC> & neuron, QList<NeuronSWC> & result, V3DLONG newrootid, double thres, double root_dist_thres)
{

    //get all root ids and parents of each node
    QList<V3DLONG> rootlist;
    vector<long> parents;
    vector<long> ids;
    QList<V3DLONG> tips;
    //QList<NeuronSWC> neuron = neurons;
    //get ids and reorder tree with ids following list
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        ids.push_back(neuron.at(i).n);
        if(neuron.at(i).n==newrootid)
        {
            newrootid=i;
        }
        //qDebug()<<neuron.at(i).n<<newrootid;
    }
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        neuron[i].n=i+1;
        if(neuron.at(i).pn ==-1)
        {
            rootlist.push_back(i);//neuron.at(i).n-1);
        }
        else
        {
            neuron[i].pn=find(ids.begin(), ids.end(),neuron.at(i).pn) - ids.begin()+1;
            parents.push_back(neuron.at(i).pn);
        }
    }
    qDebug()<<"Root ids and parents found";

    //check if new root id is valid and take first root in the list otherwise
    if(newrootid!=VOID)
    {
        //newrootid = newrootid;
        if(neuron.at(newrootid).pn==-1)
        {
            qDebug()<<"New root id is:" << ids.at(newrootid);
        }
    }
    else
    {
        newrootid = rootlist.at(0);
        qDebug()<<"Selected node is not a root, taking first root node:" << newrootid+1;
    }

    //connect all local roots directly to soma
    double dist2 = 0;
    int counter = 0;
    for(int i=0;i<rootlist.size();i++)
    {
        //qDebug()<<i<<rootlist.at(5)<<neuron.size();
        dist2 = computeDist2(neuron.at(rootlist.at(i)),neuron.at(newrootid));
        if(dist2 <= root_dist_thres && rootlist.at(i) != newrootid)
        {
            neuron[rootlist.at(i)].pn = newrootid+1;
            counter++;
        }
    }
    qDebug()<<counter<<"local roots connected";

    //find branches
    QList<V3DLONG>  child_num;
    for(V3DLONG i=0;i<neuron.size();i++)// 0 or 1? check!
    {
        child_num.push_back(count(parents.begin(),parents.end(),neuron.at(i).n));
    }

    //assign segids
    vector<int> rootsegids;
    int segid = 0;
    V3DLONG walknode = 0;
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        if(child_num.at(i) ==0)
        {
            walknode = i;
            neuron[walknode].seg_id = segid;
            while(neuron.at(walknode).pn!=-1)
            {
                walknode = neuron.at(walknode).pn-1;//find(ids.begin(), ids.end(),neuron.at(walknode).pn) - ids.begin();
                neuron[walknode].seg_id = segid;
                //qDebug()<<walknode<<segid;
            }
            neuron[walknode].seg_id = segid;
            if(neuron.at(walknode)==newrootid)
            {
                rootsegids.push_back(segid);
            }
            segid++;
        }
    }
    //qDebug()<<neuron.at(0).seg_id<<neuron.at(newrootid).seg_id;
    qDebug()<<"segids assigned";
    int root_segid = neuron.at(newrootid).seg_id;
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        if(neuron.at(i).seg_id==0)
        {
            neuron[i].seg_id = root_segid;
        }
        else if(neuron.at(i).seg_id==root_segid)
        {
            neuron[i].seg_id = 0;
        }
    }
    //qDebug()<<neuron.at(0).seg_id<<neuron.at(1).seg_id<<neuron.at(newrootid).seg_id<<newrootid<<root_segid;
    qDebug()<<"segids swapped";
    //change other root segments
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        if(count(rootsegids.begin(),rootsegids.end(),neuron.at(i).seg_id)!=0)
        {
            neuron[i].seg_id = 0;
        }
    }
    qDebug()<<"Root segid assigned to 0";

    //connect remaining segments to closest neuron nodes
    double min = VOID;
    int pid = VOID;
    int cid = VOID;
    QList<V3DLONG>  segidtips;
    if(thres==VOID)
    {
        thres=MAX_DOUBLE;
    }
    //qDebug()<<neuron.at(newrootid).seg_id;

    //get tips of each segment
    for(V3DLONG ii=0;ii<=segid;ii++)
    {
        for(V3DLONG i=0;i<neuron.size();i++)
        {
            //get tips
            if(child_num.at(i)==0)
            {
                segidtips.push_back(i);
            }
        }
    }
    qDebug()<<"tips found";

    for(V3DLONG ii=1;ii<segid;ii++)
    {
        //qDebug()<<ii<<count(rootsegids.begin(),rootsegids.end(),ii);
        if(count(rootsegids.begin(),rootsegids.end(),ii)==0)
        {
            dist2 = MAX_DOUBLE;
            min=VOID;
            //int pidchild = VOID;
            cout << "\r" <<"Connecting segments: "<< ii << " /" << segid;
            for(V3DLONG i=0;i<neuron.size();i++)
            {
                if(neuron.at(i).seg_id==ii && neuron.at(i).seg_id!=neuron.at(newrootid).seg_id)//only one segment that must be different from newroot's
                {
                    for(V3DLONG j=0;j<neuron.size();j++)
                    {
                        if(neuron.at(i).seg_id!=neuron.at(j).seg_id)// && segidparents.at(neuron.at(j).seg_id)!=neuron.at(i).seg_id)//last condition to avoid loops in segments
                        {
                            dist2 = computeDist2(neuron.at(i),neuron.at(j)); // check 0 1
                            if(dist2 < min && dist2 <= thres*thres)// && connectednodes.at(j)==0)
                            {
                                min = dist2;
                                cid = i;
                                pid = j;
                            }
                            //qDebug()<<cid+1<<pid+1<<neuron.at(i).pn<<neuron.at(j).pn;
                        }
                    }
                }
                //qDebug()<<i<<neuron.at(i).seg_id;
            }
            //qDebug()<<"Distances obtained";
            //qDebug()<<cid+1<<pid+1<<neuron.at(cid).pn<<neuron.at(pid).pn;
            if(cid != newrootid)//child node cannot be newroot
            {
                if(neuron.at(cid).pn==-1)// && neuron[pid].pn=!-1)//child node is root
                {
                    neuron[cid].pn = neuron.at(pid).n;
                    //qDebug()<<"Child node is a root";
                }
                else //if(child_num.at(cid)==0) //the child is a tip or a lonely root, if a tip the branch has to be reordered
                {
                    int walknode1 = pid;
                    int maxit = 0;
                    int walknode2 = cid;
                    ptrdiff_t walknode3 = neuron.at(walknode2).pn-1;//find(ids.begin(), ids.end(),neuron.at(walknode2).pn) - ids.begin();
                    //neuron[walknode2].pn = walknode1+1;
                    //qDebug()<<walknode1+1<<walknode2+1<<walknode3+1;
                    while(neuron.at(walknode3).pn!=-1 && neuron.at(walknode3).seg_id==neuron.at(cid).seg_id && maxit<neuron.size())// && neuron.at(walknode3).seg_id==segid) //is last condition necessary?
                    {
                        walknode1 = walknode2;
                        walknode2 = walknode3;
                        walknode3 = neuron.at(walknode2).pn-1;//find(ids.begin(), ids.end(), neuron.at(walknode2).pn) - ids.begin();
                        //walknode3 = neuron.at(walknode2).pn-1;
                        neuron[walknode2].pn = walknode1+1;//neuron.at(walknode1).n;
                        //qDebug()<<walknode1+1<<walknode2+1<<walknode3+1;
                        maxit++;
                    }
                    if(maxit==neuron.size())
                    {
                        qDebug()<<"Careful, while broke";
                    }
                    if(walknode3!=newrootid)// && neuron.at(walknode3).seg_id==segid)//is this condition necessary?
                    {
                        neuron[walknode3].pn = walknode2+1;//neuron.at(walknode2).n;
                    }
                    neuron[cid].pn = pid+1;//neuron.at(pid).n;

                }
                int parentsegid = neuron.at(pid).seg_id;
                for(V3DLONG i=0;i<neuron.size();i++) //update subtree segid to avoid loops
                {
                    if(neuron.at(i).seg_id==ii)
                    {
                        neuron[i].seg_id=parentsegid;
                    }
                }
                //qDebug()<<parentsegid;
            }
        }
    }
    qDebug()<< "\n" << "\n" << "\n" << "Connected all roots below threshold";

    //sort tree
    int rootnum = 0;
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        if(neuron.at(i).pn==-1) rootnum++;
    }
    if(rootnum!=1)
    {
        qDebug()<<rootnum;
        v3d_msg("Error, 0 or more than 1 root!");
        return(false);
    }
    else
    {
        //QList<V3DLONG> nodestoremove;
        for(V3DLONG i=0;i<neuron.size();i++)// 0 or 1? check!
        {
            child_num[i] = count(parents.begin(),parents.end(),neuron.at(i).n);
        }
        //get path lengths and topological distances
        //walk down from each node to root and get topological path distance
        QList<V3DLONG> pl;
        QList<V3DLONG> lo;
        for(V3DLONG i=0;i<neuron.size();i++) // 0 or 1? check!
        {
            ptrdiff_t walknode = i;
            pl.push_back(0);
            lo.push_back(0);
            int maxit=0;
            //qDebug()<<"it's the while";
            while(walknode!=newrootid && maxit<neuron.size())
            {
                walknode = neuron.at(walknode).pn-1;//find(ids.begin(), ids.end(), neuron.at(walknode).pn) - ids.begin();
                //walknode = neuron.at(walknode).pn-1;
                pl[i]++;
                //cout << "it's the child number " << walknode << " " << newrootid << " " << neuron.at(walknode).seg_id << "\n";
                if(child_num.at(walknode)>1)
                {
                    //lo[i]+=child_num.at(walknode);
                    lo[i]++;
                }
                maxit++;
            }
            if(maxit==neuron.size())
            {
                neuron[walknode].pn = -1;
                qDebug()<<"Careful, while broke"<<neuron.at(walknode).n;
            }
        }
        qDebug()<< "Sorting..." <<"Path and topological lengths obtained";

        pl[newrootid] = 0;
        lo[newrootid] = 0;
        //sort nodes by path length
        //QList<NeuronSWC> unsort_neuron = neuron;
        for (V3DLONG i = 0; i<neuron.size(); i++)
        {
           for (V3DLONG j = 1; j < neuron.size() - i; j++)
           {
              if (pl.at(j - 1) > pl.at(j))// && j!=newrootid)
              {
                 swap(pl[j - 1], pl[j]);
                 swap(lo[j - 1], lo[j]);
                 swap(neuron[j - 1].n, neuron[j].n);
                 swap(neuron[j - 1].pn, neuron[j].pn);
                 swap(neuron[j - 1].x, neuron[j].x);
                 swap(neuron[j - 1].y, neuron[j].y);
                 swap(neuron[j - 1].z, neuron[j].z);
                 swap(neuron[j - 1].r, neuron[j].r);
                 swap(neuron[j - 1].type, neuron[j].type);
              }
           }
        }
        qDebug()<<"Sorting..." << "Sorted by topological path length";
        // and level order
        for (V3DLONG i = 0; i<neuron.size(); i++)
        {
           for (V3DLONG j = 1; j < neuron.size() - i; j++)
           {
              if (lo.at(j - 1) > lo.at(j))// && j!=newrootid)
              {
                 swap(pl[j - 1], pl[j]);
                 swap(lo[j - 1], lo[j]);
                 swap(neuron[j - 1].n, neuron[j].n);
                 swap(neuron[j - 1].pn, neuron[j].pn);
                 swap(neuron[j - 1].x, neuron[j].x);
                 swap(neuron[j - 1].y, neuron[j].y);
                 swap(neuron[j - 1].z, neuron[j].z);
                 swap(neuron[j - 1].r, neuron[j].r);
                 swap(neuron[j - 1].type, neuron[j].type);
              }
           }
        }
        qDebug()<<"Sorting..." << "Sorted by level order";
        //adjust parent nodes accordingly
        //create ordered oldids vector
        vector<int> ordoldids;
        for (V3DLONG j = 0; j<neuron.size(); j++)
        {
            ordoldids.push_back(neuron.at(j).n);
        }
        neuron[0].n=1;
        neuron[0].pn=-1;
        int newpn;
        for (V3DLONG j = 1; j<=neuron.size(); j++)
        {
            ptrdiff_t newpn = find(ordoldids.begin(), ordoldids.end(), neuron.at(j-1).pn) - ordoldids.begin()+1;
            neuron[j-1].pn=newpn;
            neuron[j-1].n=j;
        }
        neuron[0].pn=-1;
        qDebug()<<"Sorted swc";

        //construct neuron
        for (V3DLONG i = 0; i<neuron.size(); i++)
        {
            result.append(neuron.at(i));
        }
    }
    return true;
};

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
