#include "swc_utils.h"
#include <stack>
using namespace std;
// convert inmarkers to many connected neuron structure
void swc_decompose(vector<MyMarker*> & inmarkers, vector<vector<MyMarker*> > & out_segs)
{
	map<MyMarker*, vector<MyMarker*> > root_map; // map from root to its children
	for(int m = 0; m < inmarkers.size(); m++)
	{
		MyMarker * marker = inmarkers[m];
		if(marker->parent != 0 && marker->parent != marker) continue;
		root_map[marker] = vector<MyMarker*>();
	}
	for(int m = 0; m < inmarkers.size(); m++)
	{
		MyMarker * marker = inmarkers[m];
		MyMarker * p = marker;
		while(p->parent != 0 && p->parent != p) p = p->parent;
		root_map[p].push_back(marker);
	}
	map<MyMarker*, vector<MyMarker*> >::iterator it = root_map.begin();
	while(it != root_map.end()) 
	{
		out_segs.push_back(it->second);
		it++;
	}
}

// make sure only one connected area in the neuron tree
bool swc_to_segments(vector<MyMarker*> & inmarkers, vector<vector<MyMarker*> > & outsegs, vector<int> & seg_par)
{
	map<MyMarker *, int>  childs_num;
	getLeaf_markers(inmarkers, childs_num);

	map<MyMarker*, int> segid_map;
	int seg_id = 0;
	for(int i = 0; i < inmarkers.size(); i++)
	{
		MyMarker * marker = inmarkers[i];
		int child_num = childs_num[marker];
		if (child_num == 1) continue;
		//if (marker->parent==NULL | marker->parent==marker) continue;
		vector<MyMarker*> segment;
		segid_map[marker] = seg_id++;
		do
		{
			segment.push_back(marker);  // add the begin node
			marker = marker->parent;
		}
		while(marker && marker->parent != 0 && marker->parent != marker &&  childs_num[marker] == 1);
//		if (marker) segment.push_back(marker); // add the end node
		outsegs.push_back(segment);
	}

	seg_par = vector<int>(outsegs.size(), 0);
	for(int i = 0; i < outsegs.size(); i++)
	{
		vector<MyMarker*> & seg = outsegs[i];
		MyMarker * end_marker = *(seg.rbegin());
		end_marker = end_marker->parent;
		if(segid_map.find(end_marker) == segid_map.end()) 
			seg_par[i] = i;
		else seg_par[i] = segid_map[end_marker];
	}
	return true;
}

// convert to post_order tree
bool swc_to_segments(vector<MyMarker*> & inmarkers, vector<NeuronSegment*> &tree)
{
	map<MyMarker *, int>  childs_num;
	getLeaf_markers(inmarkers, childs_num);

	map<MyMarker*, NeuronSegment*> marker_seg_map;
	MyMarker * root_marker;

	for (int i=0;i<inmarkers.size();i++)
	{
		MyMarker * marker = inmarkers[i];
		if (!marker || !marker->parent || marker->parent==marker || childs_num[marker] != 1) //tip, branch or root points
		{
			if ( !marker->parent || marker->parent==marker)
				root_marker = marker;
			NeuronSegment * seg = new NeuronSegment;
			marker_seg_map[marker] = seg;
		}
	}
	for (map<MyMarker*, NeuronSegment*>::iterator it = marker_seg_map.begin(); it != marker_seg_map.end(); it++)
	{
		MyMarker * marker = (*it).first;
		MyMarker * p = marker;
		do
		{
			marker_seg_map[marker]->markers.push_back(p);
			p = p->parent;
		}
		while (p && marker_seg_map.find(p)==marker_seg_map.end());
		if (!p) continue;
		NeuronSegment * seg_par = marker_seg_map[p];
		seg_par->child_list.push_back(marker_seg_map[marker]);
	}

	stack<NeuronSegment*> seg_stack;
	map<NeuronSegment*, bool> map_visit;
	for (map<MyMarker*, NeuronSegment*>::iterator it = marker_seg_map.begin(); it != marker_seg_map.end(); it++)
		map_visit[(*it).second] = false;
	seg_stack.push(marker_seg_map[root_marker]);
	while (!seg_stack.empty())
	{
		NeuronSegment * cur = seg_stack.top();
		bool all_childs_visited = true;
		for (int i=0;i<cur->child_list.size();i++)
		{
			if (!map_visit[cur->child_list[i]]) 
			{
				seg_stack.push(cur->child_list[i]);
				all_childs_visited = false;
			}
		}
		if (all_childs_visited)
		{
			map_visit[cur] = true;
			tree.push_back(cur);
			seg_stack.pop();
		}
	}
	
	return true;
}

