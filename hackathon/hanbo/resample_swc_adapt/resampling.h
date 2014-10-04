//resample neuronTree subject to a step length
//2012-02-29 by Yinan Wan
//2012-03-05 Yinan Wan: interpolate radius
//2014-10-1 Hanbo Chen: adaptive threshold
#ifndef __RESAMPLING_H__
#define __RESAMPLING_H__
#include "basic_surf_objs.h"
#include <vector>
using namespace std;

#define DISTP(a,b) sqrt(((a)->x-(b)->x)*((a)->x-(b)->x)+((a)->y-(b)->y)*((a)->y-(b)->y)+((a)->z-(b)->z)*((a)->z-(b)->z))
#define RRATE(a,b) ((((a)->r)<=((b)->r)?((a)->r):((b)->r))/(((a)->r)>=((b)->r)?((a)->r):((b)->r)))
#define DOTP(a,b,c) (((b)->x-(a)->x)*((c)->x-(b)->x)+((b)->y-(a)->y)*((c)->y-(b)->y)+((b)->z-(a)->z)*((c)->z-(b)->z))

struct Point;
struct Point
{
	double x,y,z,r;
    V3DLONG type;
	Point* p;
	V3DLONG childNum;
};
typedef vector<Point*> Segment;
typedef vector<Point*> Tree;

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
            if (rate<0.5) pt->type = start->type;
            else pt->type = start->p->type;
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
}

NeuronTree resample(NeuronTree input, double step)
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
        if (tree[i]->childNum!=1 || tree[i]->p == NULL)//tip or branch point or root
        {
            Segment* seg = new Segment;
            Point* cur = tree[i];
            do
            {
                seg->push_back(cur);
                cur = cur->p;
            }
            while(cur && cur->childNum==1 && cur->p != NULL);
            seg_list.push_back(seg);
        }
    }
//	printf("segment list constructed.\n");
	for (V3DLONG i=0;i<seg_list.size();i++)
	{
        resample_path(seg_list[i], step);
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


//Hanbo Chen on 141001
//angleT: threshold for bending angle (0~1), keep merging when endPointDistance/pathLength>angleT
//radiusT: radius threshold (0~1), keep merging if min(r_current,r_previous)/max(r_current,r_previous)>radiusT
void resample_path_adaptive(Segment * seg, double angleT, double radiusT)
{
	if(seg->size()<2){
		//do nothing if there is only one point in the segments
	}
	else
    {
		Segment seg_r;
        double length1 = 0, length2=0, cosAng = 0, end_dis = 0, radius_rate1=1, radius_rate2=1;
		Point* end = seg->at(0);
		Point* start = end->p;
		Point* seg_par = seg->back()->p;
		V3DLONG iter_old = 1;
        seg_r.push_back(end);
		while (iter_old < seg->size() && start && start->p)
		{
            length1 = DISTP(start,start->p);
            length2 = DISTP(end,start);
            cosAng=DOTP(end,start,start->p)/(length1*length2);
            //end_dis = DISTP(start->p,end);
			radius_rate1 = RRATE(start,start->p);
            radius_rate2 = RRATE(start,end);
            if (cosAng>=angleT && radius_rate1>=radiusT && radius_rate2>=radiusT)
			{
				//move on
                //end=start;
				start = start->p;
				iter_old++;
			}
			else 
			{
				//push current point to the segment
				seg_r.back()->p=start;
				seg_r.push_back(start);
				//move on
				end=start;
				start=start->p;
				iter_old++;
            }
		}
		seg_r.back()->p = seg_par;

		for (V3DLONG i=0;i<seg->size();i++)
			if (!seg->at(i)) {delete seg->at(i); seg->at(i) = NULL;}
		*seg = seg_r;
	}
}

NeuronTree resample_adaptive(NeuronTree input, double angleT, double radiusT)
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
		if (tree[i]->childNum!=1 || tree[i]->p == NULL)//tip or branch point or root
		{
			Segment* seg = new Segment;
			Point* cur = tree[i];
			do
			{
				seg->push_back(cur);
				cur = cur->p;
			}
			while(cur && cur->childNum==1 && cur->p != NULL);
			seg_list.push_back(seg);
		}
	}

    printf("cojoc: %d\n",seg_list.size());


//	printf("segment list constructed.\n");
	for (V3DLONG i=0;i<seg_list.size();i++)
	{
        resample_path_adaptive(seg_list[i],angleT,radiusT);
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

#endif
