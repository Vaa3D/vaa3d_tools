// q_atlasguided_seganno.h
// by Lei Qu
// 2010-07-30

#ifndef __Q_ATLASGUIDED_SEGANNO_H__
#define __Q_ATLASGUIDED_SEGANNO_H__

#include <QtGui>
#include <vector>
using namespace std;

#define WANT_STREAM
#include "newmatap.h"
#include "newmatio.h"

#include "basic_surf_objs.h"
#include "v3d_interface.h"

class point3D64F
{
public:
	double x,y,z;
	point3D64F() {x=y=z=0.0;}
	point3D64F(double x0,double y0,double z0) {x=x0;y=y0;z=z0;}
};

class CParas
{
public:
	//file i/o
	bool 	b_imgfromV3D;
	//initial align
	bool 	b_atlasupsidedown;
	long 	l_refchannel;
	double 	d_downsampleratio;
	double  d_fgthresh_factor;
	double 	d_T,d_T_min,d_annealingrate;
	long 	l_niter_pertemp;
	//refine align
	bool 	b_ref_simplealign;
	double 	d_ref_T;								//fixed temperature for refined align
	long 	l_ref_cellradius;						//cell radius (after downsample) used to compute the energy
	long 	l_ref_maxiter;
	double 	d_ref_minposchange;
	//visualize
	bool 	b_showatlas;
	bool 	b_showsegmentation;
	bool	b_stepwise;
	//work mode
	long 	l_mode;
	QWidget *qw_rootparent;

	CParas()
	{
		b_imgfromV3D=0;
		b_atlasupsidedown=0;
		l_refchannel=1;
		d_downsampleratio=4;
		d_fgthresh_factor=3;
		d_T=5;d_T_min=0.2;d_annealingrate=0.95;
		l_niter_pertemp=1;
		b_ref_simplealign=1;
		d_ref_T=0.1;
		l_ref_cellradius=2;
		b_showatlas=0;
		b_showsegmentation=0;
		l_mode=0;
		qw_rootparent=0;
	}
};

class COutputInfo
{
public:
	bool b_rotate90;							//deformed atlas exist wrongly 90 degree rotation
	bool b_flip;

	COutputInfo()
	{
		b_rotate90=0;
		b_flip=0;
	}
};

class CControlPanel : public QDialog
{
	Q_OBJECT

public:
	CControlPanel(V3DPluginCallback &_v3d, QWidget *parent=0);
	~CControlPanel();

public:
	static CControlPanel* panel;
	V3DPluginCallback &v3d;
	bool b_stop;

	void setstop();

private slots:
	void _slot_stop();
	void _slot_resume();
	signals:
	void _signal_stop();
};


//warp the atlas onto the image
//(currently only aling the muscle cells)
bool q_atlas2image(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const long sz_img[4],const QList<CellAPO> &ql_atlasapo,const QList<QString> &ql_celloi,
		QList<CellAPO> &ql_musclecell_output,unsigned char *&p_img8u_seglabel,COutputInfo &outputinfo);

bool q_atlas2image_partial(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const long sz_img[4],const QList<CellAPO> &ql_atlasapo,const QList<QString> &ql_celloi,
		QList<CellAPO> &ql_musclecell_output,COutputInfo &outputinfo);

bool q_atlas2image_prior(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const long sz_img[4],const QList<CellAPO> &ql_atlasapo,const LandmarkList &ml_makers,
		QList<CellAPO> &ql_musclecell_output);


//affine warp the atlas onto image by deterministic annealing
bool q_atlas2image_musclecell_ini_affine(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const long sz_img[4],const vector<point3D64F> &vec_musclecell,
		const vector<long> &vec_fg_ind,const vector<point3D64F> &vec_fg_xyz,
		vector<point3D64F> &vec_musclecell_output,vector< vector<double> > &vec2d_labelprob,COutputInfo &outputinfo);

//refine the warped atlas (deterministic annealing without constraint)
bool q_atlas2image_musclecell_ref_DAWC(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const long sz_img[4],const vector<point3D64F> &vec_musclecell,
		const vector<long> &vec_fg_ind,const vector<point3D64F> &vec_fg_xyz,
		vector<point3D64F> &vec_musclecell_output);
bool q_atlas2image_musclecell_ref_DAWC_prior(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const long sz_img[4],const vector<point3D64F> &vec_musclecell,const vector<long> vec_ind_anchor,
		const vector<long> &vec_fg_ind,const vector<point3D64F> &vec_fg_xyz,
		vector<point3D64F> &vec_musclecell_output);

