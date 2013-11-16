/*
 *  SimpleTracing: NEURON_TRACING.h
 *
 *  Created by Yang, Jinzhu on 20/5/11.
 *  Last update: by PHC, 20110826
 *  Last update: by PHC, 2012-1-25
 *
 */

#ifndef __SIMPLE_NEURON_TRACING__H__
#define __SIMPLE_NEURON_TRACING__H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include "v3d_interface.h"
#include <vector>
using namespace std;

//#define BACKGROUND -1 //
#define LINETAG -100 // 
//#define DISOFENDS 100  //

// 
typedef struct tagSpacePoint_t
{
	V3DLONG m_x;
	V3DLONG m_y;
	V3DLONG m_z;
	double r;
	V3DLONG sd;
	V3DLONG mark;
	V3DLONG max_ray;
	int ray_value[26];
	int ray_number[26];
	int max_direct;
	V3DLONG node_id;
	V3DLONG seg_id;
} SpacePoint_t;

// 
typedef struct tagDFBPoint_t
{
	V3DLONG m_x;
	V3DLONG m_y;
	V3DLONG m_z;
	V3DLONG m_d;
} DFBPoint_t;

//
typedef struct tagDFSPoint_t
{
	V3DLONG m_x;
	V3DLONG m_y;
	V3DLONG m_z;
	V3DLONG m_l; 
} DFSPoint_t;

class NeuronPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);

public:	
	float getPluginVersion() const {return 0.9f;}
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
	
	QStringList funclist() const ;
	
	bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback, QWidget * parent);

public:
	
	void SetImageInfo1D(V3DLONG* data, V3DLONG count, V3DLONG width, V3DLONG height);

	void Initialize1D();

	void Set_Seed(SpacePoint_t seed);
	
	void SetEndPoints(vector<SpacePoint_t> points);

		
	void SearchEndPoints();
	
	NeuronTree NeuronTracing_Rollerball_SWC(unsigned char *apsInput, V3DLONG sx, V3DLONG sy, V3DLONG sz,V3DLONG mx ,V3DLONG my, V3DLONG mz);
	NeuronTree NeuronTracing_Ray_SWC(unsigned char *apsInput, V3DLONG sx, V3DLONG sy, V3DLONG sz,V3DLONG mx ,V3DLONG my, V3DLONG mz);
	
	void NeuronTracing_Ray_cast(V3DPluginCallback &callback, QWidget *parent);
	
	void NeuronTracing_Ray_D(V3DPluginCallback &callback, QWidget *parent);
	
	void Dilation3D(unsigned char *apsInput, unsigned char *aspOutput, V3DLONG sz, V3DLONG sy, V3DLONG sx);
	
	void ExtractSingleCenterpath(DFSPoint_t endP);
	vector<SpacePoint_t> ExtractCenterpath_Ray_Cast(SpacePoint_t points);
	vector<SpacePoint_t> ExtractCenterpath_Ray_D(SpacePoint_t Cpoints,SpacePoint_t NSpoint);
	
	void NueronTree_Ray_D(SpacePoint_t point,int n);
	void NueronTree2(SpacePoint_t point,int n);
	
	void SaveSwcTree_cast(vector<SpacePoint_t> centerpath,V3DLONG pn_id,V3DLONG node_count);

	SpacePoint_t Coumpere_ball_center(SpacePoint_t points, int direct, double r);
	SpacePoint_t Coumpere_Next_Point(SpacePoint_t points, int direct, double r);
	
	vector<SpacePoint_t> Rivise_centerpath(vector<SpacePoint_t> centerpath);
	
	SpacePoint_t Rivise_point(SpacePoint_t Rpoint);
	
	SpacePoint_t Center_mass(SpacePoint_t point);
	
	bool PathMask(vector<SpacePoint_t> centerpath);
	
	SpacePoint_t Raycasting2(SpacePoint_t startpoint);
	
	SpacePoint_t GetCenterPiont(SpacePoint_t startpoint,int direction);
	
	int Getmaxderection(SpacePoint_t startpoint);

	void NeuronSeg(V3DPluginCallback &callback, QWidget *parent);
	
	vector<SpacePoint_t> BranchDetect2(vector<SpacePoint_t> centerpath);

	
	double fitRadiusPercent(V3DLONG **img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2, double imgTH, double bound_r,
							float x, float y, float z, float zthickness, bool b_est_in_xyplaneonly);
	
	
	double fitRadiusPercent_cast(V3DLONG **img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2, double imgTH, double bound_r,
								 float x, float y, float z, float zthickness, bool b_est_in_xyplaneonly);
	
	void Set_local_DFB();
	
	void Set_local_DFS();
	
	void Clear();
	void GetOrgPoint(short *apsInput, V3DLONG iImgLayer, V3DLONG iImgHei, V3DLONG iImgWid, V3DLONG cnt, SpacePoint_t &pt);

	//entry point functions
	bool Tracing_DistanceField_entry_func(const V3DPluginArgList & input, V3DPluginArgList & output);
	void Tracing_DistanceField_entry_func(V3DPluginCallback &callback, QWidget *parent);
	template <class T> NeuronTree NeuronTracing_Distance_SWC(T *apsInput, 
                                                             V3DLONG sx, V3DLONG sy, V3DLONG sz, V3DLONG sc, 
                                                             V3DLONG ch_tracing, V3DLONG mx ,V3DLONG my, V3DLONG mz);
	
	bool Tracing_Ray_SWC(const V3DPluginArgList & input, V3DPluginArgList & output);
	
	bool Tracing_Ball_SWC(const V3DPluginArgList & input, V3DPluginArgList & output);
	
	bool Neuron_Seg(const V3DPluginArgList & input, V3DPluginArgList & output);
	
	void DFS(bool** matrix, V3DLONG* neworder, V3DLONG node, V3DLONG* id, V3DLONG siz, int* numbered, int* group);
	
	bool SortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result, V3DLONG newrootid, double thres);
	
	double computeDist2(const NeuronSWC & s1, const NeuronSWC & s2);
	
	QHash<V3DLONG, V3DLONG> ChildParent(QList<NeuronSWC> &neurons, const QList<V3DLONG> & idlist, const QHash<V3DLONG,V3DLONG> & LUT);
   
	QHash<V3DLONG, V3DLONG> getUniqueLUT(QList<NeuronSWC> &neurons);
	
	bool Output_SWC(QList<NeuronSWC> & neurons,  NeuronTree & result);
	