//get root list of neuron
vector<MyMarker*> getRoots(vector<MyMarker*> & inswc)
{
	vector<MyMarker*> roots;
	for (int i=0;i<inswc.size();i++)
		if (!inswc[i]->parent || inswc[i]->parent==inswc[i])
			roots.push_back(inswc[i]);
	return roots;
}

//reroot neuron to a new root
bool reroot(vector<MyMarker*> & sbjswc, MyMarker* new_root)
{
	vector<MyMarker*> roots = getRoots(sbjswc);
	if (roots.size()!=1)
	{
		cout<<"more than one root found"<<endl;
		return false;
	}
	MyMarker* old_root = roots[0];
	if (new_root==old_root) return true;
	vector<MyMarker*> path;
	MyMarker* p = new_root;
	do
	{
		path.push_back(p);
		p = p->parent;
	}
	while (p!=old_root);
	path.push_back(p);

	for (int i=1;i<path.size();i++)
		path[i]->parent = path[i-1];
	new_root->parent = NULL;

	return true;

}

//path distance from a node to root
double path_dist_root(vector<MyMarker*> & inswc, MyMarker* node)
{
	vector<MyMarker*> roots = getRoots(inswc);
	if (roots.size()!=1)
		return -1;
	MyMarker* root = roots[0];
	double result = 0.0;
	MyMarker * p = node;
	while (p && p!=root)
	{
		MyMarker * par = p->parent;
		result += dist(*p, *par);
		p = par;
	}
	while (p && p!=root);
	return result;
}

//path length of a marker
double seg_length(vector<MyMarker*> & seg)
{
	double result = 0;
	if (seg.size()<=0) return 0.0;
	for (int i=1;i<seg.size();i++)
		result += dist(*seg[i-1], *seg[i]);
	return result;
}

//get far ends - the 2 end points of tree diameter
bool get_far_ends(vector<MyMarker*> & inswc, vector<MyMarker*> & ends)
{
	int siz = inswc.size();
	map<MyMarker *, int>  childs_num;
	vector<MyMarker*> leafs = getLeaf_markers(inswc, childs_num);
	map<MyMarker*, vector<double> > longest_path;
	map<MyMarker*, MyMarker*> chd;
	map<MyMarker*, vector<MyMarker*> > far_ends;
	for (int i=0;i<inswc.size();i++)
	{
		longest_path[inswc[i]] = vector<double>(2,0);
		chd[inswc[i]] = NULL;
		far_ends[inswc[i]] = vector<MyMarker*>(2, (MyMarker*)0);
	}

	for (int i=0;i<leafs.size();i++)
	{
		MyMarker* cur = leafs[i];
		MyMarker* par;
		do
		{
			par = cur->parent;
			double distance = dist(*cur, *par) + longest_path[cur][0];
			if (distance > longest_path[par][0])
			{
				if (chd[par] != cur)
				{
					far_ends[par][1] = far_ends[par][0];
					longest_path[par][1] = longest_path[par][0];
				}
				far_ends[par][0] = leafs[i];
				chd[par] = cur;
				longest_path[par][0] = distance;
			}
			else if (distance > longest_path[par][1] && chd[par] != cur)
			{
				far_ends[par][1] = leafs[i];
				longest_path[par][1] = distance;
			}
			cur = par;
		}
		while (childs_num[cur]!=0 && par->parent!=NULL && par->parent!=par);
	}

	ends = vector<MyMarker*>(2, (MyMarker*)0);
	double diam = -1;
	for (map<MyMarker*, vector<double> >::iterator it=longest_path.begin(); it!=longest_path.end(); it++)
	{
		if ((*it).second[0] + (*it).second[1] > diam)
		{
			diam = (*it).second[0] + (*it).second[1];
			ends[0] = far_ends[(*it).first][0];
			ends[1] = far_ends[(*it).first][1];
			if (ends[1]==0) ends[1] = (*it).first;
		}
	}
	cout<<"diam="<<diam<<endl;
	return true;
}
