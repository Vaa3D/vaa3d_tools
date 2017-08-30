#include"get_subtrees.h"
#include "pattern_analysis.h"
#include <qstack.h>
#include <stdlib.h>
#include <math.h>
#include <set>
#include "../pattern_search/my_sort.h"

Boundary getBoundary(const NeuronTree &nt)
{
    V3DLONG siz = nt.listNeuron.size();
    Boundary b;
    b.minx=VOID; b.miny=VOID;b.minz=VOID;
    b.maxx=0;b.maxy=0;b.maxz=0;
    for(int i=0; i<siz; i++)
    {
        b.minx=min(nt.listNeuron[i].x,b.minx);
        b.miny=min(nt.listNeuron[i].y,b.miny);
        b.minz=min(nt.listNeuron[i].z,b.minz);
        b.maxx=max(nt.listNeuron[i].x,b.maxx);
        b.maxy=max(nt.listNeuron[i].y,b.maxy);
        b.maxz=max(nt.listNeuron[i].z,b.maxz);
    }
    return b;
}


bool get_subtrees(const NeuronTree &nt, vector<NeuronTree> &sub_trees, double boundary_length, int pt_num,vector<vector<V3DLONG> >&p_to_tree)
{
    if(nt.listNeuron.size()==0)
    {
        cout<<"Neuron Tree is empty"<<endl;
        return false;
    }

    // split the space
    const int split_num = 10;
    double t[3];    // interval
    double r = boundary_length/2.0;
    Boundary b = getBoundary(nt);
    t[0] = (b.maxx - b.minx) / split_num;
    t[1] = (b.maxy - b.miny) / split_num;
    t[2] = (b.maxz - b.minz) / split_num;
    double m[3][split_num];
    double n[3][split_num];
    double bound[6] = {b.minx,b.maxx,b.miny,b.maxy,b.minz,b.maxz};
    for(int i=0; i<3; i++)
    {
        for(int j=0; j<split_num; j++)
        {
            m[i][j] = bound[i*2] + j*t[i] - r;
            n[i][j] = bound[i*2] + (j+1)*t[i] + r;
        }
    }
    //cout<<"r="<<r<<endl;
    //cout<<"t="<<t[0]<<"  "<<t[1] <<"  "<<t[2]<<endl;

    // put points into area
    vector<vector<int> > b2p(split_num*split_num*split_num);
    vector<vector<V3DLONG> > p2b(nt.listNeuron.size());
    for(V3DLONG i=0; i<nt.listNeuron.size(); i++)
    {
        NeuronSWC c = nt.listNeuron[i];
        int len[3]; for(int j=0; j<3; j++) len[j]=1; // store the number of cube where the point at on each dimension
        int h[3][2];
        double point[3] = {c.x,c.y,c.z};
        for(int d=0; d<3; d++)
        {
            h[d][0] = max(0, int((point[d] - m[d][0] -2*r)/t[d]));
            h[d][1] = min(split_num-1, int((point[d] - n[d][0] + 2*r)/t[d]));
            if(h[d][0] != h[d][1]) len[d] = 2;
            //cout<<h[d][0]<<"   "<<h[d][1]<<endl;
        }
        //cout<<endl<<len[0]<<"  "<<len[1] <<"  "<<len[2]<<endl;
        for(int d3=0; d3<len[2]; d3++){                 // z dim
            for(int d2=0; d2<len[1]; d2++ ){            // y dim
                for(int d1=0; d1<len[0]; d1++){         // x dim
                    int order = (h[2][d3] * split_num +h[1][d2]) * split_num +h[0][d1];
                    p2b[i].push_back(order);
                    b2p[order].push_back(i);
                }
            }
        }
    }

    // get subtrees
    double step_radio= 0.05;
    //int search_step = int(boundary_length * step_radio);
    //search_step=1;
    int search_step = 9;
    if(search_step<2)   search_step=2;
    cout<<"search_step="<<search_step<<endl;

    // get subtrees in whole nt
    for(V3DLONG i=0; i<nt.listNeuron.size() - search_step; i+=search_step)
    {
        NeuronTree s_tree;
        vector<V3DLONG> points;
        NeuronSWC cur_point = nt.listNeuron[i];
        Boundary cur_boundary;
        cur_boundary.minx = cur_point.x - r; cur_boundary.miny = cur_point.y - r; cur_boundary.minz = cur_point.z - r;
        cur_boundary.maxx = cur_point.x + r; cur_boundary.maxy = cur_point.y + r; cur_boundary.maxz = cur_point.z + r;
        // for each cube
        set<int> points_set;
        for(int j=0; j<p2b[i].size(); j++)
        {
            int b_ind = p2b[i][j];
            // for each points in the cube
            for(int k=0; k<b2p[b_ind].size(); k++)
                points_set.insert(b2p[b_ind][k]);
        }
        //cout<<"set_size="<<points_set.size()<<endl;
        // put
        set<int>::iterator set_iter = points_set.begin();
        for( ; set_iter!=points_set.end(); set_iter++)
        {
            int ind = *set_iter;
            NeuronSWC tmp_p = nt.listNeuron[ind];
            if(tmp_p.x>cur_boundary.minx && tmp_p.y>cur_boundary.miny && tmp_p.z>cur_boundary.minz &&tmp_p.x<cur_boundary.maxx
                    &&tmp_p.y<cur_boundary.maxy&&tmp_p.z<cur_boundary.maxz)
            {
                s_tree.listNeuron.push_back(tmp_p);
                points.push_back(ind);
            }
        }
        // subtree connect
        if(s_tree.listNeuron.size()<=1) {cout<<"the number of subtree within this boundary isn't more than 1"<<endl; continue;}
        if(s_tree.listNeuron.size() > 1.3*pt_num || s_tree.listNeuron.size()<0.7 * pt_num) continue;
        NeuronTree subtree_sorted;
        double sort_thres = boundary_length/10.0;
        subtree_sorted.listNeuron.clear();
        subtree_sorted.hashNeuron.clear();
        V3DLONG root_id=s_tree.listNeuron[0].n;
        subtree_sorted = sort(s_tree, root_id,sort_thres);

        // remove small part
        NeuronTree subtree_res = rmSmallPart(subtree_sorted);
        //NeuronTree subtree_res = subtree_sorted;
        // out subtree
        p_to_tree.push_back(points);
        sub_trees.push_back(subtree_res);
        points_set.clear();
        //QString name = "subtree_" + QString::number(i) + ".swc";
        //writeSWC_file(name,s_tree);
    }

    cout<<"nt_size"<<nt.listNeuron.size()<<endl;
    cout<<"sub_trees="<<sub_trees.size()<<endl;
    //v3d_msg("size");
    return true;
}
