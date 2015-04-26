// topology_analysis.h     Jan 24, 2012, by Hang Xiao

#ifndef __TOPOLOGY_ANALYSIS_H__
#define __TOPOLOGY_ANALYSIS_H__

#include <string>
#include "stackutil.h"
#include "fastmarching_linker.h"
#include "fastmarching_tree.h"

using namespace std;

#define INF 3.4e+38

enum{C_BLACK=1, C_RED , C_BLUE, C_PINK, C_LIGHT_GREEN, C_YELLOW, C_GREEN, C_BROWN, C_LIGHT_YELLOW, C_LIGHT_PINK, C_LIGHT_BLUE};

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

/**********************************************************************************************
 * topology_analysis
 *
 * function : analysis the toplogy of swc struct by disconnect every leaf segment step by step
 *
 * ********************************************************************************************/
#define COMPUTE_LENGTH_AND_SIGNAL(len, sig, markers)  \
{\
	len = 0.0; \
	sig = 0.0; \
	for(int i = 0; i < markers.size() - 1; i++) \
	{ \
		MyMarker * marker1 = markers[i], *marker2 = markers[i+1]; \
		int x1 = marker1->x + 0.5, x2 = marker2->x + 0.5; \
		int y1 = marker1->y + 0.5, y2 = marker2->y + 0.5; \
		int z1 = marker1->z + 0.5, z2 = marker2->z + 0.5; \
		long ind1 = z1*sz01 + y1*sz0 + x1, ind2 = z2*sz01 + y2*sz0 + x2; \
		double dist = (marker2->x - marker1->x) * (marker2->x - marker1->x) + \
					  (marker2->y - marker1->y) * (marker2->y - marker1->y) + \
					  (marker2->z - marker1->z) * (marker2->z - marker1->z); \
		dist = sqrt(dist); \
		if(dist == 0.0) continue; \
		len += dist *(GI(ind1) + GI(ind2))/2.0; \
		double tx = (marker2->x - marker1->x) /dist; \
		double ty = (marker2->y - marker1->y) /dist; \
		double tz = (marker2->z - marker1->z) /dist; \
		set<long> marker_set; \
		for(double d = 0; d <= dist + 0.5; d++) \
		{ \
			int x = x1 + tx * d; int y = y1 + ty * d; int z = z1 + tz * d; \
			long ind = z*sz01 + y*sz0 + x; marker_set.insert(ind);\
		}\
		double sum_int = 0.0; \
		for(set<long>::iterator it = marker_set.begin(); it != marker_set.end(); it++) sum_int += inimg1d[*it]; \
		sig += dist * sum_int/marker_set.size(); \
	}\
}

// GI parameter min_int, max_int, li
#define COMPUTE_MIN_MAX_INT  \
	double max_int = 0;  \
	double min_int = INF; \
	for(long i = 0; i < tol_sz; i++)  \
	{ \
		if(inimg1d[i] > max_int) max_int = inimg1d[i]; \
		if(inimg1d[i] < min_int) min_int = inimg1d[i]; \
	}\
	max_int -= min_int; \
	double li = 10;

#define MASK_SIBLING_AREA(tmpimg1d) \
{\
	for(int m = 0; m < sib_markers.size(); m++)\
	{\
		int x = sib_markers[m]->x + 0.5;\
		int y = sib_markers[m]->y + 0.5;\
		int z = sib_markers[m]->z + 0.5;\
		int r = sib_markers[m]->radius;\
		double r2 = r*r;\
		for(int kk = z -r; kk <= z+r; kk++)\
		{\
			if(kk < 0 || kk >= sz2) continue;\
			for(int jj = y - r; jj <= y+r; jj++)\
			{\
				if(jj < 0 || jj >= sz1) continue;\
				for(int ii = x - r; ii <= x+r; ii++)\
				{\
					if(ii < 0 || ii >= sz0) continue;\
					double rr = (kk-z)*(kk-z) + (jj-y)*(jj-y) + (ii-x)*(ii-x);\
					if(rr <= r2) tmpimg1d[kk*sz01 + jj*sz0 + ii] = 0;\
				}\
			}\
		}\
	}\
}

#define UNMASK_SIBLING_AREA(tmpimg1d) \
{\
	/*string tmpimg_file = string("topology_mask" + num2str(i,'0',3) + ".tif"); */\
	/*saveImage(tmpimg_file.c_str(), tmpimg1d, tmp_sz, V3D_UINT8); */\
	for(int m = 0; m < sib_markers.size(); m++)\
	{\
		int x = sib_markers[m]->x + 0.5;\
		int y = sib_markers[m]->y + 0.5;\
		int z = sib_markers[m]->z + 0.5;\
		int r = sib_markers[m]->radius;\
		double r2 = r*r;\
		for(int kk = z -r; kk <= z+r; kk++)\
		{\
			if(kk < 0 || kk >= sz2) continue;\
			for(int jj = y - r; jj <= y+r; jj++)\
			{\
				if(jj < 0 || jj >= sz1) continue;\
				for(int ii = x - r; ii <= x+r; ii++)\
				{\
					if(ii < 0 || ii >= sz0) continue;\
					double rr = (kk-z)*(kk-z) + (jj-y)*(jj-y) + (ii-x)*(ii-x);\
					if(rr <= r2)\
					{\
						long tmp_ind = kk*sz01 + jj*sz0 + ii;\
						tmpimg1d[tmp_ind] = inimg1d[tmp_ind];\
					}\
				}\
			}\
		}\
	}\
}

double distance_between_lines0(vector<MyMarker*> & line1, vector<MyMarker*> & line2)
{
	if(line1.empty() || line2.empty()) return INF;
	double sum_dist1 = 0.0;
	for(int i = 0; i < line1.size(); i++)
	{
		MyMarker * marker1 = line1[i];
		double min_dist = MAX_DOUBLE;
		for(int j = 0; j < line2.size(); j++)
		{
			MyMarker * marker2 = line2[j];
			double dst = dist(*marker1, *marker2);
			min_dist = MIN(dst, min_dist);
		}
		sum_dist1 += min_dist;
	}
	double sum_dist2 = 0.0;
	for(int j = 0; j < line2.size(); j++)
	{
		MyMarker * marker2 = line2[j];
		double min_dist = MAX_DOUBLE;
		for(int i = 0; i < line1.size(); i++)
		{
			MyMarker * marker1 = line1[i];
			double dst = dist(*marker1, *marker2);
			min_dist = MIN(dst, min_dist);
		}
		sum_dist2 += min_dist;
	}
	return (sum_dist1/line1.size() + sum_dist2/line2.size())/2.0;
}

