// q_celegans_straighten.h
// by Lei Qu
// 2010-08-11

#ifndef __Q_CELEGANS_STRAIGHTEN_H__
#define __Q_CELEGANS_STRAIGHTEN_H__

#include <vector>
using namespace std;

#include "v3d_interface.h"

class CSParas
{
public:
	//file i/o
	bool 	b_imgfromV3D;
	bool 	b_markerfromV3D;
	//skeleton detection
	long 	l_refchannel;
	double 	d_downsampleratio;
	long 	l_ctlpts_num;
	long	l_radius_openclose;
	double	d_fgthresh_xy;
	double	d_fgthresh_xz;
	//straighten
	long 	l_radius_cuttingplane;
	//visualize
	bool 	b_showinV3D_pts;

	CSParas()
	{
		b_imgfromV3D=1;
		b_markerfromV3D=1;
		l_refchannel=1;
		d_downsampleratio=4;
		l_ctlpts_num=10;
		l_radius_openclose=0;
		d_fgthresh_xy=3;
		d_fgthresh_xz=0;
		l_radius_cuttingplane=80;
		b_showinV3D_pts=0;
	}
};



//simply straighten along the given markers which defined along the central line of the worm
bool q_celegans_straighten_manual(V3DPluginCallback &callback,const CSParas &paras,
		const unsigned char *p_img_input,const long sz_img_input[4],
		const vector< vector<double> > vec2d_markers,
		unsigned char *&p_img_output,long *&sz_img_output);


bool q_celegans_straighten(V3DPluginCallback &callback,const CSParas &paras,
		const unsigned char *p_img_input,const long sz_img_input[4],
		const vector< vector<double> > vec2d_markers,
		unsigned char *&p_img_output,long *&sz_img_output);


bool q_celegans_restacking_xy(
		const unsigned char *p_inputimg,const long *sz_inputimg,
		const QList<ImageMarker> &ql_marker,const long l_width,
		unsigned char *&p_strimg,long *&sz_strimg);
bool q_celegans_restacking_xy(
		const unsigned char *p_inputimg,const long *sz_inputimg,
		const QList<ImageMarker> &ql_marker,const long l_width,
		unsigned char *&p_strimg,long *&sz_strimg,
		vector< vector< vector< vector<long> > > > &vec4d_mappingfield_str2ori);
bool q_celegans_restacking_xz(
		const unsigned char *p_inputimg,const long *sz_inputimg,
		const QList<ImageMarker> &ql_marker,const long l_width,
		unsigned char *&p_strimg,long *&sz_strimg);
bool q_celegans_restacking_xz(
		const unsigned char *p_inputimg,const long *sz_inputimg,
		const QList<ImageMarker> &ql_marker,const long l_width,
		unsigned char *&p_strimg,long *&sz_strimg,
		vector< vector< vector< vector<long> > > > &vec4d_mappingfield_str2ori);

bool q_restacking_alongbcurve_xy(
		const unsigned char *p_inputimg,const long *sz_inputimg,
        const double *pos_curve_x,const double *pos_curve_y,const double *alpha_curve,const long length_curve,
		const long l_width,
		unsigned char *&p_strimg, long *&sz_strimg,
		vector< vector< vector< vector<long> > > > &vec4d_mappingfield_str2ori);
bool q_restacking_alongbcurve_xz(
		const unsigned char *p_inputimg,const long *sz_inputimg,
        const double *pos_curve_x,const double *pos_curve_y,const double *alpha_curve,const long length_curve,
		const long l_width,
		unsigned char *&p_strimg, long *&sz_strimg,
		vector< vector< vector< vector<long> > > > &vec4d_mappingfield_str2ori);

#endif
