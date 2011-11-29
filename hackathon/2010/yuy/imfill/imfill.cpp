/* imfill.cpp
 * 2010-04-02: create this program by Yang Yu
 */

#include <QtGui>

#include <cmath>
#include <stdlib.h>
#include <ctime>

#include <vector>

#include <sstream>
#include <iostream>

#include "imfill.h"

#include "../basic_c_fun/basic_surf_objs.h"
#include "../basic_c_fun/stackutil.h"
#include "../basic_c_fun/volimg_proc.h"
#include "../basic_c_fun/img_definition.h"
#include "../basic_c_fun/basic_landmark.h"

#define INF 1E9
#define PI 3.14159265

//min-heap data structure (a "complete binary tree", Sedgewick) 
class HeapMinSort
{
private: 
	struct HeapNode
	{
		long idx; // Heap Node Index
		float weight; // Heap Node Weight
	}; 
	
private:
	HeapNode* pHeap; 
	long heapIdx;
	long heapSize; 

public:
	HeapMinSort(long totalpts)
	{
		pHeap = new HeapNode[totalpts]; 			
		heapIdx = 0; 
		heapSize = totalpts; 
	}
	~HeapMinSort()
	{
		if(pHeap) delete []pHeap; 
	}
public:
	void insert(long idxex, float weight)
	{
		pHeap[heapIdx].idx = idxex; 
		pHeap[heapIdx].weight = weight; 
		
		long idx = heapIdx; 
		heapIdx++; 
		if( heapIdx > heapSize ) throw std::exception(); 
		
		while(idx != 0)
		{
			// downward to idx's correct location
			long parent_idx = long((idx-1)/2); 
			if( pHeap[idx].weight < pHeap[parent_idx].weight )
			{
				HeapNode tmp = pHeap[idx]; 
				pHeap[idx] = pHeap[parent_idx]; 
				pHeap[parent_idx] = tmp; 
				
				idx = parent_idx; 
			}
			else 
				break; 
		}
	}
	
	bool del(long *idxex, float *weight)
	{
		if( heapIdx == 0 ) return false; // if the heap is empty
		
		*idxex = pHeap[0].idx; 
		*weight = pHeap[0].weight; 
		
		heapIdx --; 
		long idx = heapIdx; 
		if( idx > 0) // if exists at least one node in the heap
		{
			pHeap[0] = pHeap[idx];
			idx = 0; 
			while(1)
			{
				long child_left = 2*idx + 1; 
				long child_right = child_left +1; 
				
				if( child_left >= heapIdx )  
					break; // no children
				else
				{
					long child_idx = child_left; 
					if( child_right < heapIdx ) // at least two children.
						if( pHeap[child_idx].weight > pHeap[child_right].weight ) 
							child_idx = child_right; 
					
					// upward to idx's correct location
					if( pHeap[idx].weight > pHeap[child_idx].weight ) 
					{
						HeapNode tmp = pHeap[idx]; 
						pHeap[idx] = pHeap[child_idx]; 
						pHeap[child_idx] = tmp; 
						idx = child_idx; 
					}
					else 
						break; 
				}
			} 
		}
		
		return true; 
	}
	
	
}; 


//---------------------------------------------------------------------------------------------------------------------------------------------------
//distance transform copied from "../cellseg/FL_bwdist.h"
template <class T> inline T square(const T &x) { return x*x; };

float *dt1d(float *f, long int * label, const long int n)
{
	float *d = new float[n];
	int *v = new int[n];
	float *z = new float[n+1];
    
	int k = 0; //? k=1;
	v[0] = 0;
	z[0] = -INF;
	z[1] = +INF;
	for (int q = 1; q <= n-1; q++) 
	{
		float s  = ((f[q]+square(q))-(f[v[k]]+square(v[k])))/(2*q-2*v[k]);
		while (s <= z[k]) 
		{
			k--;
			s  = ((f[q]+square(q))-(f[v[k]]+square(v[k])))/(2*q-2*v[k]);
		}
		k++;
		v[k] = q;
		z[k] = s;
		z[k+1] = +INF;
	}
	
	k = 0;
	
	for (int q = 0; q <= n-1; q++) {
		while (z[k+1] < q)
			k++;
		d[q] = float(square(q-v[k])) + f[v[k]];
		
		if (f[v[k]]>=INF)
			label[q] = -1;
		else	
			label[q] = v[k];
		
	}
	
	if (v) {delete [] v; v=0;}
	if (z) {delete [] z; z=0;}
	return d;
}

