/* refine_swc_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-12-6 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "refine_swc_plugin.h"
#include "neuron_format_converter.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.h"


using namespace std;
#define DISTP(a,b) sqrt(((a)->x-(b)->x)*((a)->x-(b)->x)+((a)->y-(b)->y)*((a)->y-(b)->y)+((a)->z-(b)->z)*((a)->z-(b)->z))
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
#define angle(a,b,c) (acos((((b).x-(a).x)*((c).x-(a).x)+((b).y-(a).y)*((c).y-(a).y)+((b).z-(a).z)*((c).z-(a).z))/(NTDIS(a,b)*NTDIS(a,c)))*180.0/3.14159265359)

Q_EXPORT_PLUGIN2(refine_swc, refine_swc);

struct Point;
struct Point
{
    double x,y,z,r;
    V3DLONG type;
    Point* p;
    V3DLONG childNum;
    V3DLONG level,seg_id;
    QList<float> fea_val;
};

typedef vector<Point*> Segment;
typedef vector<Point*> Tree;

void break_path(Segment * seg, double step);
double distTwoSegs(NeuronTree nt, Segment * seg);


QStringList refine_swc::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList refine_swc::funclist() const
{
	return QStringList()
		<<tr("refine")
		<<tr("func2")
		<<tr("help");
}

void refine_swc::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2018-12-6"));
	}
}

bool refine_swc::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("refine"))
	{
        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();
        vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();

        if(infiles.size()<2)
        {
            fprintf (stderr, "Need input terafly image path and swc. \n");
            return false;
        }

        if(outfiles.empty())
        {
            fprintf (stderr, "Need output file name. \n");
            return false;
        }

        int k=0;
        int max_length = (paras.size() >= k+1) ? atof(paras[k]) : 200;  k++;

        NeuronTree nt = readSWC_file(QString(infiles[1]));
        V_NeuronSWC_list nt_decomposed = NeuronTree__2__V_NeuronSWC_list(nt);
        V_NeuronSWC nt_joined = join_V_NeuronSWC_vec(nt_decomposed.seg);
        NeuronTree nt2 = V_NeuronSWC__2__NeuronTree(nt_joined);


        NeuronTree nt2_broken = breakSWC(nt2,max_length);
        V_NeuronSWC_list nt2_decomposed = NeuronTree__2__V_NeuronSWC_list(nt2_broken);
        NeuronTree nt2_new = V_NeuronSWC_list__2__NeuronTree(nt2_decomposed);
        NeuronTree nt2_refined = refineSWCTerafly(callback,infiles[0],nt2_new);
        writeESWC_file(QString(outfiles[0]),nt2_refined);

	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

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
    trace_para.sp_graph_resolution_step=2;
    trace_para.b_3dcurve_width_from_xyonly = true;
    trace_para.b_pruneArtifactBranches = false;
    trace_para.sp_num_end_nodes = 2;
    trace_para.b_deformcurve = false;
    trace_para.sp_graph_resolution_step = 2;
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
        double dist;
        //check angle
        QList<NeuronSWC> nt_seg = nt_gd.listNeuron;
        int angle_size = 2;
        bool flag = false;
        if(nt_seg.size() > 2*angle_size)
        {
            for(int j = angle_size; j < nt_seg.size()-angle_size; j++)
            {
                double angle_j = angle(nt_seg[j], nt_seg[j-angle_size], nt_seg[j+angle_size]);
                if(angle_j>=179 &&
                   ((abs(nt_seg[j-angle_size].x-nt_seg[j+angle_size].x)<=2 && abs(nt_seg[j-angle_size].y-nt_seg[j+angle_size].y)<=2)||
                   (abs(nt_seg[j-angle_size].x-nt_seg[j+angle_size].x)<=2 && abs(nt_seg[j-angle_size].z-nt_seg[j+angle_size].z)<=2)||
                   (abs(nt_seg[j-angle_size].y-nt_seg[j+angle_size].y)<=2 && abs(nt_seg[j-angle_size].z-nt_seg[j+angle_size].z)<=2)))
                {
                    flag = true;
                    break;
                }
            }
        }
        if(flag)
            dist = DBL_MAX;
        else
            dist = distTwoSegs(nt_gd, seg);

        V3DLONG nt_length = result.listNeuron.size();
        V3DLONG index;
        if(nt_length>0)
            index = result.listNeuron.at(nt_length-1).n;
        else
            index = 0;

        if(dist<5)
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
                S.pn 	= (curr.pn == -1)?  curr.pn : curr.pn + index;
                result.listNeuron.append(S);
                result.hashNeuron.insert(S.n, result.listNeuron.size()-1);
            }
        }else
        {
            for(int d=0; d<seg->size(); d++)
            {
                Point* p = seg->at(d);
                NeuronSWC S;
                S.n 	= d + index + 1;
                S.type 	= 6;
                S.x 	= p->x + start_x;
                S.y 	= p->y + start_y;
                S.z 	= p->z + start_z;
                S.r 	= p->r;
                S.pn 	= (p->p == NULL)? -1 : d + index + 2;
                result.listNeuron.append(S);
                result.hashNeuron.insert(S.n, result.listNeuron.size()-1);
            }

        }
        pp.clear();
        if(total1dData) {delete [] total1dData; total1dData=0;}
    }

    return result;
}

NeuronTree breakSWC(NeuronTree input, double step)
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
        break_path(seg_list[i], step);
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

void break_path(Segment * seg, double step)
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
            pt->p = NULL;
            seg->at(i-1)->p = pt;
            seg->push_back(pt);
            seg->at(i)->p == NULL;
            path_length=0;
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
