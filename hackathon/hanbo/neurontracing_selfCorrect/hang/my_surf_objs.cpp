#ifndef __MY_SURF_OBJS_CPP__
#define __MY_SURF_OBJS_CPP__
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <cmath>

#include "common_macro.h"     // MIN MAX
#include "my_surf_objs.h"

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
	cout<<count<<" marker read"<<endl;
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

bool saveMarker_file(string marker_file, vector<MyMarker> & out_markers)
{
	cout<<"save "<<out_markers.size()<<" markers to file "<<marker_file<<endl;
	ofstream ofs(marker_file.c_str()); 
	
	if(ofs.fail())
	{
		cout<<"open marker file error"<<endl; 
		return false;
	}
	ofs<<"#x, y, z, radius"<<endl;
	for(int i = 0; i < out_markers.size(); i++)
	{
		ofs<<out_markers[i].x + MARKER_BASE<<","<<out_markers[i].y + MARKER_BASE<<","<<out_markers[i].z + MARKER_BASE<<","<<out_markers[i].radius<<endl;
	}
	ofs.close();
	return true;
}

bool saveMarker_file(string marker_file, vector<MyMarker*> & out_markers)
{
	cout<<"save "<<out_markers.size()<<" markers to file "<<marker_file<<endl;
	ofstream ofs(marker_file.c_str()); 
	
	if(ofs.fail())
	{
		cout<<"open marker file error"<<endl; 
		return false;
	}
	ofs<<"#x, y, z, radius"<<endl;
	for(int i = 0; i < out_markers.size(); i++)
	{
		ofs<<out_markers[i]->x + MARKER_BASE<<","<<out_markers[i]->y + MARKER_BASE<<","<<out_markers[i]->z+MARKER_BASE<<","<<out_markers[i]->radius<<endl;
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

    cout<<"swc has " << swc.size() << "units" <<endl;

	return swc;
}

bool saveSWC_file(string swc_file, vector<MyMarker*> & out_markers)
{
	cout<<"marker num = "<<out_markers.size()<<", save swc file to "<<swc_file<<endl;
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
	for(int i = 0; i < out_markers.size(); i++) ind[out_markers[i]] = i+1;

	for(int i = 0; i < out_markers.size(); i++)
	{
		MyMarker * marker = out_markers[i];
		int parent_id;
		if(marker->parent == 0) parent_id = -1;
		else parent_id = ind[marker->parent];
		ofs<<i+1<<" "<<marker->type<<" "<<marker->x<<" "<<marker->y<<" "<<marker->z<<" "<<marker->radius<<" "<<parent_id<<endl;
	}
	ofs.close();
	return true;
}

double dist(MyMarker a, MyMarker b)
{
	return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}

#endif
