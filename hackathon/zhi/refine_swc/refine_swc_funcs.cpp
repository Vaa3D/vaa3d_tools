/* refine_swc_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-12-6 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "refine_swc_funcs.h"

#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.h"
#include "../../../released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h"

using namespace std;
#define DISTP(a,b) sqrt(((a)->x-(b)->x)*((a)->x-(b)->x)+((a)->y-(b)->y)*((a)->y-(b)->y)+((a)->z-(b)->z)*((a)->z-(b)->z))
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
#define angle(a,b,c) (acos((((b).x-(a).x)*((c).x-(a).x)+((b).y-(a).y)*((c).y-(a).y)+((b).z-(a).z)*((c).z-(a).z))/(NTDIS(a,b)*NTDIS(a,c)))*180.0/3.14159265359)
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))

NeuronTree refineSWCTerafly(V3DPluginCallback2 &callback,QString fname_img, NeuronTree nt)
{
    NeuronTree result;
    V3DLONG siz = nt.listNeuron.size();
    Tree tree;
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron[i];
        Point* pt = new Point;
        pt->x = s.x;
        pt->y = s.y;
        pt->z = s.z;
        pt->r = s.r;
        pt ->type = s.type;
        pt->seg_id = s.seg_id;
        pt->level = s.level;
        pt->fea_val = s.fea_val;
        pt->p = NULL;
        pt->childNum = 0;
        tree.push_back(pt);
    }
    for (V3DLONG i=0;i<siz;i++)
    {
        if (nt.listNeuron[i].pn<0) continue;
        V3DLONG pid = nt.hashNeuron.value(nt.listNeuron[i].pn);
        tree[i]->p = tree[pid];
        tree[pid]->childNum++;
    }
//	printf("tree constructed.\n");
    vector<Segment*> seg_list;
    for (V3DLONG i=0;i<siz;i++)
    {
        if (tree[i]->childNum!=1)//tip or branch point
        {
            Segment* seg = new Segment;
            Point* cur = tree[i];
            do
            {
                seg->push_back(cur);
                cur = cur->p;
            }
            while(cur && cur->childNum==1);
            seg_list.push_back(seg);
        }
    }

    double weight_xy_z=1.0;
    CurveTracePara trace_para;
    trace_para.channo = 0;
    trace_para.sp_graph_background = 0;
    trace_para.b_postMergeClosebyBranches = false;
    trace_para.b_3dcurve_width_from_xyonly = true;
    trace_para.b_pruneArtifactBranches = false;
    trace_para.sp_num_end_nodes = 2;
    trace_para.b_deformcurve = false;
    trace_para.sp_graph_resolution_step = 1;
    trace_para.b_estRadii = false;


    V3DLONG start_x,start_y,start_z,end_x,end_y,end_z;
    for(V3DLONG i=0; i<seg_list.size();i++)
    {
        Segment* seg = seg_list[i];

        start_x = seg->at(0)->x;
        end_x = seg->at(0)->x;
        start_y = seg->at(0)->y;
        end_y = seg->at(0)->y;
        start_z = seg->at(0)->z;
        end_z = seg->at(0)->z;
        for(V3DLONG j=1; j<seg->size();j++)
        {
            if(start_x>seg->at(j)->x)  start_x = seg->at(j)->x;
            if(end_x<seg->at(j)->x)  end_x = seg->at(j)->x;
            if(start_y>seg->at(j)->y)  start_y = seg->at(j)->y;
            if(end_y<seg->at(j)->y)  end_y = seg->at(j)->y;
            if(start_z>seg->at(j)->z)  start_z = seg->at(j)->z;
            if(end_z<seg->at(j)->z)  end_z = seg->at(j)->z;
        }

        start_x -= 50;
        end_x   +=50;
        start_y -= 50;
        end_y   +=50;
        start_z -= 50;
        end_z   +=50;

        unsigned char * total1dData = 0;
        total1dData = callback.getSubVolumeTeraFly(fname_img.toStdString(),start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
        V3DLONG mysz[4];
        mysz[0] = end_x-start_x+1;
        mysz[1] = end_y-start_y+1;
        mysz[2] = end_z-start_z+1;
        mysz[3] = 1;

        resample_path(seg, 2.0);
        //convert seg to nt;
        NeuronTree seg_nt;
        for(int ii=0; ii<seg->size(); ii++)
        {
            Point* p = seg->at(ii);
            NeuronSWC S;
            S.n 	= ii + 1;
            S.type 	= p->type;
            S.x 	= p->x - start_x;
            S.y 	= p->y - start_y;
            S.z 	= p->z - start_z;
            S.r 	= 1;
            S.pn 	= (p->p == NULL)? -1 : ii + 2;
            seg_nt.listNeuron.append(S);
            seg_nt.hashNeuron.insert(S.n, result.listNeuron.size()-1);
        }
        unsigned char* total1dData_mask = 0;
        total1dData_mask = new unsigned char [mysz[0]*mysz[1]*mysz[2]];
        memset(total1dData_mask,0,mysz[0]*mysz[1]*mysz[2]*sizeof(unsigned char));
        double margin;
        margin = (seg->at(0)->level == 20)?10:5;
        ComputemaskImage(seg_nt, total1dData_mask, mysz[0], mysz[1], mysz[2],margin);
        for(V3DLONG j=0;j<mysz[0]*mysz[1]*mysz[2];++j)
            total1dData[j] = (total1dData_mask[j] ==0)?0:total1dData[j];

        unsigned char ****p4d = 0;
        if (!new4dpointer(p4d, mysz[0], mysz[1], mysz[2], mysz[3], total1dData))
        {
            fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
            if(p4d) {delete []p4d; p4d = 0;}
            return result;
        }

        LocationSimple p0;
        p0.x = seg->back()->x - start_x;
        p0.y = seg->back()->y - start_y;
        p0.z = seg->back()->z - start_z;

        vector<LocationSimple> pp;
        LocationSimple pEnd;
        pEnd.x = seg->at(0)->x - start_x;
        pEnd.y = seg->at(0)->y - start_y;
        pEnd.z = seg->at(0)->z - start_z;
        pp.push_back(pEnd);

        NeuronTree nt_gd = v3dneuron_GD_tracing(p4d, mysz,
                                  p0, pp,
                                  trace_para, weight_xy_z);
        for(int j=0; j<seg->size(); j++)
        {
            seg->at(j)->x -= start_x;
            seg->at(j)->y -= start_y;
            seg->at(j)->z -= start_z;
        }
        double dist = distTwoSegs(nt_gd, seg);
        //check angle
//        QList<NeuronSWC> nt_seg = nt_gd.listNeuron;
//        int angle_size = 2;
//        bool flag = false;
//        if(nt_seg.size() > 2*angle_size)
//        {
//            for(int j = angle_size; j < nt_seg.size()-angle_size; j++)
//            {
//                double angle_j = angle(nt_seg[j], nt_seg[j-angle_size], nt_seg[j+angle_size]);
//                if(angle_j>=179 &&
//                   ((abs(nt_seg[j-angle_size].x-nt_seg[j+angle_size].x)<=2 && abs(nt_seg[j-angle_size].y-nt_seg[j+angle_size].y)<=2)||
//                   (abs(nt_seg[j-angle_size].x-nt_seg[j+angle_size].x)<=2 && abs(nt_seg[j-angle_size].z-nt_seg[j+angle_size].z)<=2)||
//                   (abs(nt_seg[j-angle_size].y-nt_seg[j+angle_size].y)<=2 && abs(nt_seg[j-angle_size].z-nt_seg[j+angle_size].z)<=2)))
//                {
//                    flag = true;
//                    break;
//                }
//            }
//        }
//        if(flag)
//            dist = DBL_MAX;
//        else
//            dist = distTwoSegs(nt_gd, seg);

        V3DLONG nt_length = result.listNeuron.size();
        V3DLONG index;
        if(nt_length>0)
            index = result.listNeuron.at(nt_length-1).n;
        else
            index = 0;

//        if(dist<5 || (!flag && seg->at(0)->level == 20))
        if(dist<5 || (seg->at(0)->level == 20))
        {
            for (int d=0;d<nt_gd.listNeuron.size();d++)
            {
                NeuronSWC curr = nt_gd.listNeuron.at(d);
                NeuronSWC S;
                S.n 	= curr.n + index;
                S.type 	= seg->at(0)->type;
                S.x 	= curr.x + start_x;
                S.y 	= curr.y + start_y;
                S.z 	= curr.z + start_z;
                S.r 	= curr.r;
                S.level = 20;
                S.pn 	= (curr.pn == -1)?  curr.pn : curr.pn + index;
                result.listNeuron.append(S);
                result.hashNeuron.insert(S.n, result.listNeuron.size()-1);
            }
        }else
        {
//            writeSWC_file("/local3/refinement/test_bug/test.swc",seg_nt);
//            simple_saveimage_wrapper(callback, "/local3/refinement/test_bug/image.v3draw",(unsigned char *)total1dData, mysz, 1);
//            simple_saveimage_wrapper(callback, "/local3/refinement/test_bug/mask.v3draw",(unsigned char *)total1dData, mysz, 1);
            for(int d=0; d<seg->size(); d++)
            {
                Point* p = seg->at(d);
                NeuronSWC S;
                S.n 	= d + index + 1;
                S.type 	= p->type;
                S.x 	= p->x + start_x;
                S.y 	= p->y + start_y;
                S.z 	= p->z + start_z;
                S.r 	= p->r;
                S.level = 180;
                S.pn 	= (p->p == NULL)? -1 : d + index + 2;
                result.listNeuron.append(S);
                result.hashNeuron.insert(S.n, result.listNeuron.size()-1);
            }

        }
        pp.clear();
        if(total1dData) {delete [] total1dData; total1dData=0;}
        if(total1dData_mask) {delete [] total1dData_mask; total1dData_mask=0;}

    }

    return result;
}

NeuronTree breakSWC(NeuronTree input, double step, QList<ImageMarker>& break_points)
{
    NeuronTree result;
    V3DLONG siz = input.listNeuron.size();
    Tree tree;
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = input.listNeuron[i];
        Point* pt = new Point;
        pt->x = s.x;
        pt->y = s.y;
        pt->z = s.z;
        pt->r = s.r;
        pt ->type = s.type;
        pt->seg_id = s.seg_id;
        pt->level = s.level;
        pt->fea_val = s.fea_val;
        pt->p = NULL;
        pt->childNum = 0;
        tree.push_back(pt);
    }
    for (V3DLONG i=0;i<siz;i++)
    {
        if (input.listNeuron[i].pn<0) continue;
        V3DLONG pid = input.hashNeuron.value(input.listNeuron[i].pn);
        tree[i]->p = tree[pid];
        tree[pid]->childNum++;
    }
//	printf("tree constructed.\n");
    vector<Segment*> seg_list;
    for (V3DLONG i=0;i<siz;i++)
    {
        if (tree[i]->childNum!=1)//tip or branch point
        {
            Segment* seg = new Segment;
            Point* cur = tree[i];
            do
            {
                seg->push_back(cur);
                cur = cur->p;
            }
            while(cur && cur->childNum==1);
            seg_list.push_back(seg);
        }
    }
//	printf("segment list constructed.\n");
    for (V3DLONG i=0;i<seg_list.size();i++)
    {
        break_path(seg_list[i], step, break_points);
    }

//	printf("resample done.\n");
    tree.clear();
    map<Point*, V3DLONG> index_map;
    for (V3DLONG i=0;i<seg_list.size();i++)
        for (V3DLONG j=0;j<seg_list[i]->size();j++)
        {
            tree.push_back(seg_list[i]->at(j));
            index_map.insert(pair<Point*, V3DLONG>(seg_list[i]->at(j), tree.size()-1));
        }
    for (V3DLONG i=0;i<tree.size();i++)
    {
        NeuronSWC S;
        Point* p = tree[i];
        S.n = i+1;
        if (p->p==NULL) S.pn = -1;
        else
            S.pn = index_map[p->p]+1;
        if (p->p==p) printf("There is loop in the tree!\n");
        S.x = p->x;
        S.y = p->y;
        S.z = p->z;
        S.r = p->r;
        S.type = p->type;
        S.seg_id = p->seg_id;
        S.level = p->level;
        S.fea_val = p->fea_val;
        result.listNeuron.push_back(S);
    }
    for (V3DLONG i=0;i<tree.size();i++)
    {
        if (tree[i]) {delete tree[i]; tree[i]=NULL;}
    }
    for (V3DLONG j=0;j<seg_list.size();j++)
        if (seg_list[j]) {delete seg_list[j]; seg_list[j] = NULL;}
    for (V3DLONG i=0;i<result.listNeuron.size();i++)
        result.hashNeuron.insert(result.listNeuron[i].n, i);
    return result;
}

void break_path(Segment * seg, double step, QList<ImageMarker>& break_points)
{
    double path_length = 0;
    V3DLONG n = seg->size();
    for(int i=1; i<n;i++)
    {
        path_length += DISTP(seg->at(i),seg->at(i-1));
        if(path_length>step)
        {
            Point* pt = new Point;
            pt->x = seg->at(i)->x;
            pt->y = seg->at(i)->y;
            pt->z = seg->at(i)->z;
            pt->r = seg->at(i)->r;
            pt->type = seg->at(i)->type;
            pt->level = seg->at(i)->level;
            pt->p = NULL;
            seg->at(i-1)->p = pt;
            seg->push_back(pt);
            seg->at(i)->p == NULL;
            path_length=0;

            ImageMarker tmp;
            tmp.x = pt->x;
            tmp.y = pt->y;
            tmp.z = pt->z;
            tmp.color.r = 0;
            tmp.color.g = 255;
            tmp.color.b = 0;
            break_points.push_back(tmp);
        }
    }
};

double distTwoSegs(NeuronTree nt, Segment * seg)
{
    double dis1=0, dis2=0;
    for(int i=0;i<nt.listNeuron.size();i++)
    {
        Point* pt = new Point;
        pt->x = nt.listNeuron.at(i).x;
        pt->y = nt.listNeuron.at(i).y;
        pt->z = nt.listNeuron.at(i).z;
        double tmp = DBL_MAX;
        for(int j=0; j<seg->size();j++)
        {
            if(DISTP(pt,seg->at(j))<tmp)
                tmp = DISTP(pt,seg->at(j));
        }
        dis1 += tmp;
    }
    for(int i=0;i<seg->size();i++)
    {
        double tmp = DBL_MAX;
        for(int j=0;j<nt.listNeuron.size();j++)
        {
            Point* pt = new Point;
            pt->x = nt.listNeuron.at(j).x;
            pt->y = nt.listNeuron.at(j).y;
            pt->z = nt.listNeuron.at(j).z;
            if(DISTP(pt,seg->at(i))<tmp)
                tmp = DISTP(pt,seg->at(i));
        }
        dis2 += tmp;
    }
    double res = dis1/nt.listNeuron.size() + dis2/seg->size();
    return res;
}


NeuronTree smoothSWCTerafly(V3DPluginCallback2 &callback,QString fname_img, NeuronTree nt, QList<ImageMarker>& break_points)
{
    NeuronTree result;

    QVector<QVector<V3DLONG> > childs;

    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );

    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    double weight_xy_z=1.0;
    CurveTracePara trace_para;
    trace_para.channo = 0;
    trace_para.sp_graph_background = 0;
    trace_para.b_postMergeClosebyBranches = false;
    trace_para.b_3dcurve_width_from_xyonly = true;
    trace_para.b_pruneArtifactBranches = false;
    trace_para.sp_num_end_nodes = 2;
    trace_para.b_deformcurve = false;
    trace_para.sp_graph_resolution_step = 2;
    trace_para.b_estRadii = false;

    QList<NeuronSWC> list = nt.listNeuron;
    V3DLONG start_x,start_y,start_z,end_x,end_y,end_z;
    for(int i=0; i<break_points.size();i++)
    {
        int index = -1;
        for (int j=0;j<neuronNum;j++)
        {
            if(NTDIS(break_points.at(i),list.at(j))<1 && list.at(j).level != 180)
            {
                index = j;
                start_x = list.at(index).x;
                end_x = list.at(index).x;
                start_y = list.at(index).y;
                end_y = list.at(index).y;
                start_z = list.at(index).z;
                end_z = list.at(index).z;
                break;
            }
        }
        if(index != -1)
        {
            int cnt=0;
            int index_begin = index;
            int index_end = index;
            int index_last = index;
            while(cnt<5 && childs[index_begin].size()==1)
            {
                index_last = index_begin;
                index_begin=getParent(index_begin,nt);
                cnt++;
            }
            if(cnt==0) break;
            index_begin=childs[index_last].at(0);

            cnt = 0;
            while(cnt<5 && childs[index_end].size()==1)
            {
                index_end=childs[index_end].at(0);
                cnt++;
            }
            if(cnt==0) break;

            for(V3DLONG j=index_begin; j<index_end;j++)
            {
                if(start_x>list.at(j).x)  start_x = list.at(j).x;
                if(end_x<list.at(j).x)  end_x = list.at(j).x;
                if(start_y>list.at(j).y)  start_y = list.at(j).y;
                if(end_y<list.at(j).y)  end_y = list.at(j).y;
                if(start_z>list.at(j).z)  start_z = list.at(j).z;
                if(end_z<list.at(j).z)  end_z = list.at(j).z;
            }

            start_x -= 10;
            end_x   += 10;
            start_y -= 10;
            end_y   += 10;
            start_z -= 10;
            end_z   += 10;

            unsigned char * total1dData = 0;
            total1dData = callback.getSubVolumeTeraFly(fname_img.toStdString(),start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
            V3DLONG mysz[4];
            mysz[0] = end_x-start_x+1;
            mysz[1] = end_y-start_y+1;
            mysz[2] = end_z-start_z+1;
            mysz[3] = 1;

            unsigned char ****p4d = 0;
            if (!new4dpointer(p4d, mysz[0], mysz[1], mysz[2], mysz[3], total1dData))
            {
                fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
                if(p4d) {delete []p4d; p4d = 0;}
                return result;
            }

            LocationSimple p0;
            p0.x = list.at(index_begin).x - start_x;
            p0.y = list.at(index_begin).y - start_y;
            p0.z = list.at(index_begin).z - start_z;

            vector<LocationSimple> pp;
            LocationSimple pEnd;
            pEnd.x = list.at(index_end).x - start_x;
            pEnd.y = list.at(index_end).y - start_y;
            pEnd.z = list.at(index_end).z - start_z;
            pp.push_back(pEnd);

            NeuronTree nt_gd = v3dneuron_GD_tracing(p4d, mysz,
                                      p0, pp,
                                      trace_para, weight_xy_z);

            if(nt_gd.listNeuron.size()>0)
            {
                for(V3DLONG j=index_begin; j<index_end;j++)
                {
                    if(j<index_end-1)
                    {
                        nt.listNeuron[j+1].type = -1;
                    }
                }
            }

            V3DLONG nt_length = result.listNeuron.size();
            V3DLONG addon_index;
            if(nt_length>0)
                addon_index = result.listNeuron.at(nt_length-1).n;
            else
                addon_index = 0;


            for (int d=0;d<nt_gd.listNeuron.size();d++)
            {
                NeuronSWC curr = nt_gd.listNeuron.at(d);
                NeuronSWC S;
                S.n 	= curr.n + addon_index;
                S.type 	= list.at(index).type;
                S.x 	= curr.x + start_x;
                S.y 	= curr.y + start_y;
                S.z 	= curr.z + start_z;
                S.r 	= curr.r;
                S.level = list.at(index).level;
                S.pn 	= (curr.pn == -1)?  curr.pn : curr.pn + addon_index;
                result.listNeuron.append(S);
                result.hashNeuron.insert(S.n, result.listNeuron.size()-1);
            }

            pp.clear();
            if(total1dData) {delete [] total1dData; total1dData=0;}
        }
    }

    for(int i=nt.listNeuron.size()-1; i>=0; i--)
    {
        if(nt.listNeuron.at(i).type == -1)
            nt.listNeuron.erase(nt.listNeuron.begin()+i);
    }

    for (int d=0;d<result.listNeuron.size();d++)
    {
        NeuronSWC curr = result.listNeuron.at(d);
        NeuronSWC S;
        S.n 	= curr.n + neuronNum;
        S.type 	= curr.type;
        S.x 	= curr.x;
        S.y 	= curr.y;
        S.z 	= curr.z;
        S.r 	= curr.r;
        S.level = curr.level;
        S.pn 	= (curr.pn == -1)?  curr.pn : curr.pn + neuronNum;
        nt.listNeuron.append(S);
        nt.hashNeuron.insert(S.n, nt.listNeuron.size()-1);
    }
    return nt;
}

NeuronTree initialSWCTerafly(V3DPluginCallback2 &callback,QString fname_img, NeuronTree nt, double ds_rate)
{
    NeuronTree result;
    V3DLONG siz = nt.listNeuron.size();
    Tree tree;
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron[i];
        Point* pt = new Point;
        pt->x = s.x/ds_rate;
        pt->y = s.y/ds_rate;
        pt->z = s.z/ds_rate;
        pt->r = s.r;
        pt ->type = s.type;
        pt->seg_id = s.seg_id;
        pt->level = s.level;
        pt->fea_val = s.fea_val;
        pt->p = NULL;
        pt->childNum = 0;
        tree.push_back(pt);
    }
    for (V3DLONG i=0;i<siz;i++)
    {
        if (nt.listNeuron[i].pn<0) continue;
        V3DLONG pid = nt.hashNeuron.value(nt.listNeuron[i].pn);
        tree[i]->p = tree[pid];
        tree[pid]->childNum++;
    }
//	printf("tree constructed.\n");
    vector<Segment*> seg_list;
    for (V3DLONG i=0;i<siz;i++)
    {
        if (tree[i]->childNum!=1)//tip or branch point
        {
            Segment* seg = new Segment;
            Point* cur = tree[i];
            do
            {
                seg->push_back(cur);
                cur = cur->p;
            }
            while(cur && cur->childNum==1);
            seg_list.push_back(seg);
        }
    }

    double weight_xy_z=1.0;
    CurveTracePara trace_para;
    trace_para.channo = 0;
    trace_para.sp_graph_background = 0;
    trace_para.b_postMergeClosebyBranches = false;
    trace_para.b_3dcurve_width_from_xyonly = true;
    trace_para.b_pruneArtifactBranches = false;
    trace_para.sp_num_end_nodes = 2;
    trace_para.b_deformcurve = false;
    trace_para.sp_graph_resolution_step = 1;
    trace_para.b_estRadii = false;


    V3DLONG start_x,start_y,start_z,end_x,end_y,end_z;
    for(V3DLONG i=0; i<seg_list.size();i++)
    {
        Segment* seg = seg_list[i];
        V3DLONG seg_length=0;
        if(seg->size()>1)
        {
            for(int j=0; j<seg->size()-1;j++)
            {
                seg_length += DISTP(seg->at(j),seg->at(j+1));
            }
        }

        if(seg_length >40)
        {
            start_x = seg->at(0)->x;
            end_x = seg->at(0)->x;
            start_y = seg->at(0)->y;
            end_y = seg->at(0)->y;
            start_z = seg->at(0)->z;
            end_z = seg->at(0)->z;
            for(V3DLONG j=1; j<seg->size();j++)
            {
                if(start_x>seg->at(j)->x)  start_x = seg->at(j)->x;
                if(end_x<seg->at(j)->x)  end_x = seg->at(j)->x;
                if(start_y>seg->at(j)->y)  start_y = seg->at(j)->y;
                if(end_y<seg->at(j)->y)  end_y = seg->at(j)->y;
                if(start_z>seg->at(j)->z)  start_z = seg->at(j)->z;
                if(end_z<seg->at(j)->z)  end_z = seg->at(j)->z;
            }

            start_x -= 10;
            end_x   +=10;
            start_y -= 10;
            end_y   +=10;
            start_z -= 10;
            end_z   +=10;

            unsigned char * total1dData = 0;
            total1dData = callback.getSubVolumeTeraFly(fname_img.toStdString(),start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
            V3DLONG mysz[4];
            mysz[0] = end_x-start_x+1;
            mysz[1] = end_y-start_y+1;
            mysz[2] = end_z-start_z+1;
            mysz[3] = 1;

            unsigned char ****p4d = 0;
            if (!new4dpointer(p4d, mysz[0], mysz[1], mysz[2], mysz[3], total1dData))
            {
                fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
                if(p4d) {delete []p4d; p4d = 0;}
                return result;
            }

            LocationSimple p0;
            p0.x = seg->back()->x - start_x;
            p0.y = seg->back()->y - start_y;
            p0.z = seg->back()->z - start_z;

            vector<LocationSimple> pp;
            LocationSimple pEnd;
            pEnd.x = seg->at(0)->x - start_x;
            pEnd.y = seg->at(0)->y - start_y;
            pEnd.z = seg->at(0)->z - start_z;
            pp.push_back(pEnd);

            NeuronTree nt_gd = v3dneuron_GD_tracing(p4d, mysz,
                                                    p0, pp,
                                                    trace_para, weight_xy_z);
            for(int j=0; j<seg->size(); j++)
            {
                seg->at(j)->x -= start_x;
                seg->at(j)->y -= start_y;
                seg->at(j)->z -= start_z;
            }
            double dist = distTwoSegs(nt_gd, seg);

            for(int j=0; j<seg->size();j++)
            {
                seg->at(j)->x = (seg->at(j)->x+start_x)*ds_rate;
                seg->at(j)->y = (seg->at(j)->y+start_y)*ds_rate;
                seg->at(j)->z = (seg->at(j)->z+start_z)*ds_rate;

            }

            if(dist <5)
            {
                for(int j=0; j<seg->size();j++)
                {
                    seg->at(j)->level = 20;

                }
            }
        }else
        {
            for(int j=0; j<seg->size();j++)
            {
                seg->at(j)->x *= ds_rate;
                seg->at(j)->y *= ds_rate;
                seg->at(j)->z *= ds_rate;
            }
        }
    }

    tree.clear();
    map<Point*, V3DLONG> index_map;
    for (V3DLONG i=0;i<seg_list.size();i++)
        for (V3DLONG j=0;j<seg_list[i]->size();j++)
        {
            tree.push_back(seg_list[i]->at(j));
            index_map.insert(pair<Point*, V3DLONG>(seg_list[i]->at(j), tree.size()-1));
        }
    for (V3DLONG i=0;i<tree.size();i++)
    {
        NeuronSWC S;
        Point* p = tree[i];
        S.n = i+1;
        if (p->p==NULL) S.pn = -1;
        else
            S.pn = index_map[p->p]+1;
        if (p->p==p) printf("There is loop in the tree!\n");
        S.x = p->x;
        S.y = p->y;
        S.z = p->z;
        S.r = p->r;
        S.type = p->type;
        S.seg_id = p->seg_id;
        S.level = p->level;
        S.fea_val = p->fea_val;
        result.listNeuron.push_back(S);
    }
    for (V3DLONG i=0;i<tree.size();i++)
    {
        if (tree[i]) {delete tree[i]; tree[i]=NULL;}
    }
    for (V3DLONG j=0;j<seg_list.size();j++)
        if (seg_list[j]) {delete seg_list[j]; seg_list[j] = NULL;}
    for (V3DLONG i=0;i<result.listNeuron.size();i++)
        result.hashNeuron.insert(result.listNeuron[i].n, i);
    return result;
}

void resample_path(Segment * seg, double step)
{
    char c;
    Segment seg_r;
    double path_length = 0;
    Point* start = seg->at(0);
    Point* seg_par = seg->back()->p;
    V3DLONG iter_old = 0;
    seg_r.push_back(start);
    while (iter_old < seg->size() && start && start->p)
    {
        path_length += DISTP(start,start->p);
        if (path_length<=seg_r.size()*step)
        {
            start = start->p;
            iter_old++;
        }
        else//a new point should be created
        {
            path_length -= DISTP(start,start->p);
            Point* pt = new Point;
            double rate = (seg_r.size()*step-path_length)/(DISTP(start,start->p));
            pt->x = start->x + rate*(start->p->x-start->x);
            pt->y = start->y + rate*(start->p->y-start->y);
            pt->z = start->z + rate*(start->p->z-start->z);
            pt->r = start->r*(1-rate) + start->p->r*rate;//intepolate the radius
            pt->p = start->p;

            if (rate<0.5)
            {
                pt->type = start->type;
                pt->seg_id = start->seg_id;
                pt->level = start->level;
                pt->fea_val = start->fea_val;
            }
            else
            {
                pt->type = start->p->type;
                pt->seg_id = start->p->seg_id;
                pt->level = start->p->level;
                pt->fea_val = start->p->fea_val;

            }
            seg_r.back()->p = pt;
            seg_r.push_back(pt);
            path_length += DISTP(start,pt);
            start = pt;
        }
    }
    seg_r.back()->p = seg_par;
    for (V3DLONG i=0;i<seg->size();i++)
        if (!seg->at(i)) {delete seg->at(i); seg->at(i) = NULL;}
    *seg = seg_r;
};
