// q_pointcloud_match_basic.h
// provide some basic structure and functions
// by Lei Qu
// 2009-06-24
// seperated from q_pointcloud_match.h by Lei Qu on 2010-01-22

#ifndef __Q_POINTCLOUD_MATCH_BASIC_H__
#define __Q_POINTCLOUD_MATCH_BASIC_H__

#include <stdio.h>
#include <vector>
using namespace std;

#define WANT_STREAM
#include "../jba/newmat11/newmatap.h"
#include "../jba/newmat11/newmatio.h"

class Coord3D_PCM
{
public:
	double x,y,z;
	Coord3D_PCM(double x0,double y0,double z0) {x=x0;y=y0;z=z0;}
	Coord3D_PCM() {x=y=z=0;}
};


//sort 1d array from low to high
bool q_sort1d(vector<double> &vec,vector<int> &vec_index);
//sort 2d array from low to high
bool q_sort2d(vector< vector<double> > &vecvec,vector< vector<int> > &vecvec_index,bool b_sort_every_row,bool b_low2high);

//centrilize and scale the point set
bool q_normalize_points(const vector<Coord3D_PCM> vec_input,vector<Coord3D_PCM> &vec_output,Matrix &x4x4_normalize);

//compute and sort the distant context information
bool q_compute_dismatrix(const vector<Coord3D_PCM> vec_input,vector< vector<double> > &vecvec_dismatrix);
bool q_compute_dismatrix_euclidian(const vector<Coord3D_PCM> vec_input,vector< vector<double> > &vecvec_dismatrix);
bool q_compute_dismatrix_geodesic(const vector<Coord3D_PCM> vec_input,const int K,vector< vector<double> > &vecvec_dismatrix);

//sort similarity matrix and voting the best several canditates with given weight
bool q_vote_simmatrix(const vector< vector<double> > &vecvec_simmatrix,const int weight,vector< vector<double> > &vecvec_votematrix);


//kill all multi-matched pairs (although one of they may be right!)
bool q_kill_allmultimatch(vector<int> &arr1d_A2Bindex,const int n_lengthB);
//kill multi-match by only keep one pair that has the highest match cost (for affine based refine match)
bool q_kill_badmultimatch_keephigestcost(vector<int> &arr1d_A2Bindex,const vector<double> &vec_A2B_cost,const int n_lengthB);
//kill multi-match by only keep one pair that has the lowest match cost (for manifold based refine match)
bool q_killbadmultimatch_keeplowestcost(vector<int> &vec_A2Bindex,const vector<double> &vec_A2Bcost,const int n_lengthB);


//output the matched pair to swc format
bool q_export_matches2swc(const vector<Coord3D_PCM> &vec_1,const vector<Coord3D_PCM> &vec_2,const vector<int> &vec_1to2index,const char *filename);
//output the matched pair to apo format (only points corrdinate, no matching information)
bool q_export_matches2apo(const vector<Coord3D_PCM> &vec_1,const vector<Coord3D_PCM> &vec_2,const vector<Coord3D_PCM> &vec_2_invp,const char *filename,const int output_option);
//output the matched pair to marker format (only points corrdinate, no matching information)
bool q_export_matches2marker(const vector<Coord3D_PCM> &vec_1,const vector<Coord3D_PCM> &vec_2,const vector<Coord3D_PCM> &vec_2_invp,const char *filename,const int output_option);
//output only valid markers to marker file(output_flag=1: output marker in vec_1; output_flag=2: output marker in vec_2)
bool q_export_validmarker(const int output_flag,const vector<Coord3D_PCM> &vec,const vector<int> &vec_1to2index,const char *filename);


#endif

