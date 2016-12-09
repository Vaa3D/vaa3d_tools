#ifndef __MY_SURF_OBJS_CPP__
#define __MY_SURF_OBJS_CPP__
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <cmath>

#include "my_surf_objs.h"

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

using namespace std;

vector<MyMarker> readMarker_file(string marker_file)
{
	vector<MyMarker> markers; markers.reserve(1000);
	ifstream ifs(marker_file.c_str()); if(ifs.fail()){cout<<" unable to open marker file "<<marker_file<<endl; return markers;}
	set<MyMarker> marker_set;
	int count = 0;
	while(ifs.good())
	{
		if(ifs.peek() == '#' || ifs.eof()){ifs.ignore(1000,'\n'); continue;}
		MyMarker marker;
		ifs>>marker.x;ifs.ignore(10,',');ifs>>marker.y;ifs.ignore(10,',');ifs>>marker.z;ifs.ignore(10,',');ifs>>marker.radius;ifs.ignore(1000,'\n');
		
		marker.x -= MARKER_BASE;
		marker.y -= MARKER_BASE;
		marker.z -= MARKER_BASE;

		if(marker_set.find(marker) != marker_set.end())
		{
			cout<<"omit duplicated marker"<<markers.size()<<" : x = "<<marker.x<<" y = "<<marker.y<<" z = "<<marker.z<<" r = "<<marker.radius<<endl;
		}
		else 
		{
			markers.push_back(marker);
			marker_set.insert(marker);
			if(0) cout<<"marker"<<markers.size()<<" : x = "<<marker.x<<" y = "<<marker.y<<" z = "<<marker.z<<" r = "<<marker.radius<<endl;
		}
		count++;
	}
	cout<<count<<" markers loaded"<<endl;
	ifs.close();
	return markers;
}

bool readMarker_file(string marker_file, vector<MyMarker*> &markers)
{
	ifstream ifs(marker_file.c_str()); if(ifs.fail()){cout<<" unable to open marker file "<<marker_file<<endl; return false;}
	set<MyMarker> marker_set; // to check the duplicated marker
	while(ifs.good())
	{
		if(ifs.peek() == '#' || ifs.eof()){ifs.ignore(1000,'\n'); continue;}
		MyMarker *marker = new MyMarker;
		ifs>>marker->x;ifs.ignore(10,',');ifs>>marker->y;ifs.ignore(10,',');ifs>>marker->z;ifs.ignore(10,',');ifs>>marker->radius;ifs.ignore(1000,'\n');

		marker->x -= MARKER_BASE;
		marker->y -= MARKER_BASE;
		marker->z -= MARKER_BASE;

		if(marker_set.find(*marker) != marker_set.end())
		{
			cout<<"omit duplicated marker"<<markers.size()<<" : x = "<<marker->x<<" y = "<<marker->y<<" z = "<<marker->z<<" r = "<<marker->radius<<endl;
		}
		else
		{
			markers.push_back(marker);
			marker_set.insert(*marker);
			if(0) cout<<"marker"<<markers.size()<<" : x = "<<marker->x<<" y = "<<marker->y<<" z = "<<marker->z<<" r = "<<marker->radius<<endl;
		}
	}
	ifs.close();
	return true;
}

bool saveMarker_file(string marker_file, vector<MyMarker> & outmarkers)
{
	cout<<"save "<<outmarkers.size()<<" markers to file "<<marker_file<<endl;
	ofstream ofs(marker_file.c_str()); 
	
	if(ofs.fail())
	{
		cout<<"open marker file error"<<endl; 
		return false;
	}
	ofs<<"#x, y, z, radius"<<endl;
	for(int i = 0; i < outmarkers.size(); i++)
	{
		ofs<<outmarkers[i].x + MARKER_BASE<<","<<outmarkers[i].y + MARKER_BASE<<","<<outmarkers[i].z + MARKER_BASE<<","<<outmarkers[i].radius<<endl;
	}
	ofs.close();
	return true;
}

