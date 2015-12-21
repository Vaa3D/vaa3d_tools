/* smartscope_ip.h
 * 2010-04-02: create this program by Yang Yu
 * 2011-12-21: adapted from Yang Yu's program by Jianlong Zhou
 */


#ifndef __SMARTSCOPE_IP_H__
#define __SMARTSCOPE_IP_H__


//
#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

#define INF 1E9
#define PI 3.14159265

// extension to 3D from Hanchuan's 2D region growing codes
#define UBYTE unsigned char
#define BYTE signed char

static V3DLONG phcDebugPosNum = 0; //volatile
static V3DLONG phcDebugRgnNum = 0;



class POS
{
public:
	V3DLONG pos;
	V3DLONG order;
	POS * next;
	POS() 
	{
		pos = -1;order=-1;
		next = 0;
		phcDebugPosNum++;
	}
	~POS()
	{
		phcDebugPosNum--;
	}
};

class RGN
{
public:
	V3DLONG layer;
	V3DLONG no;
	POS *poslist;
	V3DLONG poslistlen;
	RGN * next;
	RGN() 
	{
		layer=no=-1;
		poslistlen=0;poslist=0;
		next=0;
		phcDebugRgnNum++;
	}
	~RGN()
	{
		layer=no=-1;
		poslistlen = 0;
		phcDebugRgnNum--;
	}
};

//statistics of count of labeling
class STCL
{
public:
	
	V3DLONG count;
	V3DLONG no;
	V3DLONG *desposlist;
	STCL *next;
	
	STCL() 
	{
		count=no=-1;
		next=0;
		phcDebugRgnNum++;
	}
	~STCL()
	{
		count=no=-1;
		phcDebugRgnNum--;
	}
};

// region growing class
class RgnGrow3dClass
{
public:
	RgnGrow3dClass();
	~RgnGrow3dClass();
	
public:
	V3DLONG ImgWid, ImgHei, ImgDep;
	UBYTE * quantImg1d,  *** quantImg3d;
	BYTE *** PHCDONEIMG3d, * PHCDONEIMG1d;
	
	int STACKCNT;
	int MAXSTACKSIZE;
	int IFINCREASELABEL;
	V3DLONG PHCURLABEL;
	int ***PHCLABELSTACK3d, * PHCLABELSTACK1d;
	V3DLONG PHCLABELSTACKPOS;
	
	POS * PHCURGNPOS, * PHCURGNPOS_head;
	RGN * PHCURGN, * PHCURGN_head;

	V3DLONG TOTALPOSnum, TOTALRGNnum;
}; 

class RegionGrowing
{
public:
	RegionGrowing(){bbox_minx=bbox_miny=bbox_maxx= bbox_maxy=0;}
	~RegionGrowing(){}

	void rgnfindsub(int rowi,int colj, int depk, int direction,int stackinc, RgnGrow3dClass * pRgnGrow);
	bool doRegionGrow(V3DPluginCallback* callback, QWidget *parent);

public:
	//unsigned char *pGrowResult;
	//float *pGrowResult;
	V3DLONG bbox_minx, bbox_miny, bbox_maxx, bbox_maxy;

};

//function of swap
template <class T>
void swap (T& x, T& y)
{
	T tmp = x;	x = y; y = tmp;
}

//function of quickSort
template <class T>
void quickSort(T a[], int l, int r)
{
	if(l>=r) return;
	int i = l;
	int j = r+1;
	
	T pivot = a[l];
	while(true)
	{
		do{ i = i+1; } while(a[i]>pivot);
		do{ j = j-1; } while(a[j]<pivot);
		if(i >= j) break;
		swap(a[i], a[j]);
	}
	a[l] = a[j];
	a[j] = pivot;
	quickSort(a, l, j-1);
	quickSort(a, j+1, r);
}

//memory management
template <class T> int newIntImage3dPairMatlabProtocal(T *** & img3d,T * & img1d, V3DLONG imgdep, V3DLONG imghei,V3DLONG imgwid)
{
	V3DLONG totalpxlnum = imghei*imgwid*imgdep;
	
	try
	{
		img1d = new T [totalpxlnum];
		img3d = new T ** [imgdep];
		
		V3DLONG i,j;
		
		for (i=0;i<imgdep;i++) 
		{
			img3d[i] = new T * [imghei];
			for(j=0; j<imghei; j++)
				img3d[i][j] = img1d + i*imghei*imgwid + j*imgwid;
			
		}
		
		memset(img1d, 0, sizeof(T)*totalpxlnum);
	}
	catch(...)
	{
		if (img1d) {delete img1d;img1d=0;}
		if (img3d) {delete img3d;img3d=0;}
		printf("Fail to allocate mem in newIntImage2dPairMatlabProtocal()!");
		return 0; //fail
	}
	
	return 1; //succeed
}

template <class T> void deleteIntImage3dPairMatlabProtocal(T *** & img3d,T * & img1d)
{
	if (img1d) {delete img1d;img1d=0;}
	if (img3d) {delete img3d;img3d=0;}
}

//generating an int image for any input image type
template <class T> void copyvecdata(T * srcdata, V3DLONG len, UBYTE * desdata, int& nstate, UBYTE &minn, UBYTE &maxx)
{
	if(!srcdata || !desdata)
	{
		printf("NULL pointers in copyvecdata()!\n");
		return;
	} 
	
	V3DLONG i;
	
	//note: originally I added 0.5 before rounding, however seems the negative numbers and 
	//      positive numbers are all rounded towarded 0; hence int(-1+0.5)=0 and int(1+0.5)=1;
	//      This is unwanted because I need the above to be -1 and 1.
	// for this reason I just round with 0.5 adjustment for positive and negative differently
	
	//copy data
	if (srcdata[0]>0)
		maxx = minn = int(srcdata[0]+0.5);
	else
		maxx = minn = int(srcdata[0]-0.5);
	
	int tmp;
	double tmp1;
	for (i=0;i<len;i++)
	{
		tmp1 = double(srcdata[i]);
		tmp = (tmp1>0)?(int)(tmp1+0.5):(int)(tmp1-0.5);//round to integers
		minn = (minn<tmp)?minn:tmp;
		maxx = (maxx>tmp)?maxx:tmp;
		desdata[i] = (UBYTE)tmp;
	}
	maxx = (UBYTE)maxx;
	minn = (UBYTE)minn;
	
	//return the #state
	nstate = (maxx-minn+1);
	
	return;
}

#endif



