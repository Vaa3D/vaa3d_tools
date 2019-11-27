#include "map"
#include "v3d_message.h"
#include "iostream"
#include "basic_surf_objs.h"
using namespace std;
struct SwcTree{
	QString filePath;
	NeuronTree nt;
	QList <ImageMarker> listMarker;
	set<V3DLONG> branchPoints;
	set<V3DLONG> crossPoints;
	V3DLONG num_p;
	vector<vector<V3DLONG> > children;
	V3DLONG searchRadius =1;

	int initialize();
	int crossBranchDetect(int &crossType1, int &crossType2);
	int crossBranchDeal(NeuronTree &ntNoneCrossBranch);
	int dfs(NeuronSWC neu);
	// added by DZC 19 Jul 2019
	//    NeuronTree refine_swc(QString braindir, double thresh, V3DPluginCallback2 &callback);
	//    bool gd_on_nt(NeuronTree &branch_nt, NeuronTree & tree_out,QString braindir,V3DPluginCallback2 &callback);


};

struct Vector{
	double x;
	double y;
	double z;
};

template<class T>
inline double distance_two_point(T &point1, T &point2)
{
	return sqrt(((double)point1.x - (double)point2.x)*((double)point1.x - (double)point2.x) + ((double)point1.y - (double)point2.y)*((double)point1.y - (double)point2.y) + ((double)point1.z - (double)point2.z)*((double)point1.z - point2.z));
}

template<class T>
inline double angle_two_vector(T &vector1, T &vector2)
{
	double a = sqrt(vector1.x*vector1.x + vector1.y*vector1.y + vector1.z*vector1.z);
	double b = sqrt(vector2.x*vector2.x + vector2.y*vector2.y + vector2.z*vector2.z);
	double ab = vector1.x*vector2.x + vector1.y*vector2.y + vector1.z*vector2.z;
	double costheta = ab / (a*b);
	return acos(costheta) * 180 / 3.1415926;
}

int SwcTree::initialize(){
	
	num_p = nt.listNeuron.size();
	cout << "wp_debug: " << __LINE__ << " " << num_p << endl;

	NeuronSWC ori;
	children = vector<vector<V3DLONG> >(num_p, vector<V3DLONG>());

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
	
	return 0;
}



int SwcTree::crossBranchDetect(int &crossType1, int &crossType2){
	for (V3DLONG i = 0; i < num_p; i++){
		V3DLONG par = nt.listNeuron[i].parent;
		if (par<0)
		{
			//cout << "wp_debug: " << __FUNCTIONW__ << ": " << __LINE__ << endl;
			//cout << nt.listNeuron[i].radius << endl;
			continue;
		}
		//cout << children[nt.hashNeuron.value(par)].size() << endl;
		if (children[nt.hashNeuron.value(nt.listNeuron[i].n)].size()>1){
			branchPoints.insert(nt.hashNeuron.value(nt.listNeuron[i].n));
			//cout << par << " " << nt.listNeuron[par].n << " " << nt.listNeuron[par].parent << " " << nt.hashNeuron.value(par) << endl;
			//nt.listNeuron[nt.hashNeuron.value(par)].r = 10;
			//nt.listNeuron[nt.hashNeuron.value(par)].type = 5;
			//cout << i << " " << nt.listNeuron[i].x << endl;
		}
	}



	cout << "start" << endl;
	for (set<V3DLONG>::iterator it1 = branchPoints.begin(); it1 != branchPoints.end(); it1++){
		//cout << branchPoints[i] << endl;
		NeuronSWC p1 = nt.listNeuron[*it1];
		if (children[nt.hashNeuron.value(p1.n)].size() > 2){
			crossPoints.insert(*it1);
		}

		for (set<V3DLONG>::iterator it2 = it1; it2 != branchPoints.end(); it2++){

			NeuronSWC p2 = nt.listNeuron[*it2];
			if (p1.n == p2.n){
				continue;
			}
			if (distance_two_point(p1, p2) <= searchRadius){
				if (nt.listNeuron[*it1].parent == nt.listNeuron[*it2].n || nt.listNeuron[*it2].parent == nt.listNeuron[*it1].n){
					cout << p1.n << " " << p2.n << " " << distance_two_point(p1, p2) << endl;
					crossPoints.insert(*it1);
					crossPoints.insert(*it2);
				}

			}
		}

	}

	for (set<V3DLONG>::iterator it = crossPoints.begin(); it != crossPoints.end(); it++){
		//V3DLONG par = nt.listNeuron[*it].parent;

		ImageMarker marker;
		marker.x = nt.listNeuron[*it].x;
		marker.y = nt.listNeuron[*it].y;
		marker.z = nt.listNeuron[*it].z;


		if (children[*it].size()>2){
			crossType1++;
			marker.radius = 1;
			marker.type = 4;
			listMarker.push_back(marker);
			nt.listNeuron[*it].r = 5;
			nt.listNeuron[*it].type = 2;

		}
		else{
			crossType2++;
			marker.radius = 1;
			marker.type = 5;
			listMarker.push_back(marker);
			nt.listNeuron[*it].r = 5;
			nt.listNeuron[*it].type = 5;

		}


	}

	cout << "branchPoints: " << branchPoints.size() << endl;
	cout << "wp_debug: " << "crossPoints" << " " << crossPoints.size() << endl;


	return crossPoints.size();
}



int SwcTree::crossBranchDeal(NeuronTree &ntNoneCrossBranch){



	for (set<V3DLONG>::iterator it = crossPoints.begin(); it != crossPoints.end(); it++){
		Vector vector1, vector2;
		vector1.x = nt.listNeuron[*it].x - nt.listNeuron[nt.hashNeuron.value(nt.listNeuron[*it].parent)].x;
		vector1.y = nt.listNeuron[*it].y - nt.listNeuron[nt.hashNeuron.value(nt.listNeuron[*it].parent)].y;
		vector1.z = nt.listNeuron[*it].z - nt.listNeuron[nt.hashNeuron.value(nt.listNeuron[*it].parent)].z;

		double min_angle=999;
		V3DLONG min_angle_child = 0;
		if (children[*it].size() > 2){
			for (int i = 0; i < children[*it].size(); i++){
				NeuronSWC child = nt.listNeuron[children[*it][i]];
				vector2.x = child.x - nt.listNeuron[*it].x;
				vector2.y = child.y - nt.listNeuron[*it].y;
				vector2.z = child.z - nt.listNeuron[*it].z;
				double angle = angle_two_vector(vector1, vector2);
				if ( angle < min_angle){
					min_angle = angle;
					min_angle_child = i;
				}
			}

			for (int i = 0; i < children[*it].size(); i++){
				if (i == min_angle_child){
					continue;
				}
				
				dfs(nt.listNeuron[children[*it][i]]);
				
			}
		}
	}

	
	for (int i = 0; i < nt.listNeuron.size(); i++){
		if (nt.listNeuron[i].r != -1){
			ntNoneCrossBranch.listNeuron.append(nt.listNeuron[i]);
		}
	}
	return 0;
}

int SwcTree::dfs(NeuronSWC neu){
	nt.listNeuron[nt.hashNeuron.value(neu.n)].r = -1;
	nt.listNeuron[nt.hashNeuron.value(neu.n)].parent = -1;
	for (int i = 0; i < children[nt.hashNeuron.value(neu.n)].size(); i++){
		dfs(nt.listNeuron[children[nt.hashNeuron.value(neu.n)][i]]);
	}
	return 0;
}