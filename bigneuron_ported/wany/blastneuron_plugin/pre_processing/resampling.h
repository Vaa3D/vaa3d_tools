//resample neuronTree subject to a step length
//2012-02-29 by Yinan Wan
//2012-03-05 Yinan Wan: interpolate radius
#ifndef __RESAMPLING_H__
#define __RESAMPLING_H__
#include "basic_surf_objs.h"
#include <vector>
using namespace std;

#define DISTP(a,b) sqrt(((a)->x-(b)->x)*((a)->x-(b)->x)+((a)->y-(b)->y)*((a)->y-(b)->y)+((a)->z-(b)->z)*((a)->z-(b)->z))

struct Point;
struct Point
{
	double x,y,z,r;
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
		S.type = 2;
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