// use percentage
double distance_between_lines1(vector<MyMarker*> & line1, vector<MyMarker*> & line2, double thresh = 2.0)
{
	if(line1.empty() || line2.empty()) return INF;
	double sum1 = 0.0;
	for(int i = 0; i < line1.size(); i++)
	{
		MyMarker * marker1 = line1[i];
		double min_dist = MAX_DOUBLE;
		for(int j = 0; j < line2.size(); j++)
		{
			MyMarker * marker2 = line2[j];
			double dst = dist(*marker1, *marker2);
			min_dist = MIN(dst, min_dist);
		}
		sum1 += (min_dist >= thresh) ? 1.0 : 0.0;
	}
	double sum2 = 0.0;
	for(int j = 0; j < line2.size(); j++)
	{
		MyMarker * marker2 = line2[j];
		double min_dist = MAX_DOUBLE;
		for(int i = 0; i < line1.size(); i++)
		{
			MyMarker * marker1 = line1[i];
			double dst = dist(*marker1, *marker2);
			min_dist = MIN(dst, min_dist);
		}
		sum2 += (min_dist >= thresh) ? 1.0 : 0.0;
	}
	sum1 /= line1.size();
	sum2 /= line2.size();
	return MAX(sum1, sum2);
	//return (sum1/line1.size() + sum2/line2.size())/2.0;
}

//use maximum of minimum distance
double distance_between_lines2(vector<MyMarker*> & line1, vector<MyMarker*> & line2)
{
    if(line1.empty() || line2.empty()) return INF;
    double max_dist1 = 0.0;
    for(int i = 0; i < line1.size(); i++)
    {
        MyMarker * marker1 = line1[i];
        double min_dist = MAX_DOUBLE;
        for(int j = 0; j < line2.size(); j++)
        {
            MyMarker * marker2 = line2[j];
            double dst = dist(*marker1, *marker2);
            min_dist = MIN(dst, min_dist);
        }
        max_dist1 = MAX(min_dist,max_dist1);
    }
    double max_dist2 = 0.0;
    for(int j = 0; j < line2.size(); j++)
    {
        MyMarker * marker2 = line2[j];
        double min_dist = MAX_DOUBLE;
        for(int i = 0; i < line1.size(); i++)
        {
            MyMarker * marker1 = line1[i];
            double dst = dist(*marker1, *marker2);
            min_dist = MIN(dst, min_dist);
        }
        max_dist2 = MAX(min_dist,max_dist2);
    }
    return MIN(max_dist1,max_dist2);
}
// algorithm flow
// 1. swc to neuron segment
// 2. for each segment, calculate the new path between the start point and the end point
// 3. calcute the corelation between new path and old path
#define SAVE_TOPOLOGY_ANALYSIS_TEMP_SWC \
{\
	vector<MyMarker*> outswc;  \
	outswc.insert(outswc.end(), inmarkers.begin(), inmarkers.end());\
	outswc.insert(outswc.end(), linker.begin(), linker.end()); \
	for(int m = 0; m < outswc.size(); m++) outswc[m]->type = C_GREEN; \
	for(int m = 0; m < seg->markers.size(); m++) seg->markers[m]->type = C_RED; \
	for(int m = 0; m < linker.size(); m++) linker[m]->type = C_BLUE; \
	string tmpswc_file = "topology_swc" + num2str(i, '0', 3) + ".swc"; \
	saveSWC_file(tmpswc_file, outswc); \
}

#define GET_LEFT_RIGHT_SEGMENTS \
{\
	for(int j = 0; j < leaf_segs.size(); j++) \
	{\
		bool is_breaked = false;\
		vector<NeuronSegment*> segs_bottom;\
		vector<NeuronSegment*> segs_top;\
		NeuronSegment * p = leaf_segs[j]; \
		while(p)\
		{\
			if(p == cur_seg) {is_breaked = true; p = p->parent; continue;}\
			else\
			{\
				if(is_breaked) segs_top.push_back(p);\
				else segs_bottom.push_back(p);\
				p=p->parent;\
			}\
		}\
		if(is_breaked)\
		{\
			left_segs.insert(segs_bottom.begin(), segs_bottom.end());\
			right_segs.insert(segs_top.begin(), segs_top.end());\
		}\
		else\
		{\
			right_segs.insert(segs_bottom.begin(), segs_bottom.end());\
		}\
	}\
}

#define GET_LEFT_RIGHT_TARGET_SEEDS \
{\
	set<NeuronSegment*>::iterator it;\
	if(!left_segs.empty())\
	{\
		for(it = left_segs.begin(); it != left_segs.end(); it++)\
		{\
			NeuronSegment * tmp_seg = *it;\
			left_seeds.insert(left_seeds.end(), tmp_seg->markers.begin(), tmp_seg->markers.end());\
		}\
		left_seeds.push_back(cur_seg->first());\
	}\
	if(!right_segs.empty())\
	{\
		for(it = right_segs.begin(); it != right_segs.end(); it++)\
		{\
			NeuronSegment * tmp_seg = *it;\
			right_seeds.insert(right_seeds.end(), tmp_seg->markers.begin(), tmp_seg->markers.end());\
		}\
	}\
	target_seeds.insert(target_seeds.end(), cur_seg->markers.begin()+1, cur_seg->markers.end());\
}

#define VORONOI_MASK_TO_LINK_MASK \
{\
	parent = new long[tol_sz]; for(long ind = 0; ind < tol_sz; ind++) parent[ind] = ind;\
	map<int, long> msk_id_map;\
	for(long ind = 0; ind < tol_sz; ind++)\
	{\
		int msk_id = voro_mask[ind];\
		if(msk_id_map.find(msk_id) != msk_id_map.end())\
		{\
			long ind2 = msk_id_map[msk_id];\
			long tmp = parent[ind];\
			parent[ind] = parent[ind2];\
			parent[ind2] = tmp;\
		}\
		else msk_id_map[msk_id] = ind;\
	}\
	msk_id_map.clear();\
	{\
		vector<long> msk_sz(inmarkers.size(), 0);\
		long bkg_sz = 0;\
		for(long ind = 0; ind < tol_sz; ind++) if(voro_mask[ind] > 0) msk_sz[voro_mask[ind]-1]++;else bkg_sz++;\
		long sum = bkg_sz;\
		for(int m = 0; m < inmarkers.size(); m++)sum+= msk_sz[m];\
        cout<<"bkg_sz = "<<bkg_sz<<endl;\
		cout<<"sum = "<<sum<<endl; if(sum != tol_sz) cout<<"sum is not euqal to tol_sz : "<<tol_sz<<endl;else cout<<"sum is equal to tol_sz"<<endl;\
		long sum2 = 0;\
		for(int m = 0; m < inmarkers.size(); m++)\
		{\
			int x = inmarkers[m]->x + 0.5;\
			int y = inmarkers[m]->y + 0.5;\
			int z = inmarkers[m]->z + 0.5;\
			long ind = z * sz01 + y * sz0 + x;\
			sum2++;\
			long pind = parent[ind];\
			while(pind != ind){sum2++; pind = parent[pind];}\
		}\
		cout<<"sum2 = "<<sum2<<endl; if(sum2 != tol_sz) cout<<"sum2 is not euqal to tol_sz : "<<tol_sz<<endl;else cout<<"sum2 is equal to tol_sz"<<endl;\
	}\
}