//refine the warped atlas (deterministic annealing without TPS constraint)
bool q_atlas2image_musclecell_ref_DATPS(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const long sz_img[4],const vector<point3D64F> &vec_musclecell,
		const vector<long> &vec_fg_ind,const vector<point3D64F> &vec_fg_xyz,
		vector<point3D64F> &vec_musclecell_output);

//refine the warped atlas (LHHV: locally horizental halfway vibration)
bool q_atlas2image_musclecell_ref_LHHV(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const long sz_img[4],const vector<point3D64F> &vec_musclecell,
		const vector<long> &vec_fg_ind,const vector<point3D64F> &vec_fg_xyz,
		vector<point3D64F> &vec_musclecell_output);

//refine the position of atlas points by mean-shift
bool q_atlas2image_musclecell_ref_mshift(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const long sz_img[4],const vector<point3D64F> &vec_musclecell,
		vector<point3D64F> &vec_musclecell_output);
bool q_atlas2image_musclecell_ref_mshift_adaptive(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const long sz_img[4],const vector<point3D64F> &vec_musclecell,const double d_thresh_fg,
		vector<point3D64F> &vec_musclecell_output);

//group the cells according to the given distance (used to estimate cell num and post-process the clustered cells)
bool q_groupcells(const vector<point3D64F> &vec_cell,const double d_mingroupdis,
		vector< vector<long> > &vec2d_groupind);

//post process the anno result (backward push the overlapped anno)
bool q_postprocess_backwardpushanno(const QList<CellAPO> &ql_musclecell_input,
		QList<CellAPO> &ql_musclecell_output);

//compute the energy for given deformed atlas (energy=ocupied pixel intensity sum)
bool q_compute_energy(const unsigned char *p_img8u,const long sz_img[4],const vector<point3D64F> &vec_musclecell,
		const vector<long> &vec_fg_ind,const vector<point3D64F> &vec_fg_xyz,
		const long &l_radius,
		double &d_energy);

//show the point set in current 3D viewer window
bool q_push2V3D_pts(V3DPluginCallback &callback,vector<point3D64F> &vec_pts,long l_makradius);


//------------------------------------------------------------------------------------------------------------------------------------
Matrix q_pseudoinverse(Matrix inputMat);

//normalize 3D point cloud
bool q_normalize_points_3D(const vector<point3D64F> &vec_input,vector<point3D64F> &vec_output,Matrix &x4x4_normalize);
//compute the rigid(actually is similar) transform matrix B=T*A
bool q_compute_rigidmatrix_3D(const vector<point3D64F> &vec_A,const vector<point3D64F> &vec_B,Matrix &x4x4_rigidmatrix);
//compute the affine transform matrix B=T*A
bool q_compute_affinmatrix_3D(const vector<point3D64F> &vec_A,const vector<point3D64F> &vec_B,Matrix &x4x4_affinematrix);

//compute the affine term d and nonaffine term c which decomposed from TPS (tar=sub*d+K*c)
//more stable compare to the q_affine_compute_affinmatrix_3D()
bool q_TPS_cd(const vector<point3D64F> &vec_sub,const vector<point3D64F> &vec_tar,const double d_lamda,
		Matrix &x4x4_d,Matrix &xnx4_c,Matrix &xnxn_K);
//generate TPS kernel matrix
bool q_TPS_k(const vector<point3D64F> &vec_sub,const vector<point3D64F> &vec_basis,Matrix &xmxn_K);

//compute the sub2tar warped postion for give subject point based on given sub2tar tps warping parameters
bool q_compute_ptwarped_from_tpspara_3D(
		const point3D64F &pt_sub,const vector <point3D64F> &vec_ctlpts_sub,const Matrix &wa_sub2tar,
		point3D64F &pt_sub2tar);
//compute sub2tar tps warp parameters based on given subject and target control points
//tps_para_wa=[w;a]
bool q_compute_tps_paras_3D(
		const vector <point3D64F> &vec_ctlpts_sub,const vector <point3D64F> &vec_ctlpts_tar,
		Matrix &wa);

//------------------------------------------------------------------------------------------------------------------------------------
bool q_align_dapicells(const CParas &paras,V3DPluginCallback &callback,
		const unsigned char *p_img8u,const long sz_img[4],const QList<CellAPO> &ql_atlasapo,const QList<QString> &ql_celloi,
		QList<CellAPO> &ql_musclecell_output,COutputInfo &outputinfo);

#endif
