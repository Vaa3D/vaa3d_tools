#ifndef __NEURON_SEGMENT_H__
#define __NEURON_SEGMENT_H__

// the last marker is the root of current segment
// the last marker point to the first marker of parent segment
struct NeuronSegment
{
	NeuronSegment * parent;
	vector<NeuronSegment *> childs;
	vector<MyMarker *> markers;

	NeuronSegment(){parent =  0;}
	MyMarker * first(){return (markers.empty() ? 0 : (*(markers.begin())));}
	MyMarker * last(){return (markers.empty() ? 0 : (*(markers.rbegin())));}
	bool add_child(NeuronSegment * child)
	{
		if(child) childs.push_back(child);
		return true;
	}
};

// make sure that there is no redundent marker in inswc
vector<NeuronSegment*> swc_to_neuron_segment(vector<MyMarker*> & inswc)
{
	vector<NeuronSegment*> neuron_segs;

	map<MyMarker*, int> swc_map; for(int i = 0; i < inswc.size(); i++) swc_map[inswc[i]] = i;
	vector<int> child_num(inswc.size(),0);
	for(int i = 0; i < inswc.size(); i++) if(inswc[i]->parent) child_num[swc_map[inswc[i]->parent]]++;
	map<MyMarker*, NeuronSegment*> seg_map;
	for(int i = 0; i < inswc.size(); i++) 
	{
		if(child_num[i] != 0) continue;
		MyMarker * cur_marker = inswc[i];
		NeuronSegment * child_seg = 0;
		while(true)
		{
			NeuronSegment * cur_seg = new NeuronSegment; neuron_segs.push_back(cur_seg);
			if(child_seg) 
			{
				child_seg->parent = cur_seg;
				cur_seg->add_child(child_seg);
			}

			cur_seg->markers.push_back(cur_marker);
			seg_map[cur_marker] = cur_seg;
			MyMarker * p = cur_marker->parent;
			while(p && child_num[swc_map[p]] <= 1)
			{
				cur_seg->markers.push_back(p);
				p = p->parent;
			}
			if(p == 0) break;
			if(seg_map.find(p) != seg_map.end())
			{
				NeuronSegment * par_seg = seg_map[p];
				cur_seg->parent = par_seg;
				par_seg->add_child(cur_seg);
				break;
			}

			cur_marker = p;
			child_seg = cur_seg;
		}
	}
	return neuron_segs;
}

vector<NeuronSegment*> leaf_neuron_segments(vector<NeuronSegment*> & neuron_segs)
{
	vector<NeuronSegment*> leaf_segs;
	int n = neuron_segs.size();
	vector<int> seg_childs(n, 0);
	map<NeuronSegment*, int> seg_map; for(int i = 0; i < n; i++) seg_map[neuron_segs[i]] = i;
	for(int i = 0; i < n; i++) 
	{
		NeuronSegment * seg = neuron_segs[i];
		if(seg->parent) seg_childs[seg_map[seg->parent]]++;
	}

	// test every leaf segment
	for(int i = 0; i < n; i++)
	{
		if(seg_childs[i] == 0) leaf_segs.push_back(neuron_segs[i]);
	}
	return leaf_segs;
}

#endif