#define VORONOI_MASK_TO_LINK_MASK2 \
{\
	long tol_sz = sz0 * sz1 * sz2;\
	parent = new long[tol_sz];\
	bool * visited = new bool[tol_sz]; for(long ind = 0; ind < tol_sz; ind++) visited[ind] = 0;\
	for(long ind = 0; ind < tol_sz; ind++) parent[ind] = ind;\
\
	long i = -1, j = -1, k = -1;\
	for(long ind = 0; ind < tol_sz; ind++)\
	{\
		i++; if(i%sz0 ==0){i= 0; j++; if(j%sz1 == 0){j=0; k++;}}\
		if(visited[ind]) continue;\
		for(long kk = -1; kk <=1; kk++)\
		{\
			long k2 = kk + k;\
			if(k2 < 0 || k2 >= sz2) continue;\
			for(long jj = -1; jj <= 1; jj++)\
			{\
				long j2 = jj + j;\
				if(j2 < 0 || j2 >= sz1) continue;\
				for(long ii = -1; ii <= 1; ii++)\
				{\
					long i2 = ii + i;\
					if(i2 < 0 || i2 >= sz0) continue;\
					long offset = ABS(ii) + ABS(jj) + ABS(kk);\
					if(offset > cnn_type) continue;\
					long ind2 = k2 * sz01 + j2 * sz0 + i2;\
					if(!visited[ind2] && voro_mask[ind] == voro_mask[ind2])\
					{\
						long tmp_par = parent[ind2];\
						parent[ind2] = parent[ind];\
						parent[ind] = tmp_par;\
						visited[ind2] = 1;\
					}\
				}\
			}\
		}\
		visited[ind] = 1;\
	}\
	delete [] visited; visited = 0;\
	{\
		vector<long> msk_sz(inmarkers.size(), 0);\
		long bkg_sz = 0;\
		for(long ind = 0; ind < tol_sz; ind++) if(voro_mask[ind] > 0) msk_sz[voro_mask[ind]-1]++;else bkg_sz++;\
		long sum = bkg_sz;\
		for(int m = 0; m < inmarkers.size(); m++)sum+= msk_sz[m];\
		cout<<"bkg_sz = "<<bkg_sz<<endl;\
		cout<<"sum = "<<sum<<endl; if(sum != tol_sz) cout<<"sum is not euqal to tol_sz : "<<tol_sz<<endl;else cout<<"sum is equal to tol_sz"<<endl;\
		long sum2 = 0;\
		for(int m = 0; m < inmarkers.size(); m++)\
		{\
			int x = inmarkers[m]->x + 0.5;\
			int y = inmarkers[m]->y + 0.5;\
			int z = inmarkers[m]->z + 0.5;\
			long ind = z * sz01 + y * sz0 + x;\
			sum2++;\
			long pind = parent[ind];\
			while(pind != ind){sum2++; pind = parent[pind];}\
		}\
		cout<<"sum2 = "<<sum2<<endl; if(sum2 != tol_sz) cout<<"sum2 is not euqal to tol_sz : "<<tol_sz<<endl;else cout<<"sum2 is equal to tol_sz"<<endl;\
	}\
}

#define INIT_TMP_PHI_VORO_STATE(markers) \
{\
	int voro_ind = 0;\
	int msk_sz = 0;\
	for(vector<MyMarker*>::iterator it = markers.begin(); it != markers.end(); it++) \
	{\
		MyMarker * marker = *it; \
		long x = marker->x + 0.5;\
		long y = marker->y + 0.5;\
		long z = marker->z + 0.5;\
		if(x < 0 || x >= sz0 || y < 0 || y >= sz1 || z < 0 || z >= sz2) continue;\
		msk_sz++;\
		long ind = z * sz01 + y * sz0 + x;\
		tmp_phi[ind] = phi[ind];\
		tmp_state[ind] = ALIVE;\
		tmp_voro[ind] = voro_ind+1;\
		long par_ind = parent[ind];\
		while(par_ind != ind)\
		{\
			tmp_phi[par_ind] = phi[par_ind];\
			tmp_state[par_ind] = ALIVE;\
			tmp_voro[par_ind] = voro_ind+1;\
			par_ind = parent[par_ind];\
		}\
		voro_ind++;\
	}\
	cout<<"msk_sz = "<<msk_sz<<endl;\
}

#define SAVE_MARCHING_RESULT_TO_SWC1 \
if(par_marker) {\
	MyMarker * tmp_marker1 = new MyMarker; *tmp_marker1 = *(target_seeds[m]);\
	MyMarker * tmp_marker2 = new MyMarker; *tmp_marker2 = *par_marker;\
	tmp_marker1->parent = tmp_marker2; tmp_marker2->parent = 0;\
	tmp_swc.push_back(tmp_marker1);\
	tmp_swc.push_back(tmp_marker2);\
}

#define SAVE_MARCHING_RESULT_TO_SWC2 \
{\
	string tmp_file = "seg" + num2str(seg_id) + ".swc";\
	saveSWC_file(tmp_file, tmp_swc);\
}
	//tmp_swc.insert(tmp_swc.end(), target_seeds.begin(), target_seeds.end());\
	//tmp_swc.insert(tmp_swc.end(), right_seeds.begin(), right_seeds.end());\