public:
	
	SpacePoint_t                  m_sptSeed;
	
	NeuronTree                    SS;
	QList <NeuronSWC>             listNeuron;
	
	V3DLONG num ;
	
	V3DLONG N_first ;
	
//	SpacePoint_t                  direc_point;
	
	vector<DFSPoint_t>            m_vdfsptEndPoint;
	
	vector<DFBPoint_t>            m_vdfbptSurface;
	
	vector<SpacePoint_t>          m_vsptCenterpath;
	
	vector<SpacePoint_t>          m_Markerpoint;
	
	vector<vector<SpacePoint_t> > m_vvsptCenterpaths;
	
	vector<vector<SpacePoint_t> > m_StatisticalPiont; //
	
	vector<vector<SpacePoint_t> > m_GaussFilterPiont;
	
	vector<vector<SpacePoint_t> > m_DetectionPiont;
	
	vector<vector<SpacePoint_t> > centerpathall;
	
	vector<vector<SpacePoint_t> > centerpathall1;
	int count;
	
	//unsigned char**						  m_ppsImgData;
	V3DLONG**                             m_ppsImgData;
	unsigned char *					      m_ppsMaskData;
	
	unsigned char *					      m_ppsMaskBranch;
	
	V3DLONG*			              m_ppsOriData1D;
	
	V3DLONG*	                        m_psTemp;
	
	V3DLONG                           m_iImgWidth;
	
	V3DLONG                           m_iImgHeight;
	
	V3DLONG                           m_OiImgWidth;
	
	V3DLONG                           m_OiImgHeight;
	
	V3DLONG                           m_iImgSize;
	
	V3DLONG                           m_iImgCount;
	
	V3DLONG                          m_ulVolumeSize;
	
	V3DLONG                            m_iMinLength;
	
	V3DLONG*                               m_piDFB;
	V3DLONG*                               m_piDFS;
	V3DLONG*                               m_piMskDFS;
	
	QList <ImageMarker> listMarker;
		
};

class NeuronDialog : public QDialog
{
	Q_OBJECT
	
public:
	QGridLayout *gridLayout;
	
	QLabel *labelx;
	QLabel *labely;
    QSpinBox* Ddistance; 
	QSpinBox* Dnumber;
	
	QPushButton* ok;
	QPushButton* cancel;
	
	V3DLONG Dn;
	V3DLONG Dh;
	
public:
	NeuronDialog(V3DPluginCallback &cb, QWidget *parent)
	{
		Image4DSimple* image = cb.getImage(cb.currentImageWindow());
		QString imageName = cb.getImageName(cb.currentImageWindow());		
		//create a dialog
		Ddistance= new QSpinBox();
		Dnumber = new QSpinBox();
	
		
		Dnumber->setMaximum(255); Dnumber->setMinimum(1); Dnumber->setValue(3);
		Ddistance->setMaximum(255); Ddistance->setMinimum(1); Ddistance->setValue(5);
		
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		gridLayout = new QGridLayout();
		
		labelx = new QLabel(QObject::tr("sampling interval"));
		labely = new QLabel(QObject::tr("number of sampling points"));
		
		gridLayout->addWidget(labelx, 0,0); gridLayout->addWidget(Ddistance, 0,1);
		gridLayout->addWidget(labely, 1,0); gridLayout->addWidget(Dnumber, 1,1);
		
		gridLayout->addWidget(cancel, 6,1); gridLayout->addWidget(ok, 6,0);
		setLayout(gridLayout);
		setWindowTitle(QString("Change parameters"));
		
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		
		//slot interface
		connect(Ddistance, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(Dnumber,SIGNAL(valueChanged(int)), this, SLOT(update()));
	}
	
	~NeuronDialog(){}
	
	public slots:
	void update();
};


//interface to the neuron foreground extraction
bool do_seg(short *pData, V3DLONG sx, V3DLONG sy, V3DLONG sz, int & iVesCnt, bool b_binarization);
void Neuron_segment_entry_func(V3DPluginCallback &callback, QWidget *parent);
void Neuron_segment_entry_func(const V3DPluginArgList & input, V3DPluginArgList & output, bool b_binarization);



#endif