void dt3d(float *data, long int * label, const long int *sz) 
{
	
	long int i,j,k;
	
	long int sz10 = sz[1]*sz[0];
	long int sz20 = sz[2]*sz[0];
	long int sz12 = sz[1]*sz[2];
	long int count;
	long int tmp_k, tmp_j;
	
	long len = std::max(std::max(sz[0], sz[1]),sz[2]);
	long len2 = sz[0]*sz[1]*sz[2];
	
	printf("%ld %ld %ld %ld %ld \n",sz10, sz20, sz12, len, len2);
	
	float *f = new float [len];
	long int *lab1 = new long int [len2];
	long int *lab2 = new long int [len2];
	long int *lab3 = new long int [len2];
	
	
	// transform along the i dimension
	for (k = 0; k<sz[2]; k++)
	{
		tmp_k =  k*sz10;
		
		for (j = 0; j < sz[1]; j++) 
		{
			tmp_j = j*sz[0];
			
			for (i = 0; i < sz[0]; i++) 
			{
				f[i] = *(data + tmp_k + tmp_j + i); 
			}
			
			float *d = dt1d(f, lab1+tmp_k+tmp_j, sz[0]); 
			
			for (i = 0; i < sz[0]; i++) 
			{					
				*(data + tmp_k + tmp_j + i) = d[i];
			}
			
			if (d) {delete [] d; d = 0;}
		}	
	}
	
	// transform along the j dimension
	for (k = 0; k < sz[2]; k++)
	{
		tmp_k =  k*sz10;
		
		for (i = 0; i < sz[0]; i++) 
		{
			
			for (j = 0; j < sz[1]; j++) 
			{
				f[j] = *(data + tmp_k + j*sz[0] + i); 
			}
			
			float *d = dt1d(f, lab2+k*sz10+i*sz[1], sz[1]);
			
			for (j = 0; j < sz[1]; j++) 
			{			
				*(data + tmp_k + j*sz[0] + i) = d[j];			
			}
			
			if (d) {delete [] d; d = 0;}
		}	
	}
	
	
	// transform along the k dimension
	for (j = 0; j<sz[1]; j++)
	{		
		tmp_j = j*sz[0];
		
		for (i = 0; i < sz[0]; i++) 
		{			
			for (k = 0; k < sz[2]; k++) 
			{
				f[k] = *(data + k*sz10 + tmp_j + i); 
			}
			
			float *d = dt1d(f, lab3+j*sz20+i*sz[2], sz[2]);
			
			for (k = 0; k < sz[2]; k++) 
			{
				*(data + k*sz10 + tmp_j + i) = d[k]; 
			}
			
			if (d) {delete [] d; d = 0;}
		}	
	}
	
	// assign pixel index
	long int ii,jj,kk;
	
	for (i = 0; i<sz[0]; i++)
	{		
		for (j = 0; j < sz[1]; j++) 
		{			
			for (k = 0; k < sz[2]; k++) 
			{
				kk = *(lab3+j*sz20+i*sz[2]+k);
				jj = *(lab2+kk*sz10+i*sz[1]+j);
				ii = *(lab1+kk*sz10+jj*sz[0]+i);
				
				*(label + k*sz10+j*sz[0]+i) = kk*sz10 + jj*sz[0] + ii;
			}
		}	
	}
	
	
	if (f) {delete [] f; f = 0;}
	if (lab1) {delete [] lab1; lab1 = 0;}
	if (lab2) {delete [] lab2; lab2 = 0;}
	if (lab3) {delete [] lab3; lab3 = 0;}
	
}

