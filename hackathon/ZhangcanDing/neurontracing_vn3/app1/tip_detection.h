/*
 *  TIP_DETECTION.h
 *
 *  Extensively revised from Yang, Jinzhu's plugin by Hanchuan Peng, 2010-Dec-20
 *
 */

#ifndef __TIP_DETECTION__H__
#define __TIP_DETECTION__H__

#include <vector>
using namespace std;

#include "v3d_basicdatatype.h"


//空间坐标点DFS编码
typedef struct tagDFSPoint_t  //DFS is "distance from source"
{
	V3DLONG m_x;
	V3DLONG m_y;
	V3DLONG m_z;
	double m_l; 
} DPoint_t;

class ImageTip
{
public:
	bool Initialize(unsigned char* data, V3DLONG sz[3]);
	void Set_DFS_Seed(const DPoint_t & seed);
	void Compute_DFS();
	vector<DPoint_t> Search_End_Points();
	void Clear();
	~ImageTip() {Clear();}
	
public:
	DPoint_t					m_sptSeed;
	vector<DPoint_t>            m_vdfsptEndPoint;
	vector<DPoint_t>            m_vdfbptSurface;
	
	unsigned char*				m_ppsOriData1D;
	V3DLONG                     m_szx, m_szy, m_szz; 

    unsigned char*				m_ppsImgData; 
	float*                      m_piDFS;
	V3DLONG						realszx, realszy, realszz, totallen;
};

vector<DPoint_t> tip_detection(unsigned char *p, V3DLONG sz[3]);


#endif