//mask the image by inswc, fast marching from inswc[0] to inswc[end] to generate outswc
template<class T> bool perturb_linker(const vector<MyMarker *> &inswc, T * inimg1d, vector<MyMarker *> &outswc, double radiusFactor, int sz0, int sz1, int sz2, T * in_masked_img1d = 0, int cnn_type = 2)
{
    if(inswc.size()<2)
        return false;
    T* masked_img1d = in_masked_img1d;
    if(!masked_img1d){
        masked_img1d = new T[sz0*sz1*sz2];
        memcpy(masked_img1d, inimg1d, sz0*sz1*sz2*sizeof(T));
    }

    //mask the image
    double STEP=0.4;
    for(int nid=0; nid<inswc.size()-1; nid++){
        MyMarker* current = inswc.at(nid);
        MyMarker* parent = current->parent;
        if(parent<=0){
            continue; //continue if it has no parent
        }

        double xf,yf,zf,rf;
        int xi,yi,zi,ri;
        double xv=parent->x-current->x;
        double yv=parent->y-current->y;
        double zv=parent->z-current->z;
        double rv=parent->radius-current->radius;
        double len=sqrt(xv*xv+yv*yv+zv*zv);

        for(double lenAccu=0; lenAccu<len; lenAccu+=STEP){
            xf=current->x+xv*lenAccu/len;
            yf=current->y+yv*lenAccu/len;
            zf=current->z+zv*lenAccu/len;
            rf=(current->radius+rv*lenAccu/len)*radiusFactor;
            ri=rf+0.5;
            for(int dx=-ri; dx<=ri; dx++){
                for(int dy=-ri; dy<=ri; dy++){
                    for(int dz=-ri; dz<=ri; dz++){
                        if(dx*dx+dy*dy+dz*dz>rf*rf){
                            continue;
                        }
                        xi=xf+dx;
                        yi=yf+dy;
                        zi=zf+dz;
                        if(xi<0 || xi>=sz0)
                            continue;
                        if(yi<0 || zi>=sz1)
                            continue;
                        if(zi<0 || zi>=sz2)
                            continue;
                        masked_img1d[xi+yi*sz0+zi*sz0*sz1]=0;
                    }
                }
            }
        }
    }
    bool result=fastmarching_linker(*(inswc.front()),*(inswc.back()),masked_img1d,outswc,sz0,sz1,sz2,cnn_type);

//    //for test
//    V3DLONG sz[4]={1};
//    sz[0]=sz0; sz[1]=sz1; sz[2]=sz2; sz[3]=1;
//    char tmp_fname[1000];
//    sprintf(tmp_fname,"test_63_masked.raw");
//    saveImage(tmp_fname, (unsigned char *)masked_img1d, sz, 1);
//    sprintf(tmp_fname,"test_63_linkage.swc");
//    saveSWC_file(tmp_fname,outswc);

    if(in_masked_img1d){
        memcpy(masked_img1d, inimg1d, sz0*sz1*sz2*sizeof(T));
    }else{
        delete [] masked_img1d;
    }
    return result;
}

//link inseg[0] to inseg[end] by a straight line
template<class T> bool direct_linker(const vector<MyMarker *> &inswc, vector<MyMarker *> &outswc)
{
    if(inswc.size()<2)
        return false;
    double STEP = 1;
    MyMarker * start = *(inswc.begin());
    MyMarker * end = *(inswc.back());

    double xf,yf,zf,rf;
    int xi,yi,zi,ri;
    double xv=end->x-start->x;
    double yv=end->y-start->y;
    double zv=end->z-start->z;
    double len=sqrt(xv*xv+yv*yv+zv*zv);
    outswc.clear();
    for(double dis=0; dis<len; dis+=STEP){
        MyMarker * newmark = new MyMarker(start->x+xv*dis/len,start->y+yv*dis/len,start->z+zv*dis/len);
        if(outswc.size()>0){
            newmark->parent = (*outswc.back());
        }
        outswc.push_back(newmark);
    }

    MyMarker * newend = new MyMarker(start->x,start->y,start->z);
    if(outswc.size()>0){
        newend->parent = (*outswc.back());
    }
    outswc.push_back(newend);

    return true;
}

//inswc should be a ordered segment that inswc[i]->parent=inswc[i+1]
//intensity[i]=img(inswc(i));
//length=sum(distance(i,i+1))
//intensity_accu=sum((intensity[i]+intensity[i+1])/2*distance(i,i+1))
template<class T> bool get_linker_intensScore(const vector<MyMarker *> &inswc, T * inimg1d, vector<double> &intensity, double &length, double &intensity_accu, int sz0, int sz1, int sz2)
{
    length=0;
    intensity_accu=0;
    MyMarker* cur = 0;
    intensity.resize(inswc.size(),0);

    if(inswc.size()<1) return false;

    int x,y,z;
    for(int i=0; i<inswc.size(); i++){
        cur = inswc[i];
        x = inswc[i]->x;
        y = inswc[i]->y;
        z = inswc[i]->z;
        if(x>=0 && x<sz0 && y>=0 && y<sz1 && z>=0 && z<sz2){
            intensity[i] = inimg1d[x+y*sz0+z*sz0*sz1];
        }
    }
    if(inswc.size()<2){
        qDebug()<<"WARNING: single node trace";
        intensity_accu=intensity[0];
    }else{
        if(inswc.at(0)->parent==inswc.at(1)){
            for(int i=0; i<inswc.size()-1; i++){
                cur = inswc[i];
                MyMarker* parent = cur->parent;
                if(parent==0){ //should not happen
                    qDebug()<<"WARNING: unexpected things happened. Check the data!";
                    continue;
                }
                double dis=(cur->x-parent->x)*(cur->x-parent->x);
                dis+=(cur->y-parent->y)*(cur->y-parent->y);
                dis+=(cur->z-parent->z)*(cur->z-parent->z);
                dis=sqrt(dis);
                length+=dis;
                intensity_accu+=dis*(intensity[i]+intensity[i+1])/2;
            }
        }else if(inswc.at(1)->parent==inswc.at(0)){
            for(int i=1; i<inswc.size(); i++){
                cur = inswc[i];
                MyMarker* parent = cur->parent;
                if(parent==0){ //should not happen
                    qDebug()<<"WARNING: unexpected things happened. Check the data!";
                    continue;
                }
                double dis=(cur->x-parent->x)*(cur->x-parent->x);
                dis+=(cur->y-parent->y)*(cur->y-parent->y);
                dis+=(cur->z-parent->z)*(cur->z-parent->z);
                dis=sqrt(dis);
                length+=dis;
                intensity_accu+=dis*(intensity[i]+intensity[i-1])/2;
            }
        }else{
            qDebug()<<"WARNING: linker is not sorted";
            return false;
        }
    }
    return true;
}