template <class T1, class T2> void dt3d_binary(T1 *indata, T2 *outdata, long int * label, const long int *sz, unsigned char tag) 
{
	long int len = sz[0]*sz[1]*sz[2];
	long int i;
	
	float *tmpdata = new float [len];
	
	for (i=0; i<len; i++)
	{
		
		tmpdata[i] = (float)indata[i];
		
		if (tag ==0)
			if (tmpdata[i]>0)
				tmpdata[i] = 0;
			else
				tmpdata[i] = INF;
		else
			if (tmpdata[i]>0)
				tmpdata[i] = INF;
			else
				tmpdata[i] = 0;
	}
	
	dt3d(tmpdata, label, sz);
	
	for (i=0; i<len; i++)
		outdata[i] = (T2)tmpdata[i];
	
	if (tmpdata) {delete [] tmpdata; tmpdata =0;}
	
}

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(imfill, ImFillPlugin);

void imfilling(V3DPluginCallback &callback, QWidget *parent);

//plugin funcs
const QString title = "Image Fill";
QStringList ImFillPlugin::menulist() const
{
    return QStringList() << tr("Image Fill");
}

void ImFillPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
    if (menu_name == tr("Image Fill"))
    {
    	imfilling(callback, parent);
    }
}

void imfilling(V3DPluginCallback &callback, QWidget *parent)
{
    v3dhandleList win_list = callback.getImageWindowList();
	
	if(win_list.size()<1) 
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}
	
	ImFillDialog dialog(callback, parent);
	if (dialog.exec()!=QDialog::Accepted)
		return;
	
	int start_t = clock(); // record time point
	
	dialog.update();
	
	int i1 = dialog.i1;
	int ch = dialog.ch_rgb; 
	bool mean_thresh = dialog.mean_thresh;
	
	int thresh, range;
	
	if(mean_thresh)
	{
		thresh = dialog.thresh;
		range = dialog.range;
	}
	
	//qDebug() << " test... " << thresh << range;
	
	bool use_marker = true;
	
	Image4DSimple* subject = callback.getImage(win_list[i1]);
	ROIList pRoiList=callback.getROI(win_list[i1]);
	
	QString m_InputFileName = callback.getImageName(win_list[i1]);
	
	if (!subject)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}
	if (subject->getDatatype()!=V3D_UINT8)
	{
		QMessageBox::information(0, title, QObject::tr("This demo program only supports 8-bit data. Your current image data type is not supported."));
		return;
	}
	
	
	//also get the landmark from the subject
	LandmarkList list_landmark_sub=callback.getLandmark(win_list[i1]);
	if(list_landmark_sub.size()<1)
	{
		use_marker = false;
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setText("The marker has been choosen.");
		msgBox.setInformativeText("Do you want to use your first marker as a seed to tell image background?");
		msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Ok);
		int ret = msgBox.exec();
		
		switch (ret) {
			case QMessageBox::Ok:
				// Ok was clicked
				break;

			case QMessageBox::Cancel:
				// Cancel was clicked
				return;
				break;
			default:
				// should never be reached
				break;
		}
	}

	
    unsigned char* pSbject = subject->getRawData();
	
	long sz0 = subject->getXDim();
    long sz1 = subject->getYDim();
    long sz2 = subject->getZDim();
	long sz3 = subject->getCDim();
	
	long pagesz_sub = sz0*sz1*sz2;
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------
	//finding the bounding box of ROI
	bool vxy=true,vyz=true,vzx=true; // 3 2d-views
	
	QRect b_xy = pRoiList.at(0).boundingRect();
	QRect b_yz = pRoiList.at(1).boundingRect();
	QRect b_zx = pRoiList.at(2).boundingRect();
	
	if(b_xy.left()==-1 || b_xy.top()==-1 || b_xy.right()==-1 || b_xy.bottom()==-1)
		vxy=false;
	if(b_yz.left()==-1 || b_yz.top()==-1 || b_yz.right()==-1 || b_yz.bottom()==-1)
		vyz=false;
	if(b_zx.left()==-1 || b_zx.top()==-1 || b_zx.right()==-1 || b_zx.bottom()==-1)
		vzx=false;
		
	long bpos_x, bpos_y, bpos_z, bpos_c, epos_x, epos_y, epos_z, epos_c;
	
	// 8 cases
	if(vxy && vyz && vzx) // all 3 2d-views
	{
		bpos_x = qBound(long(0), long(qMax(b_xy.left(), b_zx.left())), sz0-1);
		bpos_y = qBound(long(0), long(qMax(b_xy.top(),  b_yz.top())), sz1-1);
		bpos_z = qBound(long(0), long(qMax(b_yz.left(), b_zx.top())), sz2-1);
		
		epos_x = qBound(long(0), long(qMin(b_xy.right(), b_zx.right())), sz0-1);
		epos_y = qBound(long(0), long(qMin(b_xy.bottom(), b_yz.bottom())), sz1-1);
		epos_z = qBound(long(0), long(qMin(b_yz.right(), b_zx.bottom())), sz2-1);
	}
	else if(!vxy && vyz && vzx) // 2 of 3
	{
		bpos_x = qBound(long(0), long(qMax(0, b_zx.left())), sz0-1);
		bpos_y = qBound(long(0), long(qMax(0,  b_yz.top())), sz1-1);
		bpos_z = qBound(long(0), long(qMax(b_yz.left(), b_zx.top())), sz2-1);
		
		epos_x = qBound(long(0), long(fmin(sz0-1, b_zx.right())), sz0-1);
		epos_y = qBound(long(0), long(fmin(sz1-1, b_yz.bottom())), sz1-1);
		epos_z = qBound(long(0), long(qMin(b_yz.right(), b_zx.bottom())), sz2-1);
	}
	else if(vxy && !vyz && vzx)
	{
		bpos_x = qBound(long(0), long(qMax(b_xy.left(), b_zx.left())), sz0-1);
		bpos_y = qBound(long(0), long(qMax(b_xy.top(),  0)), sz1-1);
		bpos_z = qBound(long(0), long(qMax(0, b_zx.top())), sz2-1);
		
		epos_x = qBound(long(0), long(qMin(b_xy.right(), b_zx.right())), sz0-1);
		epos_y = qBound(long(0), long(fmin(b_xy.bottom(), sz1-1)), sz1-1);
		epos_z = qBound(long(0), long(fmin(sz2-1, b_zx.bottom())), sz2-1);
	}
	else if(vxy && vyz && !vzx)
	{
		bpos_x = qBound(long(0), long(qMax(b_xy.left(), 0)), sz0-1);
		bpos_y = qBound(long(0), long(qMax(b_xy.top(),  b_yz.top())), sz1-1);
		bpos_z = qBound(long(0), long(qMax(b_yz.left(), 0)), sz2-1);
		
		epos_x = qBound(long(0), long(fmin(b_xy.right(), sz0-1)), sz0-1);
		epos_y = qBound(long(0), long(qMin(b_xy.bottom(), b_yz.bottom())), sz1-1);
		epos_z = qBound(long(0), long(fmin(b_yz.right(), sz2-1)), sz2-1);
	}
	else if(vxy && !vyz && !vzx) // only 1 of 3
	{
		bpos_x = qBound(long(0), long(qMax(b_xy.left(), 0)), sz0-1);
		bpos_y = qBound(long(0), long(qMax(b_xy.top(),  0)), sz1-1);
		bpos_z = 0;
		
		epos_x = qBound(long(0), long(fmin(b_xy.right(), sz0-1)), sz0-1);
		epos_y = qBound(long(0), long(fmin(b_xy.bottom(), sz1-1)), sz1-1);
		epos_z = sz2-1;
	}
	else if(!vxy && vyz && !vzx)
	{
		bpos_x = 0;
		bpos_y = qBound(long(0), long(qMax(0,  b_yz.top())), sz1-1);
		bpos_z = qBound(long(0), long(qMax(b_yz.left(), 0)), sz2-1);
		
		epos_x = sz0-1;
		epos_y = qBound(long(0), long(fmin(sz1-1, b_yz.bottom())), sz1-1);
		epos_z = qBound(long(0), long(fmin(b_yz.right(), sz2-1)), sz2-1);
	}
	else if(!vxy && !vyz && vzx)
	{
		bpos_x = qBound(long(0), long(qMax(0, b_zx.left())), sz0-1);
		bpos_y = 0;
		bpos_z = qBound(long(0), long(qMax(0, b_zx.top())), sz2-1);
		
		epos_x = qBound(long(0), long(fmin(sz0-1, b_zx.right())), sz0-1);
		epos_y = sz1-1;
		epos_z = qBound(long(0), long(fmin(sz2-1, b_zx.bottom())), sz2-1);
	}
	else // 0
	{
		bpos_x = 0;
		bpos_y = 0;
		bpos_z = 0;
		
		epos_x = sz0-1;
		epos_y = sz1-1;
		epos_z = sz2-1;
	}

	//qDebug("x %d y %d z %d x %d y %d z %d ",bpos_x,bpos_y,bpos_z,epos_x,epos_y,epos_z);

	//ROI extraction
	long sx = (epos_x-bpos_x)+1;
    long sy = (epos_y-bpos_y)+1;
    long sz = (epos_z-bpos_z)+1;
	long sc = sz3; // 0,1,2
	
	//choose the channel stack
	long pagesz = sx*sy*sz;
	
	double meanv=0;
	
	long offset_sub = ch*pagesz_sub;
	
	//------------------------------------------------------------------------------------------------------------------------------------
	// scans time
	int i_progress=0;
	int num_progress = 4; // haw many scanning process
	
	//
	QProgressDialog progress("Filling hole...", "Abort Image Filling", 0, num_progress, parent);
	progress.setWindowModality(Qt::WindowModal);
	
	// first scan
	progress.setValue(++i_progress); 
	
	unsigned char *data1d = new unsigned char [pagesz];
	if (!data1d) 
	{
		printf("Fail to allocate memory.\n");
		return;
	}
	else
	{
		for(long k=bpos_z; k<=epos_z; k++)
		{
			long offset_z = k*sz0*sz1;
			long offset_crop_z = (k-bpos_z)*sx*sy;
			for(long j=bpos_y; j<=epos_y; j++)
			{
				long offset_y = j*sz0 + offset_z;
				long offset_crop_y = (j-bpos_y)*sx + offset_crop_z;
				for(long i=bpos_x; i<=epos_x; i++)
				{
					data1d[(i-bpos_x) + offset_crop_y] = pSbject[offset_sub + i+offset_y];
					
					meanv += data1d[(i-bpos_x) + offset_crop_y];
				}
			}
		}
	}
	meanv /= pagesz;
	
	qDebug("mean value %lf", meanv);

	//----------------------------------------------------------------------------------------------------------------------------------
	// preprocess the landmarker
	int seed_x, seed_y, seed_z;
	unsigned char seed_val;
	int range_val=3;
	
	if(use_marker)
	{
		seed_x = list_landmark_sub[0].x -1; // notice 0-based and 1-based difference
		seed_y = list_landmark_sub[0].y -1;
		seed_z = list_landmark_sub[0].z -1;
		
		seed_val = pSbject[seed_z*sz0*sz1 + seed_y*sz0 + seed_x + offset_sub];
	
	}
	else
	{
		if(mean_thresh)
		{
			seed_val = thresh;
			range_val = range;
			
			qDebug() << "seed value" << seed_val;
			
			for(long k=bpos_z; k<=epos_z; k++)
			{
				long offset_crop_z = (k-bpos_z)*sx*sy;
				for(long j=bpos_y; j<=epos_y; j++)
				{
					long offset_crop_y = (j-bpos_y)*sx + offset_crop_z;
					for(long i=bpos_x; i<=epos_x; i++)
					{
						
						if(i==bpos_x || i==epos_x || j==bpos_y || j==epos_y || k==bpos_z || k==epos_z)
							continue;
						
						if(data1d[(i-bpos_x) + offset_crop_y] == seed_val)
						{
							seed_x = (i-bpos_x) ; seed_y = (j-bpos_y); seed_z = (k-bpos_z);
							break;
						}
					}
				}
			}
			
			
		}
		else
		{
			seed_val = 0;
			range_val = meanv;
			
			qDebug() << "seed value" << seed_val;
			
			if(seed_val<0) seed_val = 0;
			
			for(long k=bpos_z; k<=epos_z; k++)
			{
				long offset_crop_z = (k-bpos_z)*sx*sy;
				for(long j=bpos_y; j<=epos_y; j++)
				{
					long offset_crop_y = (j-bpos_y)*sx + offset_crop_z;
					for(long i=bpos_x; i<=epos_x; i++)
					{
						
						if(i==bpos_x || i==epos_x || j==bpos_y || j==epos_y || k==bpos_z || k==epos_z)
							continue;
						
						if(data1d[(i-bpos_x) + offset_crop_y] == seed_val)
						{
							seed_x = (i-bpos_x) ; seed_y = (j-bpos_y); seed_z = (k-bpos_z);
							break;
						}
					}
				}
			}
		
		}
	}
	
	
	qDebug("x %d y %d z %d thresh %d range %d intensity %d", seed_x, seed_y, seed_z, seed_val, range_val, data1d[seed_z*sx*sy + seed_y*sx + seed_x]);
	
	//----------------------------------------------------------------------------------------------------------------------------------
	
	// de-alloc
	//if (pSbject) {delete []pSbject; pSbject=0;} // image visualized in v3d now

	
	// second scan
	progress.setValue(++i_progress); 
	
	// 3D region growing
	//----------------------------------------------------------------------------------------------------------------------------------
	
	int end_preprocess = clock();
	
	printf("time eclapse %d s for preprocessing!\n", (end_preprocess-start_t)/1000000);
	
	enum states {Known, Alive, FarAway, Trial, BOUNDARY}; 
	
	long offset_y, offset_z;
	
	offset_y=sx;
	offset_z=sx*sy;
	
	long neighborhood_6[6] = {-1, 1, -offset_y, offset_y, -offset_z, offset_z}; 
	long neighborhood_26[26] = {-1, 1, -offset_y, offset_y, -offset_z, offset_z,
								-offset_y-1, -offset_y+1, -offset_y-offset_z, -offset_y+offset_z, 
								offset_y-1, offset_y+1, offset_y-offset_z, offset_y+offset_z,
								offset_z-1, offset_z+1, -offset_z-1, -offset_z+1,
								-1-offset_y-offset_z, -1-offset_y+offset_z, -1+offset_y-offset_z, -1+offset_y+offset_z,
								1-offset_y-offset_z, 1-offset_y+offset_z, 1+offset_y-offset_z, 1+offset_y+offset_z}; 
	long neighbors = 26;
	
	long bound_idx = (sz-1)*sx*sy + (sy-1)*sx + sx-1; 
	
	//Fast Marching
	HeapMinSort heap(pagesz);
	
	// initial state heap
	unsigned char* state = new unsigned char[pagesz]; 
	
	unsigned char* phi = new unsigned char [pagesz];
	
	bool* inserted = new bool [pagesz];
	
	for(long k = 0; k < sz; k++) 
	{				
		long idxk = k*offset_z;
		for(long j = 0;  j < sy; j++) 
		{
			long idxj = idxk + j*offset_y;
			
			for(long i = 0, idx = idxj; i < sx;  i++, idx++) 
			{
				
				phi[idx] = 255; 
				state[idx] = BOUNDARY; 
				inserted[idx] = false;
				
				if(i==0 || i==sx-1 || j==0 || j==sy-1 || k==0 || k==sz-1)
					continue;
				
				if( i==seed_x && j==seed_y && k==seed_z ) 
				{
					heap.insert(idx, 0); 
					phi[idx] = 0; 
					state[idx] = Known; 
					inserted[idx] = true;
				}
				else
					state[idx] = FarAway;
				
				
			}
		}
	}
	