bool saveMarker_file(string marker_file, vector<MyMarker*> & outmarkers)
{
	cout<<"save "<<outmarkers.size()<<" markers to file "<<marker_file<<endl;
	ofstream ofs(marker_file.c_str()); 
	
	if(ofs.fail())
	{
		cout<<"open marker file error"<<endl; 
		return false;
	}
	ofs<<"#x, y, z, radius"<<endl;
	for(int i = 0; i < outmarkers.size(); i++)
	{
		ofs<<outmarkers[i]->x + MARKER_BASE<<","<<outmarkers[i]->y + MARKER_BASE<<","<<outmarkers[i]->z+MARKER_BASE<<","<<outmarkers[i]->radius<<endl;
	}
	ofs.close();
	return true;
}

vector<MyMarker*> readSWC_file(string swc_file)
{
	vector<MyMarker*> swc;

	ifstream ifs(swc_file.c_str());

	if(ifs.fail()) 
	{
		cout<<"open swc file : "<< swc_file <<" error"<<endl;
		return swc;
	}

	map<int, MyMarker*> marker_map;
	map<MyMarker*, int> parid_map;
	while(ifs.good())
	{
		if(ifs.peek() == '#'){ifs.ignore(1000,'\n'); continue;}
		MyMarker *  marker = new MyMarker;
		int my_id = -1 ; ifs >> my_id;
		if(my_id == -1) break;
		if(marker_map.find(my_id) != marker_map.end())
		{
			cerr<<"Duplicate Node. This is a graph file. Please read is as a graph."<<endl; return vector<MyMarker*>();
		}
		marker_map[my_id] = marker;

		ifs>> marker->type;
		ifs>> marker->x;
		ifs>> marker->y;
		ifs>> marker->z;
		ifs>> marker->radius;
		int par_id = -1; ifs >> par_id;

		parid_map[marker] = par_id;
		swc.push_back(marker);
	}
	ifs.close();
	vector<MyMarker*>::iterator it = swc.begin();
	while(it != swc.end())
	{
		MyMarker * marker = *it;
		marker->parent = marker_map[parid_map[marker]];
		it++;
	}
	return swc;
}

bool saveSWC_file(string swc_file, vector<MyMarker*> & outmarkers)
{
	if(swc_file.find_last_of(".dot") == swc_file.size() - 1) return saveDot_file(swc_file, outmarkers);

	cout<<"marker num = "<<outmarkers.size()<<", save swc file to "<<swc_file<<endl;
	map<MyMarker*, int> ind;	
	ofstream ofs(swc_file.c_str()); 
	
	if(ofs.fail())
	{
		cout<<"open swc file error"<<endl; 
		return false;
	}
	ofs<<"#name "<<swc_file<<endl;
	ofs<<"#comment "<<endl;
	ofs<<"##n,type,x,y,z,radius,parent"<<endl;
	for(int i = 0; i < outmarkers.size(); i++) ind[outmarkers[i]] = i+1;

	for(int i = 0; i < outmarkers.size(); i++)
	{
		MyMarker * marker = outmarkers[i];
		int parent_id;
		if(marker->parent == 0) parent_id = -1;
		else parent_id = ind[marker->parent];
		ofs<<i+1<<" "<<marker->type<<" "<<marker->x<<" "<<marker->y<<" "<<marker->z<<" "<<marker->radius<<" "<<parent_id<<endl;
	}
	ofs.close();
	return true;
}

bool saveDot_file(string dot_file, vector<MyMarker*> & outmarkers)
{
	cout<<"marker num = "<<outmarkers.size()<<", save swc file to "<<dot_file<<endl;
	map<MyMarker*, int> ind;	
	ofstream ofs(dot_file.c_str()); 
	
	if(ofs.fail())
	{
		cout<<"open swc file error"<<endl; 
		return false;
	}
	ofs<<"digraph \""<<dot_file<<"\" {"<<endl;
	ofs<<"\trankdir = BT;"<<endl;

	for(int i = 0; i < outmarkers.size(); i++) ind[outmarkers[i]] = i+1;
	for(int i = 0; i < outmarkers.size(); i++)
	{
		MyMarker * marker = outmarkers[i];
		if(marker->parent)
		{
			int parent_id = ind[marker->parent];
			MyMarker * parent = marker->parent;
			ofs<<"\t"<<i+1<<" -> "<<parent_id<<";"<<endl;
		}
	}
	ofs<<"}"<<endl;
	ofs.close();
	return true;
}