//inswc should be a ordered segment that inswc[i]->parent=inswc[i+1]
//intensity[i]=img(inswc(i));
//length=sum(distance(i,i+1))
//intensity_accu=sum((intensity[i]+intensity[i+1])/2*distance(i,i+1))
template<class T> vector<double> intensScore_between_linkers(const vector<MyMarker*> &orgswc, const vector<MyMarker*> &newswc, T * inimg1d, int sz0, int sz1, int sz2, int scoreType=0)
{
    vector<double> scores;
    double orgLen, orgAccu, newLen, newAccu, newMean, orgMean;
    get_linker_intensScore(newswc, inimg1d, scores, newLen, newAccu, sz0, sz1, sz2);
    newMean = newAccu/(newLen+1e-16);
    //for test
    cout<<"\t new swc score: "<<newswc.size()<<" : "<<newAccu<<" : "<<newLen;
    get_linker_intensScore(orgswc, inimg1d, scores, orgLen, orgAccu, sz0, sz1, sz2);
    //for test
    cout<<";\torig swc score: "<<orgswc.size()<<" : "<<orgAccu<<" : "<<orgLen;
    orgMean=orgAccu/(orgLen+1e-16);
    for(int i=0; i<scores.size(); i++){
        if(scoreType==1){
            scores[i]=newMean/orgMean;
        }else{
            scores[i]=newMean/scores[i];
        }
    }
    return scores;
}

template<class T> bool topology_analysis0_old(T * inimg1d, vector<MyMarker*> & inmarkers, int sz0, int sz1, int sz2, int cnn_type = 2.0)
{
	vector<NeuronSegment*> neuron_segs = swc_to_neuron_segment(inmarkers);
	vector<NeuronSegment*> leaf_segs = leaf_neuron_segments(neuron_segs);

	int n = neuron_segs.size(); cout<<"segment number = "<<n<<endl;
	cout<<"leaf number = "<<leaf_segs.size()<<endl;

	map<MyMarker*, double> score_map; for(int m = 0; m < inmarkers.size(); m++) score_map[inmarkers[m]] = 0.0;

	map<MyMarker*, NeuronSegment*> marker_map;
	for(int i = 0; i < n; i++) 
	{
		NeuronSegment * seg = neuron_segs[i];
		for(vector<MyMarker*>::iterator it = seg->markers.begin(); it != seg->markers.end(); it++) marker_map[*it] = seg;
	}

	long tol_sz = sz0 * sz1 * sz2, sz01 = sz0 * sz1;

	float * phi = 0; int * voro_mask = 0; long * parent = 0;
	float * tmp_phi = new float[tol_sz]; 
	char * tmp_state = new char[tol_sz]; 
	int * tmp_voro = new int[tol_sz]; 
	vector<MyMarker> allmarkers(inmarkers.size()); for(int m = 0; m < inmarkers.size(); m++) allmarkers[m]=*(inmarkers[m]);
	fastmarching_voronoi(allmarkers, inimg1d, voro_mask, phi, sz0, sz1, sz2, cnn_type);
	cout<<"phi = "<<phi<<endl;
	assert( phi != 0);
	VORONOI_MASK_TO_LINK_MASK;

	for(int seg_id = 0; seg_id < n; seg_id++)
	{
		cout<<" seg "<<seg_id<<endl;
		NeuronSegment * cur_seg = neuron_segs[seg_id];
		set<NeuronSegment * > left_segs; 
		set<NeuronSegment * > right_segs;
		GET_LEFT_RIGHT_SEGMENTS;

		vector<MyMarker*> left_seeds; left_seeds.reserve(1000);
		vector<MyMarker*> right_seeds; right_seeds.reserve(1000);
		vector<MyMarker*> target_seeds; target_seeds.reserve(1000);
		GET_LEFT_RIGHT_TARGET_SEEDS;

		if(target_seeds.empty()) continue;

		vector<MyMarker> tar_markers(target_seeds.size()); for(int m = 0; m < target_seeds.size(); m++) tar_markers[m] = *(target_seeds[m]);
		enum{ALIVE = -1, TRIAL = 0, FAR = 1};
	
		vector<double> left_scores(target_seeds.size(),1.0);
		vector<double> right_scores(target_seeds.size(),1.0);
		// left fastmarching
		if(0 && !left_seeds.empty())
		{
			for(long ind = 0; ind < tol_sz; ind++) tmp_phi[ind] = INF;
			for(long ind = 0; ind < tol_sz; ind++) tmp_state[ind] = FAR;
			for(long ind = 0; ind < tol_sz; ind++) tmp_voro[ind] = 0;
			INIT_TMP_PHI_VORO_STATE(left_seeds);

			fastmarching_voronoi(tar_markers, inimg1d, tmp_phi, tmp_state,  tmp_voro, sz0, sz1, sz2, cnn_type); 
			for(int m = 0; m < target_seeds.size(); m++)
			{
				int x = target_seeds[m]->x + 0.5;
				int y = target_seeds[m]->y + 0.5;
				int z = target_seeds[m]->z + 0.5;
				long ind = z * sz01 + y * sz0 + x;
				MyMarker * par_marker = tmp_voro[ind] >= 1 ? left_seeds[tmp_voro[ind]-1] : 0;
				NeuronSegment * par_seg = (par_marker != 0) ? marker_map[par_marker] : 0;
				double left_score = 1.0;
				if(par_seg == cur_seg) left_score = 1.0;
				else if(par_seg->parent == cur_seg)
				{
					MyMarker * p = par_marker;
					double len = 1.0;
					while(p != par_seg->last()){len++; p = p->parent;}
					left_score = (par_seg->markers.size()  - len) /par_seg->markers.size();
				}
				else left_score = 0.0;
				left_scores[m] = left_score;
			}
		}
		// right fastmarching
		if(!right_seeds.empty())
		{
			for(long ind = 0; ind < tol_sz; ind++) tmp_phi[ind] = INF;
			for(long ind = 0; ind < tol_sz; ind++) tmp_state[ind] = FAR;
			for(long ind = 0; ind < tol_sz; ind++) tmp_voro[ind] = 0;
			INIT_TMP_PHI_VORO_STATE(right_seeds);

			fastmarching_voronoi(tar_markers, inimg1d, tmp_phi, tmp_state,  tmp_voro, sz0, sz1, sz2, cnn_type); 
			vector<MyMarker*> tmp_swc;
			for(int m = 0; m < target_seeds.size(); m++)
			{
				int x = target_seeds[m]->x + 0.5;
				int y = target_seeds[m]->y + 0.5;
				int z = target_seeds[m]->z + 0.5;
				long ind = z * sz01 + y * sz0 + x;
				MyMarker * par_marker = tmp_voro[ind] >= 1 ? right_seeds[tmp_voro[ind]-1] : 0;
				NeuronSegment * par_seg = (par_marker != 0) ? marker_map[par_marker] : 0;
				double right_score = 1.0;
				if(par_seg == cur_seg->parent || (par_seg && par_seg->parent == cur_seg->parent))
				{
					MyMarker * p = par_marker;
					double len = 1.0;
					while(p != par_seg->last()){len++; p = p->parent;}
					right_score = len /par_seg->markers.size();
				}
				else right_score = 0.0;
				right_scores[m] = right_score;
				SAVE_MARCHING_RESULT_TO_SWC1;
			}
			SAVE_MARCHING_RESULT_TO_SWC2;
		}
		// average score
		for(int m = 0; m < target_seeds.size(); m++)
		{
			double score = right_scores[m];//(left_scores[m] + right_scores[m])/2.0;
			cout<<"score "<<m<<" = "<<score<<endl;
			target_seeds[m]->type = (1.0 - score) * 255 + 0.5 + 19;
		}
	}

	if(phi){delete [] phi; phi = 0;}
	if(voro_mask){delete [] voro_mask; voro_mask = 0;}
	if(parent){delete [] parent; parent = 0;}
	if(tmp_phi){delete [] tmp_phi; tmp_phi = 0;}
	if(tmp_state){delete [] tmp_state; tmp_state = 0;}
	if(tmp_voro){delete [] tmp_voro; tmp_voro = 0;}
}

