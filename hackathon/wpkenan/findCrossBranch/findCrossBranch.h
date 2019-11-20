#include "map"
#include "v3d_message.h"
#include "iostream"
#include "basic_surf_objs.h"
using namespace std;
struct SwcTree{
	QString filePath;
	NeuronTree nt;
	QList <ImageMarker> listMarker;
	V3DLONG searchRadius =1;

	int initialize();

	// added by DZC 19 Jul 2019
	//    NeuronTree refine_swc(QString braindir, double thresh, V3DPluginCallback2 &callback);
	//    bool gd_on_nt(NeuronTree &branch_nt, NeuronTree & tree_out,QString braindir,V3DPluginCallback2 &callback);


};

template<class T>
inline double distance_two_point(T &point1, T &point2)
{
	return sqrt(((double)point1.x - (double)point2.x)*((double)point1.x - (double)point2.x) + ((double)point1.y - (double)point2.y)*((double)point1.y - (double)point2.y) + ((double)point1.z - (double)point2.z)*((double)point1.z - point2.z));
}

int SwcTree::initialize(){
	
	V3DLONG num_p = nt.listNeuron.size();
	cout << "wp_debug: " << __LINE__ << " " << num_p << endl;

	NeuronSWC ori;
	vector<vector<V3DLONG> > children = vector<vector<V3DLONG> >(num_p, vector<V3DLONG>());

	for (V3DLONG i = 0; i<num_p; ++i)
	{
		
		nt.listNeuron[i].r = 1;
		V3DLONG par = nt.listNeuron[i].parent;

		
		if (par<0)
		{
			ori = nt.listNeuron[i];
			continue;
		}
		children[nt.hashNeuron.value(par)].push_back(i);
	}

	set<V3DLONG> branchPoints;
	for (V3DLONG i = 0; i < num_p; i++){
		V3DLONG par = nt.listNeuron[i].parent;
		if (par<0)
		{
			continue;
		}
		//cout << children[nt.hashNeuron.value(par)].size() << endl;
		if (children[nt.hashNeuron.value(par)].size()>1){
			branchPoints.insert(nt.hashNeuron.value(par));
			//cout << par << " " << nt.listNeuron[par].n << " " << nt.listNeuron[par].parent << " " << nt.hashNeuron.value(par) << endl;
			//nt.listNeuron[nt.hashNeuron.value(par)].r = 10;
			//nt.listNeuron[nt.hashNeuron.value(par)].type = 5;
			//cout << i << " " << nt.listNeuron[i].x << endl;
		}
	}

	
	set<V3DLONG> crossPoints;
	cout << "start" << endl;
	for (set<V3DLONG>::iterator it1 = branchPoints.begin(); it1 != branchPoints.end(); it1++){
		//cout << branchPoints[i] << endl;
		NeuronSWC p1 = nt.listNeuron[*it1];
		for (set<V3DLONG>::iterator it2 = it1; it2 != branchPoints.end(); it2++){
			
			NeuronSWC p2 = nt.listNeuron[*it2];
			if (p1.n == p2.n){
				continue;
			}
			//cout << distance_two_point(p1, p2) << endl;
			if (distance_two_point(p1, p2) <= searchRadius){
				cout << p1.n << " " << p2.n << " " << distance_two_point(p1, p2) << endl;
				crossPoints.insert(*it1);
				crossPoints.insert(*it2);
			}
		}
		
	}

	for (set<V3DLONG>::iterator it = crossPoints.begin(); it != crossPoints.end(); it++){
		V3DLONG par = nt.listNeuron[*it].parent;

		ImageMarker marker;
		marker.x = nt.listNeuron[*it].x;
		marker.y = nt.listNeuron[*it].y;
		marker.z = nt.listNeuron[*it].z;
		
	
		if (children[nt.hashNeuron.value(par)].size()>2){
			marker.radius = 1;
			marker.type = 4;
			listMarker.push_back(marker);
			nt.listNeuron[*it].r = 10;
			nt.listNeuron[*it].type = 4;
			
		}
		else{
			marker.radius = 1;
			marker.type = 5;
			listMarker.push_back(marker);
			nt.listNeuron[*it].r = 10;
			nt.listNeuron[*it].type = 5;
			
		}
		

	}

	cout << "branchPoints: " << branchPoints.size() << endl;
	cout << "wp_debug: " << "crossPoints" << " " << crossPoints.size() << endl;


	

	return crossPoints.size();
}