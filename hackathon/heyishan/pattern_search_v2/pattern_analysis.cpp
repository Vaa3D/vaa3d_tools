#include "pattern_analysis.h"
#include "../pattern_search/my_sort.h"

double dist_p2p(double x1, double y1, double z1, double x2, double y2, double z2)
{
    return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2));
}

NeuronTree fill_boundary(const NeuronTree &nt, const Boundary & b)
{
    V3DLONG siz = nt.listNeuron.size();
    NeuronTree res;
    for(V3DLONG i=0; i<siz; i++)
    {
        NeuronSWC curr = nt.listNeuron[i];
        if(curr.x>b.minx && curr.y>b.miny && curr.z>b.minz &&curr.x<b.maxx && curr.y<b.maxy&&curr.z<b.maxz)
            res.listNeuron.push_back(curr);
    }
    return res;
}

NeuronTree rmSmallPart(NeuronTree & nt_sorted)
{
    V3DLONG tot_len = nt_sorted.listNeuron.size();
    vector<V3DLONG> sublens;
    vector<V3DLONG> root_ids;
    V3DLONG max_sublen = 0;
    V3DLONG sublen;
    for(V3DLONG j=0; j<tot_len; j++)
    {
        NeuronSWC cur = nt_sorted.listNeuron[j];
        if(cur.pn == -1)
        {
            sublens.push_back(1);
            root_ids.push_back(j);
            sublen = 0;
        }
        sublen += 1;
        if(sublen > max_sublen)
            max_sublen = sublen;
        sublens.back() = sublen;

    }
    V3DLONG rm_thres = max_sublen / 5;
    NeuronTree nt_res; //sub means the biggest connected subtree in area
    for(V3DLONG j=0; j<sublens.size(); j++)
    {
        if(sublens[j] >rm_thres)
        {
            for(V3DLONG k=root_ids[j]; k<sublens[j]; k++)
                nt_res.listNeuron.push_back(nt_sorted.listNeuron[k]);
        }
    }
    if(nt_res.listNeuron.size()<1) return nt_sorted;
    V3DLONG root_id=nt_res.listNeuron[0].n;
    nt_res = sort(nt_res,root_id,0);
    return nt_res;
}

bool pattern_analysis(const NeuronTree &nt,const NeuronTree &boundary,vector<NeuronTree> & pt_list, vector<double> & pt_lens, vector<int>& pt_nums, V3DPluginCallback2 &callback)
{
    V3DLONG boundary_size = boundary.listNeuron.size();
    V3DLONG nt_size= nt.listNeuron.size();
    if(boundary_size==0 || nt_size==0)
    {
        v3d_msg("Input is empty, please retry");
        return false;
    }

    // get the boundary of each area of interest
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

    for(int i=0; i<v_boundary.size(); i++)
    {
        // get the boundary of large area
        Boundary cur = v_boundary[i];
        Boundary large_boundary;
        double c_x = cur.minx + (cur.maxx - cur.minx) /2.0;
        double c_y = cur.miny + (cur.maxy - cur.miny) /2.0;
        double c_z = cur.minz + (cur.maxz - cur.minz) /2.0;
        double d;
        //d = max(cur.maxx - cur.minx, cur.maxy - cur.miny);
        //d = max(float(d), cur.maxz - cur.minz);
        d = (cur.maxx -cur.minx + cur.maxy - cur.miny + cur.maxz - cur.minz) / 3.0;
        large_boundary.maxx = c_x + d; large_boundary.maxy = c_y + d; large_boundary.maxz = c_z +d;
        large_boundary.minx = c_x - d; large_boundary.miny = c_y - d; large_boundary.minz = c_z - d;

        cout<<"boundary:"<<endl;
        cout<<cur.maxx<<"   "<<cur.maxy<<"  "<<cur.maxz<<endl;
        cout<<"large_boundary:"<<endl;
        cout<<large_boundary.maxx<<"   "<<large_boundary.maxy<<"  "<<large_boundary.maxz<<endl;

        // find the nearest point away from boundary center
        NeuronTree big_area  = fill_boundary(nt, large_boundary);
        double min_dist = VOID;
        NeuronSWC c_point;
        for(V3DLONG j=0; j<big_area.listNeuron.size(); j++)
        {
            NeuronSWC p = big_area.listNeuron[j];
            double tmp_dist =  dist_p2p(p.x,p.y,p.z,c_x,c_y,c_z);
            if(tmp_dist<min_dist)
            {
                min_dist = tmp_dist;
                c_point = p;
            }
        }
        // get pattern boundary and pattern area
        Boundary b_pattern;
        NeuronTree area_pattern;
        b_pattern.maxx = c_point.x + d/2.0; b_pattern.maxy = c_point.y + d/2.0; b_pattern.maxz = c_point.z + d/2.0;
        b_pattern.minx = c_point.x - d/2.0; b_pattern.miny = c_point.y - d/2.0; b_pattern.minz = c_point.z - d/2.0;
        area_pattern = fill_boundary(big_area, b_pattern);

        // pattern connect
        if(area_pattern.listNeuron.size()<=1) {cout<<"the number of pattern within this boundary isn't more than 1"<<endl; continue;}
        NeuronTree pattern_sorted;
        double sort_thres = d/10.0;
        pattern_sorted.listNeuron.clear();
        pattern_sorted.hashNeuron.clear();
        V3DLONG root_id=area_pattern.listNeuron[0].n;
        pattern_sorted = sort(area_pattern, root_id,sort_thres);

        // remove small part   ps:It would be better to transfer to a function
         NeuronTree pattern_res = rmSmallPart(pattern_sorted);

        // out pattern
        pt_list.push_back(pattern_res);
        pt_lens.push_back(d);
        pt_nums.push_back(pattern_res.listNeuron.size());
    }

    return true;
}