template<class T> bool topology_analysis0(T * inimg1d, vector<MyMarker*> & inmarkers, int sz0, int sz1, int sz2, int cnn_type = 2.0, int line_compare_method = 1, double line_compare_thresh = 2.0)
{
	vector<NeuronSegment*> neuron_segs = swc_to_neuron_segment(inmarkers);
	int n = neuron_segs.size(); cout<<"segment number = "<<n<<endl;

	long tol_sz = (long)sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;

	map<long, double> score_map; for(int m = 0; m < inmarkers.size(); m++) score_map[inmarkers[m]->ind(sz0, sz01)] = 0.0;
	double min_score = MAX_DOUBLE, max_score = - MAX_DOUBLE;

	float * phi = new float[tol_sz];

	vector<MyMarker*> all_new_linkers;
	for(int i = 0; i < n; i++)
	{
		cout<<endl<<" segment "<<i<<endl;
		NeuronSegment * seg = neuron_segs[i];
		if(seg->markers.size() <=1 && seg->last()->parent == 0) continue;
		MyMarker * root = (seg->last()->parent == 0) ? seg->last() : seg->last()->parent;
		int tar_sz = (root == seg->last()) ? seg->markers.size() - 1 : seg->markers.size();
		vector<MyMarker> target(tar_sz);  for(int m = 0; m < tar_sz; m++) target[m] = *(seg->markers[m]);

		for(long ind = 0; ind < tol_sz; ind++) phi[ind] = INF;
		vector<MyMarker*> outswc; map<long, MyMarker *> marker_map;
		fastmarching_tracing(*root, target, inimg1d, outswc, phi, sz0, sz1, sz2);
		cout<<endl<<"fastmarching finished"<<endl;
		
		for(int s = 0; s < outswc.size(); s++) marker_map[outswc[s]->ind(sz0, sz01)] = outswc[s];

		MyMarker * swc_root = marker_map[root->ind(sz0, sz01)]; swc_root->parent = 0;
		cout<<"set root parent to 0"<<endl;
		for(int m = 0; m < tar_sz; m++)
		{
			vector<MyMarker * > ori_linker;
			MyMarker * marker = seg->markers[m];
			long marker_ind = marker->ind(sz0, sz01);
			ori_linker.insert(ori_linker.begin(), seg->markers.begin() + m, seg->markers.end());
			if(seg->last() != root) ori_linker.push_back(root);

			vector<MyMarker * > new_linker;
			MyMarker * p = marker_map[marker_ind];
			while(p) {new_linker.push_back(p); p = p->parent;}
			double score = (line_compare_method == 0) ? distance_between_lines0(ori_linker, new_linker) : distance_between_lines1(ori_linker, new_linker, line_compare_thresh) ; cout<<"score = "<<score<<endl;
			min_score = MIN(min_score, score);
			max_score = MAX(max_score, score);
			score_map[marker_ind] = score;
			all_new_linkers.insert(all_new_linkers.end(), new_linker.begin(), new_linker.end());
		}
	}
	max_score -= min_score; if(max_score == 0) max_score = 0.000001;
	for(int m = 0; m < inmarkers.size(); m++)
	{
		MyMarker * marker = inmarkers[m];
		long  marker_ind = marker->ind(sz0, sz01);
		marker->type = (score_map[marker_ind] - min_score)/max_score * 255 + 0.5 + 19;
	}

	string outlinker_file = "topology_linkers.swc";
	saveSWC_file(outlinker_file, all_new_linkers);

	return true;
}