#ifdef __ESWC__
bool readESWC_file(string swc_file, vector<MyMarkerX*> & swc)
{
	ifstream ifs(swc_file.c_str());

	if(ifs.fail()) 
	{
		cout<<"open swc file : "<< swc_file <<" error"<<endl;
		return false;
	}

	map<int, MyMarkerX*> marker_map;
	map<MyMarkerX*, int> parid_map;
	while(ifs.good())
	{
		if(ifs.peek() == '#'){ifs.ignore(1000,'\n'); continue;}
		MyMarkerX *  marker = new MyMarkerX;
		int my_id = -1 ; ifs >> my_id;
		if(my_id == -1) break;
		if(marker_map.find(my_id) != marker_map.end())
		{
			cerr<<"Duplicate Node. This is a graph file. Please read it as a graph."<<endl; return false;
		}
		marker_map[my_id] = marker;

		ifs>> marker->type;
		ifs>> marker->x;
		ifs>> marker->y;
		ifs>> marker->z;
		ifs>> marker->radius;
		marker->seg_id = -1;
		marker->seg_level = -1;
		marker->feature = 0.0;
		int par_id = -1; ifs >> par_id;

		parid_map[marker] = par_id;
		swc.push_back(marker);
	}
	ifs.close();
	vector<MyMarkerX*>::iterator it = swc.begin();
	while(it != swc.end())
	{
		MyMarkerX * marker = *it;
		marker->parent = marker_map[parid_map[marker]];
		it++;
	}
	return true;
}

bool saveESWC_file(string swc_file, vector<MyMarkerX*> & outmarkers)
{
	cout<<"marker num = "<<outmarkers.size()<<", save swc file to "<<swc_file<<endl;
	map<MyMarkerX*, int> ind;	
	ofstream ofs(swc_file.c_str()); 
	
	if(ofs.fail())
	{
		cout<<"open swc file error"<<endl; 
		return false;
	}
	ofs<<"#name "<<swc_file<<endl;
	ofs<<"#comment "<<endl;
	ofs<<"##n,type,x,y,z,radius,parent,seg_id,seg_level,feature"<<endl;
	for(int i = 0; i < outmarkers.size(); i++) ind[outmarkers[i]] = i+1;

	for(int i = 0; i < outmarkers.size(); i++)
	{
		MyMarkerX * marker = outmarkers[i];
		int parent_id;
		if(marker->parent == 0) parent_id = -1;
		else parent_id = ind[(MyMarkerX *) marker->parent];
		ofs<<i+1<<" "<<marker->type<<" "<<marker->x<<" "<<marker->y<<" "<<marker->z<<" "<<marker->radius<<" "<<parent_id<<" "<<marker->seg_id<<" "<<marker->seg_level<<" "<<marker->feature<<endl;
	}
	ofs.close();
	return true;
}
#endif

double dist(MyMarker a, MyMarker b)
{
	return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}

vector<MyMarker*> getLeaf_markers(vector<MyMarker*> & inmarkers)
{
	set<MyMarker*> par_markers;
	vector<MyMarker*> leaf_markers;
	for(int i = 0; i < inmarkers.size(); i++)
	{
		MyMarker * marker = inmarkers[i];
		if(marker->parent) par_markers.insert(marker->parent);
	}
	for(int i = 0; i < inmarkers.size(); i++)
	{
		if(par_markers.find(inmarkers[i]) == par_markers.end()) leaf_markers.push_back(inmarkers[i]);
	}
	par_markers.clear();
	return leaf_markers;
}

vector<MyMarker*> getLeaf_markers(vector<MyMarker*> & inmarkers, map<MyMarker *, int> & childs_num)
{
	for(int i = 0; i < inmarkers.size(); i++) childs_num[inmarkers[i]] = 0;

	vector<MyMarker*> leaf_markers;
	for(int i = 0; i < inmarkers.size(); i++)
	{
		MyMarker * marker = inmarkers[i];
		MyMarker * parent = marker->parent;
		if(parent) childs_num[parent]++;
	}
	for(int i = 0; i < inmarkers.size(); i++)
	{
		if(childs_num[inmarkers[i]] == 0) leaf_markers.push_back(inmarkers[i]);
	}
	return leaf_markers;
}
#endif
