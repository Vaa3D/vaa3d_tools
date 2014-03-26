#ifndef __HP_H__
#define __HP_H__

#include <iostream>
#include <vector>
#include <string>

#include "fastmarching_tree.h"
#include "fastmarching_dt.h"
#include "hierarchy_prune.h"
#include "marker_radius.h"
using namespace std;


#include "my_surf_objs.h"

bool swc_to_segments(vector<MyMarker*> & inmarkers, vector<vector<V_NeuronSWC_unit> > &segments)
{
	map<MyMarker *, int>  child_num;
	vector<MyMarker*> leaf_markers = getLeaf_markers(inmarkers, child_num);
	map<MyMarker *, int> node_index; for(int i = 0; i < inmarkers.size(); i++) node_index[inmarkers[i]] = i+1;

	set<MyMarker*> start_markers;
	for(int i = 0; i < leaf_markers.size(); i++) 
	{
		MyMarker * start_marker = leaf_markers[i];
		while(start_markers.find(start_marker) == start_markers.end())
		{
			start_markers.insert(start_marker);
			vector<V_NeuronSWC_unit> segment;
			MyMarker * p = start_marker;
			do
			{
				V_NeuronSWC_unit m2;
				m2.x = p->x;
				m2.y = p->y;
				m2.z = p->z;
				m2.r = p->radius;
				m2.nchild = child_num[p];
				m2.n = node_index[p];
				m2.parent = (p->parent) ? node_index[p->parent] : -1;
				segment.push_back(m2);
				p = p->parent;
			}while(p && child_num[p] == 1);
			if(true && p) // add branch or root node to segment
			{
				V_NeuronSWC_unit m2;
				m2.x = p->x;
				m2.y = p->y;
				m2.z = p->z;
				m2.r = p->radius;
				m2.nchild = child_num[p];
				m2.n = node_index[p];
				m2.parent = (p->parent) ? node_index[p->parent] : -1;
				segment.push_back(m2);
			}
			segments.push_back(segment);
			if(p) start_marker = p;
			else break;
		}
	}
	return true;
}

char* find_shortest_path_graphimg_FM(unsigned char ***img3d, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2,  //image
													 float zthickness, // z-thickness for weighted edge
													 //const V3DLONG box[6],  //bounding box
													 V3DLONG bx0, V3DLONG by0, V3DLONG bz0, V3DLONG bx1, V3DLONG by1, V3DLONG bz1, //bounding box (ROI)
													 float x0, float y0, float z0,       // start node
													 int n_end_nodes,                    // n_end_nodes == (0 for shortest path tree) (1 for shortest path) (n-1 for n pair path)
													 float x1[], float y1[], float z1[],    // all end nodes
													 vector< vector<V_NeuronSWC_unit> >& mmUnit, // change from Coord3D for shortest path tree
													 const ParaShortestPath & para)
{
	printf("===== Fast marching method based shortest path ====\n");
	V_NeuronSWC_list tracedNeuron;

	bool is_gsdt = false;
	int bkg_thresh = para.imgTH; //30;
	double length_thresh = 2.0;
	int cnn_type = 2; // default connection type 2
	V3DLONG channel = 0;

	cout<<"bkg_thresh = "<<bkg_thresh<<endl;
	cout<<"length_thresh = "<<length_thresh<<endl;
	cout<<"is_gsdt = "<<is_gsdt<<endl;
	cout<<"cnn_type = "<<cnn_type<<endl;
	cout<<"channel = "<<channel<<endl;

	unsigned char * indata1d = img3d[0][0];
	V3DLONG  in_sz[4] = {sz0, sz1, sz2, 1};

	vector<MyMarker> inmarkers;
	vector<MyMarker *> outtree;

	inmarkers.push_back(MyMarker(x0, y0, z0));
	for(int i = 0; i < n_end_nodes; i++)
	{
		inmarkers.push_back(MyMarker(x1[i], y1[i], z1[i]));
	}

	cout<<"======================================="<<endl;
	if(inmarkers.size() == 1)
	{
		cout<<"only one input marekr"<<endl;
		if(is_gsdt) 
		{
			float * phi = 0;
			cout<<"processing fastmarching distance transformation ..."<<endl;
			fastmarching_dt(indata1d, phi, in_sz[0], in_sz[1], in_sz[2], cnn_type, bkg_thresh);
			cout<<endl<<"constructing fastmarching tree ..."<<endl;
			fastmarching_tree(inmarkers[0], phi, outtree, in_sz[0], in_sz[1], in_sz[2], cnn_type, bkg_thresh);
			if(phi){delete [] phi; phi = 0;}
		}
		else 
		{
			fastmarching_tree(inmarkers[0], indata1d, outtree, in_sz[0], in_sz[1], in_sz[2], cnn_type, bkg_thresh);
		}
	}
	else
	{
		vector<MyMarker> target; target.insert(target.end(), inmarkers.begin()+1, inmarkers.end());
		if(is_gsdt)
		{
			float * phi = 0;
			cout<<"processing fastmarching distance transformation ..."<<endl;
			fastmarching_dt(indata1d, phi, in_sz[0], in_sz[1], in_sz[2], cnn_type, bkg_thresh);
			cout<<endl<<"constructing fastmarching tree ..."<<endl;
			fastmarching_tree(inmarkers[0], target, phi, outtree, in_sz[0], in_sz[1], in_sz[2], cnn_type);
			if(phi){delete [] phi; phi = 0;}

		}
		else fastmarching_tree(inmarkers[0], target, indata1d, outtree, in_sz[0], in_sz[1], in_sz[2], cnn_type);
	}
	cout<<"======================================="<<endl;

	if(0) // if pruning
	{
		vector<MyMarker*> & inswc = outtree;
		vector<MyMarker*> outswc;
		//hierarchy_prune(inswc, outswc, indata1d, in_sz[0], in_sz[1], in_sz[2], length_thresh);
		happ(inswc, outswc, indata1d, in_sz[0], in_sz[1], in_sz[2], bkg_thresh, length_thresh);
		swc_to_segments(outswc, mmUnit);
	}
	else swc_to_segments(outtree, mmUnit);
	//rearrange_and_remove_labeled_deletion_nodes_mmUnit(mmUnit);
	//int nSegsTrace = mergeback_mmunits_to_neuron_path(pp.size(), mmUnit, tracedNeuron);
	printf("===== Finish FM tree construction ====\n");
	return 0;

}
#endif