//	//
//	progress.setValue(++i_progress); 
	
	long count=1;
	long wc=0;
	
	// del a point from the heap
	long idx;
	float weight; 
	while( heap.del(&idx, &weight) )
	{
		if( state[idx] == Alive ) continue; 		
		
		// using region growing critera here 
		if( state[idx] != Known )
		{
			for(int ineighbor=0; ineighbor<neighbors; ineighbor++)
			{
				long n_idx = idx + neighborhood_26[ineighbor]; 
				
				if(n_idx<bound_idx && n_idx>0)
				{
					
					if(phi[n_idx] == 0 && (fabs(data1d[idx] - seed_val)<=range_val))
					{
						phi[idx] = 0;
						break;
					}
					
				}
				
			}
		}
		else
		{
			phi[idx] = 0;
		}

		count--; wc++;
		
		//set processed one to state "Alive" 
		state[idx] = Alive; 
		
		//find new neighbors and added to heap
		for(int ineighbor=0; ineighbor<neighbors; ineighbor++)
		{
			long n_idx = idx + neighborhood_26[ineighbor]; 
			
			if(n_idx<bound_idx && n_idx>0)
			{
				if( (state[n_idx]==FarAway || state[n_idx]==Trial) && inserted[n_idx]==false)
				{
					heap.insert(n_idx, phi[n_idx]+wc);  count++;
					state[n_idx] = Trial; 
					inserted[n_idx] = true;
				}
			}
		}
		
	}

	// de-alloc
	if (inserted) {delete []inserted; inserted=0;}
	if (state) {delete []state; state=0;}
	
	//----------------------------------------------------------------------------------------------------------------------------------
	
	// third scan
	progress.setValue(++i_progress); 
	
	int end_rgn = clock();
	
	printf("time eclapse %d s for region growing!\n", (end_rgn-end_preprocess)/1000000);
	
	// dist transform
	//----------------------------------------------------------------------------------------------------------------------------------
	long sz_data[4];
	sz_data[0]=sx; sz_data[1]=sy; sz_data[2]=sz; sz_data[3]=1;
	
	long *pDist = new long [pagesz];
	if (!pDist) 
	{
		printf("Fail to allocate memory.\n");
		return;
	}
	long *pLabel = new long [pagesz];
	if (!pLabel) 
	{
		printf("Fail to allocate memory.\n");
		return;
	}
	unsigned char *pData = new unsigned char [2*pagesz]; // first filled and second dist map
	if (!pData) 
	{
		printf("Fail to allocate memory.\n");
		return;
	}
	
	for(long k = 0; k < sz; k++) 
	{				
		long idxk = k*offset_z;
		for(long j = 0;  j < sy; j++) 
		{
			long idxj = idxk + j*offset_y;
			for(long i = 0, idx = idxj; i < sx;  i++, idx++) 
			{
				
				if(i==0 || i==sx-1 || j==0 || j==sy-1 || k==0 || k==sz-1 || phi[idx]==0) // background and boundary
				{
					pData[idx] = 0;
					pData[idx + pagesz] = 0;
				}
				else 
				{

					pData[idx] = 1; //data1d[idx];
					pData[idx + pagesz] = 1;

				}
			}
		}
	
	}
	
	// de-alloc
	if (phi) {delete []phi; phi=0;}
	
	// dist transform
	dt3d_binary(pData, pDist, pLabel, sz_data, 1);
	
	long maxtest=0, mintest=INF;
	for(long i=0; i<pagesz; i++)
	{
		long tmp=pDist[i];
		
		if(maxtest<tmp) maxtest=tmp;
		if(mintest>tmp) mintest=tmp;
	}
	maxtest -= mintest;
	
	if(maxtest)
	{
		for(long i=0; i<pagesz; i++)
		{
			unsigned char tmp = 255*(pDist[i]-mintest)/maxtest;
	
			// image blending
			long tmp1 = tmp + data1d[i];
			
			pData[i] = (tmp1>255)?255:tmp1; // sum
			

			pData[i + pagesz] = tmp;
		}
	}
	
		
	// de-alloc
	if (data1d) {delete []data1d; data1d=0;} //
	if (pDist) {delete []pDist; pDist=0;}
	if (pLabel) {delete []pLabel; pLabel=0;}
	
	//----------------------------------------------------------------------------------------------------------------------------------

	
	int end_dist = clock();
	
	printf("time eclapse %d s for dist computing!\n", (end_dist-end_rgn)/1000000);
	
	
	progress.setValue(num_progress);

	
	Image4DSimple p4DImage;
	p4DImage.setData((unsigned char*)pData, sx, sy, sz, 2, subject->datatype);
	
	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &p4DImage);
	callback.setImageName(newwin, QString("filled_image"));
	callback.updateImageWindow(newwin);

	
}
