#include <set>
#include <map>
#include <iostream>

#include "stackutil.h"
#include "my_surf_objs.h"
#include "neuron_segment.h"

using namespace std;

int main(int argc, char ** argv)
{
	if(argc < 2)
	{
		cerr<<"Usage : "<<argv[0]<<" <inswc_file> [<inimg_file>]"<<endl;
		return 0;
	}
	unsigned char * inimg1d = 0; 
	V3DLONG * in_sz = 0; int datatype;
	long sz0, sz01;
	if(argc == 3) 
	{
		loadImage(argv[2], inimg1d, in_sz, datatype);
		sz0 = in_sz[0]; 
		sz01 = in_sz[0] * in_sz[1];
	}

	vector<MyMarker*> inmarkers = readSWC_file(argv[1]);
	
	map<MyMarker, set<NeuronSegment*> > overlap_map;
	for(int m = 0; m < inmarkers.size(); m++) 
	{
		MyMarker * marker = inmarkers[m];
		int x = marker->x + 0.5;
		int y = marker->y + 0.5;
		int z = marker->z + 0.5;
		overlap_map[MyMarker(x,y,z)] = set<NeuronSegment*>();
	}
	vector<NeuronSegment*> neuron_segs = swc_to_neuron_segment(inmarkers);

	for(int s = 0; s < neuron_segs.size(); s++)
	{
		NeuronSegment * seg = neuron_segs[s];
		for(int m = 0; m < seg->markers.size(); m++)
		{
			MyMarker * marker = seg->markers[m];
			int x = marker->x + 0.5;
			int y = marker->y + 0.5;
			int z = marker->z + 0.5;
			double r = marker->radius + 0.5;
			for(int k = -r; k <= r; k++)
			{
				int zz = z + k;
				for(int j = -r; j <= r; j++)
				{
					int yy = y + j;
					for(int i = -r; i <= r; i++)
					{
						int xx = x + i;
						MyMarker tmp_marker(xx,yy,zz);
						if(overlap_map.find(tmp_marker) != overlap_map.end())
						{
							overlap_map[tmp_marker].insert(seg);
						}
					}
				}
			}
		}
	}

	for(int i = 0; i < inmarkers.size(); i++) inmarkers[i]->type = 20;
	vector<NeuronSegment*> leaf_segs = leaf_neuron_segments(neuron_segs);
	for(int s = 0; s < leaf_segs.size(); s++)
	{
		NeuronSegment * seg = leaf_segs[s];
		MyMarker * first_marker = seg->first();
		MyMarker * last_marker = seg->last();
		double sum_score = 0.0;
		double mask_score = 0.0;
		for(vector<MyMarker*>::iterator it = seg->markers.begin(); it != seg->markers.end(); it++)
		{
			MyMarker * marker = *it;
			int x = marker->x + 0.5;
			int y = marker->y + 0.5;
			int z = marker->z + 0.5;
			MyMarker tmp(x,y,z);

			double score = (inimg1d) ? inimg1d[tmp.ind(sz0, sz01)]/255.0 : 1.0;
			if(overlap_map[tmp].size() > 1) mask_score += score;
			sum_score += score;
		}
		for(vector<MyMarker*>::iterator it = seg->markers.begin(); it != seg->markers.end(); it++)
		{
			double ratio = (sum_score > 0.0) ? mask_score/sum_score : 1.0;
			(*it)->type = ratio * 254.0 + 20.5;
		}
	}
	string outswc_file = string(argv[1]) + "_out.swc";
	saveSWC_file(outswc_file, inmarkers);
	return 0;
}