// algorithm flow
// 1. swc to neuron segment
// 2. for each segment, find shortest path from start to end by fast marching
// 3. calculate the overlap rate
// 4. rescale the rate for output
template<class T> bool topology_analysis1(T * inimg1d, vector<MyMarker*> & inmarkers, int sz0, int sz1, int sz2, int cnn_type = 2.0, int line_compare_method = 1, double line_compare_thresh = 2.0)
{
	vector<NeuronSegment*> neuron_segs = swc_to_neuron_segment(inmarkers);
	int n = neuron_segs.size(); cout<<"segment number = "<<n<<endl;

	map<MyMarker*, double> score_map; for(int m = 0; m < inmarkers.size(); m++) score_map[inmarkers[m]] = 0.0;
    vector<MyMarker*> all_links;
	double min_score = MAX_DOUBLE, max_score = - MAX_DOUBLE;

	for(int i = 0; i < n; i++)
	{
		cout<<endl<<" segment "<<i<<endl;
		NeuronSegment * seg = neuron_segs[i]; 
		MyMarker * sub_marker = seg->first();
		MyMarker * tar_marker = (seg->last()->parent == 0) ? seg->last() : seg->last()->parent;
		if(sub_marker == tar_marker) continue;
		
		float * phi = 0;
		vector<MyMarker*>  linker;
		fastmarching_linker(*sub_marker, *tar_marker, inimg1d, linker, sz0, sz1, sz2, cnn_type);
		if(phi){delete [] phi; phi = 0;}
		vector<MyMarker*> orig_linker = seg->markers; if(seg->last()->parent) orig_linker.push_back(seg->last()->parent);
		double score = (line_compare_method == 0) ? distance_between_lines0(orig_linker, linker) : distance_between_lines1(orig_linker, linker, line_compare_thresh); cout<<"score = "<<score<<endl; 
		min_score = MIN(min_score, score);
		max_score = MAX(max_score, score);
		for(int m = 0; m < seg->markers.size(); m++) score_map[seg->markers[m]] = score;
		
		//SAVE_TOPOLOGY_ANALYSIS_TEMP_SWC;
        all_links.insert(all_links.end(), linker.begin(), linker.end());
	}

    max_score -= min_score; if(max_score == 0) max_score = 0.000001;
    for(int i = 0; i < inmarkers.size(); i++)
    {
        MyMarker * marker = inmarkers[i];
        marker->type = (score_map[marker] - min_score)/max_score * 255+ 0.5 + 19;
    }

    for(int m = 0; m < all_links.size(); m++) all_links[m]->type = C_RED;
	//for(int m = 0; m < inmarkers.size(); m++) inmarkers[m]->type = C_GREEN;
	//all_links.insert(all_links.end(), inmarkers.begin(), inmarkers.end());
    string outlinker_file = "topology_linkers.swc";
    saveSWC_file(outlinker_file, all_links);

	return true;
}

// algorithm flow
// 1. swc to neuron segment
// 2. for each segment, connect it to its non sibling segments
// 3. calculate the length and signal of the orig path and new path
template<class T> bool topology_analysis2(T * inimg1d, vector<MyMarker*> & inmarkers, int sz0, int sz1, int sz2, int cnn_type = 2)
{
	vector<NeuronSegment*> neuron_segs = swc_to_neuron_segment(inmarkers);
	int n = neuron_segs.size(); cout<<"segment number = "<<n<<endl;

	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;
	T * tmpimg1d = new T[tol_sz]; for(long ind = 0; ind < tol_sz; ind++) tmpimg1d[ind] = inimg1d[ind];
	V3DLONG tmp_sz[4] = {sz0, sz1, sz2, 1}; 
	COMPUTE_MIN_MAX_INT;

	map<MyMarker*, NeuronSegment*> marker_map;
	for(int i = 0; i < n; i++) 
	{
		NeuronSegment * seg = neuron_segs[i];
		for(vector<MyMarker*>::iterator it = seg->markers.begin(); it != seg->markers.end(); it++) marker_map[*it] = seg;
	}

	vector<NeuronSegment*> leaf_segs = leaf_neuron_segments(neuron_segs);
	// test every leaf segment
	for(int i = 0; i < leaf_segs.size(); i++)
	{
		cout<<endl<<" leaf segment "<<i<<endl;
		NeuronSegment * sub_seg = leaf_segs[i];
		NeuronSegment * par_seg = sub_seg->parent;

		vector<MyMarker *> sub_markers = sub_seg->markers;
		vector<MyMarker *> tar_markers, sib_markers; // sibling markers, including parent
		for(int j = 0; j < n; j++)
		{
			if(neuron_segs[j] == sub_seg) continue;
			if(neuron_segs[j] == par_seg || neuron_segs[j]->parent == par_seg) 
			{
				vector<MyMarker*> & jmarkers = neuron_segs[j]->markers;
				sib_markers.insert(sib_markers.end(), jmarkers.begin(), jmarkers.end());
				continue;
			}
			else
			{
				NeuronSegment * jseg = neuron_segs[j];
				vector<MyMarker*> & jmarkers = jseg->markers;
				tar_markers.insert(tar_markers.end(), jmarkers.begin(), jmarkers.end());
				//if(jseg->parent && jseg->parent->parent == par_seg) tar_markers.push_back(jseg->last()->parent);
			}
		}
		if(tar_markers.empty()) continue;

		vector<MyMarker*> all_markers, lnk_markers;
		MASK_SIBLING_AREA(tmpimg1d);
		fastmarching_linker(sub_markers, tar_markers, tmpimg1d, lnk_markers, sz0, sz1, sz2, cnn_type);
		UNMASK_SIBLING_AREA(tmpimg1d);
		vector<MyMarker*> alt1_markers, alt2_markers; // the alternative path (the other is lnk_markers) from sub_marker to tar_marker
		{
			// first part of alt_markers
			MyMarker * sub_marker = (*lnk_markers.rbegin());
			for(int m = 0; m < sub_markers.size(); m++){if((*sub_marker) == (*(sub_markers[m]))) sub_marker = sub_markers[m];}
			MyMarker * pm = sub_marker;

			while(pm != sub_seg->last()->parent) {alt1_markers.push_back(pm); pm = pm->parent;} alt1_markers.push_back(pm);

			// second part of alt_markers
			MyMarker * tar_marker = (*lnk_markers.begin())->parent; assert(marker_map.find(tar_marker) != marker_map.end());
			NeuronSegment * tar_seg = marker_map[tar_marker], * pn = tar_seg;
			bool is_link_to_root_seg = true;
			while(pn) {if(pn->parent && pn->parent->parent == par_seg){is_link_to_root_seg = false; break;} pn = pn->parent;}

			if(is_link_to_root_seg)
			{
				alt2_markers.insert(alt2_markers.begin(), par_seg->markers.begin(), par_seg->markers.end()); alt2_markers.push_back(par_seg->last()->parent);
			}
			else 
			{
				pm = tar_seg->last()->parent; while(pm != sub_seg->last()->parent){alt2_markers.push_back(pm); pm = pm->parent;} alt2_markers.push_back(pm);
			}
		}

		cout<<"sub_markers : "<<sub_markers.size()<<"\ttar_markers : "<<tar_markers.size()<<endl;
		cout<<"sib_markers : "<<sib_markers.size()<<"\tlnk_markers : "<<lnk_markers.size()<<endl;

		double lnk_len = 0.0, lnk_sig = 0.0;
		double alt1_len = 0.0, alt1_sig = 0.0;
		double alt2_len = 0.0, alt2_sig = 0.0;
		double alt_len = 0.0, alt_sig = 0.0;
		COMPUTE_LENGTH_AND_SIGNAL(lnk_len, lnk_sig, lnk_markers);
		COMPUTE_LENGTH_AND_SIGNAL(alt1_len, alt1_sig, alt1_markers);
		COMPUTE_LENGTH_AND_SIGNAL(alt2_len, alt2_sig, alt2_markers);
		cout<<"orig len : "<<lnk_len<<"\tsig : "<<lnk_sig<<endl;
		cout<<"new  len : "<<alt1_len + alt2_len<<"\tsig : "<<alt1_sig + alt2_sig<<endl;

		// color : 1 black, 2 red, 3 blue, 4 pink, 5 light green, 6 yellow, 7 green, 8 brown, 9 light yellow, 10 light pink, 11 light blue
		all_markers.insert(all_markers.end(), tar_markers.begin(), tar_markers.end()); for(int m = 0; m < tar_markers.size(); m++) tar_markers[m]->type = C_GREEN;
		all_markers.insert(all_markers.end(), sib_markers.begin(), sib_markers.end()); for(int m = 0; m < sib_markers.size(); m++) sib_markers[m]->type = C_YELLOW;
		all_markers.insert(all_markers.end(), sub_markers.begin(), sub_markers.end()); for(int m = 0; m < sub_markers.size(); m++) sub_markers[m]->type = C_RED; 
		all_markers.insert(all_markers.end(), lnk_markers.begin(), lnk_markers.end()); for(int m = 0; m < lnk_markers.size(); m++) lnk_markers[m]->type = C_BLUE;
		for(int m = 0; m < alt1_markers.size(); m++) alt1_markers[m]->type = 12;
		for(int m = 0; m < alt2_markers.size(); m++) alt2_markers[m]->type = 12;

		string outswc_file = string("topology_swc" + num2str(i,'0',3) + ".swc");
		saveSWC_file(outswc_file, all_markers);
	}
	if(tmpimg1d){delete [] tmpimg1d; tmpimg1d = 0;}
}

