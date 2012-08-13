//q_principalskeleton_detection.h
//deform the input initial principal skeleton based on given image
//by Lei Qu
//2009-11-17

#ifndef __Q_PRINCIPALSKELETON_DETECTION__
#define __Q_PRINCIPALSKELETON_DETECTION__


#include <stdio.h>
#include <vector>
using namespace std;

#include "../../basic_c_fun/basic_surf_objs.h"
 

class point3D64F
{
public:
	double x,y,z;
	point3D64F() {x=y=z=0.0;}
	point3D64F(double x0,double y0,double z0) {x=x0;y=y0;z=z0;}
};

struct PSDParas
{
	//input
	int 		n_index_channel;
	double 		d_inizoomfactor_skeleton;
	long		l_maxitertimes;
	double 		d_stopiter_threshold;
	double 		d_foreground_treshold;//times of image mean value
	long 		l_diskradius_openning;
	long 		l_diskradius_closing;
	bool		b_removeboundaryartifact;//remove noise tissue in the boundary of image
	int			i_baseimage_methhod;//deform skeleton on (0: original image, 1: mask image)
	//output
};

//given 2D gray image and initial skeleton (position, domain+weight), output deformed principal skeleton
//Processing flow:
//(1). parse the topology of skeleton from input domain file (extract neighbor index and weight, skeleton end points ...)
//(4). extract foreground region from input image
//(5). perform morphology operation(closing+opening) on the foreground image (fill small hole and remove small object)
//(6). generate base image(on which we deform the principal skeleton). [method 1:base=foregound, method 2:base=mask+bone]
//(7). compute voronoi region of each control point, the external term and internal term, update the skeleton iteratively
//(8). stop iteration when the total shift of skeleton less than the given threshold (0.01)
//
//Parameters:
//p_img_input:			input 2D gray image pointer
//sz_img_input:			input 2D gray image size array pointer, [width, height]
//vec_cptpos_input:		input control points position array
//vecvec_domain_length_ind: input domain definition for length constraint (index of control points in every domain)
//vec_domain_length_weight: input domain weight definition for length constraint
//paras_input:			input parameters for principal skeleton detection
//vec_cptpos_output:	output deformed control points position array
//
bool q_principalskeleton_detection(
		const unsigned char *p_img_input,const long sz_img_input[2],
		const vector<point3D64F> &vec_cptpos_input,
		const vector< vector<long> > &vecvec_domain_length_ind,const vector<double> &vec_domain_length_weight,
		const vector< vector<long> > &vecvec_domain_smooth_ind,const vector<double> &vec_domain_smooth_weight,
		const PSDParas &paras_input,
		vector<point3D64F> &vec_cptpos_output);


#endif