// algorithm flow
// 1. swc to neuron segment
// 2. for each segment, find shortest path from start to end by fast marching
// 3. calculate the hausdorff distance between pathes
template<class T> bool topology_analysis3(T * inimg1d, vector<MyMarker*> & inmarkers, map<MyMarker*, double> & score_map, int sz0, int sz1, int sz2, int cnn_type = 2.0)
{
    vector<NeuronSegment*> neuron_segs = swc_to_neuron_segment(inmarkers);
    int n = neuron_segs.size(); cout<<"segment number = "<<n<<endl;

    score_map.clear();
    for(int m = 0; m < inmarkers.size(); m++) score_map[inmarkers[m]] = 0.0;
    for(int i = 0; i < n; i++)
    {
        cout<<" segment "<<i<<": ";
        NeuronSegment * seg = neuron_segs[i];
        MyMarker * sub_marker = seg->first();
        MyMarker * tar_marker = (seg->last()->parent == 0) ? seg->last() : seg->last()->parent;
        double score = 0;
        if(sub_marker != tar_marker){
            vector<MyMarker*>  linker;
            fastmarching_linker(*sub_marker, *tar_marker, inimg1d, linker, sz0, sz1, sz2, cnn_type);
            vector<MyMarker*> orig_linker = seg->markers; if(seg->last()->parent) orig_linker.push_back(seg->last()->parent);
            score = distance_between_lines2(orig_linker, linker);
        }
        cout<<"score = "<<score<<endl;
        for(int m = 0; m < seg->markers.size(); m++) score_map[seg->markers[m]] = score;
    }
    return true;
}


// algorithm flow
// 1. swc to neuron segment
// 2. for each segment, find shortest path from start to end by fast marching after perturbation
// 3. calculate the intensity score by comparing original segment and new segment
// score type: 0:scored by node; 1:scored by segment
template<class T> bool topology_analysis_perturb_intense(T * inimg1d, vector<MyMarker*> & inmarkers, map<MyMarker*, double> & score_map, double radius_factor, int sz0, int sz1, int sz2, int scoreType = 0, int cnn_type = 2.0)
{
    vector<NeuronSegment*> neuron_segs = swc_to_neuron_segment(inmarkers);
    int n = neuron_segs.size(); cout<<"segment number = "<<n<<endl;

    T * tmp_img1d = new T [sz0*sz1*sz2];
    memcpy(tmp_img1d,inimg1d,sz0*sz1*sz2*sizeof(T));

    score_map.clear();
    for(int m = 0; m < inmarkers.size(); m++) score_map[inmarkers[m]] = 0.0;
    for(int i = 0; i < n; i++)
    {
        cout<<" segment "<<i<<": ";
        vector<double> score;
        NeuronSegment * seg = neuron_segs[i];
        MyMarker * sub_marker = seg->first();
        MyMarker * tar_marker = (seg->last()->parent == 0) ? seg->last() : seg->last()->parent;
        if(sub_marker != tar_marker){
            vector<MyMarker*> orig_linker = seg->markers; if(seg->last()->parent) orig_linker.push_back(seg->last()->parent);
            vector<MyMarker*> new_linker;
            perturb_linker(orig_linker, inimg1d, new_linker, radius_factor, sz0, sz1, sz2, tmp_img1d, cnn_type);
            score = intensScore_between_linkers(orig_linker, new_linker, inimg1d, sz0, sz1, sz2, scoreType);

            cout<<";\tscore sample = "<<score[0]<<" : "<<score.back()<<endl;
            for(int m = 0; m < seg->markers.size(); m++) score_map[seg->markers[m]] = score[m];
        }else{
            cout<<"single node segment"<<endl;
            score_map[sub_marker] = 1;
        }
    }
    return true;
}

template<class T> bool topology_analysis(int method, T * inimg1d, vector<MyMarker*> & inmarkers, int sz0, int sz1, int sz2, int cnn_type = 2.0, int line_compare_method = 1, double line_compare_thresh = 2.0)
{
	if(method == 0) return topology_analysis0(inimg1d, inmarkers, sz0, sz1, sz2, cnn_type, line_compare_method, line_compare_thresh);
	if(method == 1) return topology_analysis1(inimg1d, inmarkers, sz0, sz1, sz2, cnn_type, line_compare_method, line_compare_thresh);
    if(method == 2) return topology_analysis2(inimg1d, inmarkers, sz0, sz1, sz2, cnn_type);
}

#undef COMPUTE_LENGTH_AND_SIGNAL
#undef COMPUTE_MASKED_IMAGE
#undef COMPUTE_MIN_MAX_INT

#endif
