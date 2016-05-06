/*
These are C-functions used in NeuTuRelease.py and NeuTuAutoTrace.c
 
Copyright (C) 2014, Dezhe Z. Jin (dzj2@psu.edu), Ting Zhao

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <stdint.h>
//#include "image.h"
#include "libNeuTu.h"

struct valInd *heap;		 
struct NeuTuAutoParams PARAMS;
int heapSize;

void readParam(const std::string &filenameParam)
{
  FILE *fpt = NULL;
  if (!filenameParam.empty()) {
    printf("Reading parameters from file %s\n",filenameParam.c_str());
    fpt = fopen(filenameParam.c_str(),"r");
    if (fpt == NULL) {
      printf("WARNING: Failed to read parameter file %s\n",filenameParam.c_str()); 
      printf("Use default parameters instead.");
    }
  }
  if (fpt != NULL) {
    int ind = 0;
    char *line = 0;
    size_t len = 0;
    ssize_t readSize;	
    while ((readSize = getline(&line, &len, fpt)) != -1) {
      if (len > 0 && line[0] == '#') {	//this is a comment.
        continue;
      }
      if (len > 0 && line[0] == '\n') {	//this is an empty line
        continue;
      }
      if (len == 0) continue;
      if (ind == 0) {
        sscanf(line,"%d",&PARAMS.nSplit);
      } else if (ind == 1) {
        sscanf(line,"%f",&PARAMS.zfact);
      } else if (ind == 2) {
        sscanf(line,"%f",&PARAMS.sparse);
      } else if (ind == 3) {
        sscanf(line,"%f",&PARAMS.zJumpFact);
      } else if (ind == 4) {
        sscanf(line,"%f",&PARAMS.expCompr);
      } else if (ind == 5) {
        sscanf(line,"%f",&PARAMS.lambdaRatioThr);
      } else if (ind == 6) {
        sscanf(line, "%d",&PARAMS.levelSetIter);
      } else if (ind == 7) {	
        sscanf(line,"%f",&PARAMS.sigma);
      } else if (ind == 8) {
        sscanf(line,"%d",&PARAMS.Wx);
      } else if (ind == 9) {
        sscanf(line,"%d",&PARAMS.Wy);
      } else if (ind == 10) {
        sscanf(line,"%d",&PARAMS.smallArea);
      } else if (ind == 11) {
        sscanf(line, "%d",&PARAMS.smallLen);
      } else if (ind == 12) {
        sscanf(line, "%d",&PARAMS.mindp);
      } else if (ind == 13) {
        sscanf(line, "%f",&PARAMS.distFact);
      } else if (ind == 14) {	
        sscanf(line, "%d",&PARAMS.minNumPointsBr);
      } else if (ind == 15) {	
        sscanf(line, "%d",&PARAMS.WxSmooth);
      } else if (ind == 16) {	
        sscanf(line, "%d",&PARAMS.WySmooth);
      } else if (ind == 17) {	
        sscanf(line, "%f",&PARAMS.sigmaSmooth);
      } else if (ind == 18) {	
        sscanf(line, "%d",&PARAMS.maxDistFillGaps);
      } else if (ind == 19) { 
        sscanf(line, "%f",&PARAMS.minRadius);
      }
      ind++;
    }
    if (line) free(line);
    fclose(fpt);	
  } else {
    printf("Using default parameters.\n");
    PARAMS.sigma = 1.0;				//Gaussian smoothing withs in pixel unit. 
    PARAMS.Wx = 5; 
    PARAMS.Wy = 5;					//window size for gaussian filter.
    PARAMS.sparse = 0.05;			//IMPOARTANT, upper limit of sparseness in determining the threshold. 
    PARAMS.expCompr = 0.2;			//compression exponent. 
    PARAMS.lambdaRatioThr = 2.0;	//threshold for the ratio lambda1/lambda2 in Hessian to supress blob.  
    PARAMS.levelSetIter = 200;				//levelset smoothing number of iterations. 
    PARAMS.smallArea = 100;			//number of pixels of small areas to be removed in the mask. 

    PARAMS.smallLen = 20;			//length smaller than this in skeleton are pruned. 
    PARAMS.mindp = 5;				//minimum distance between the points in pixel
    PARAMS.distFact = 3.0;			//sets the minimum distance for connecting points, distFact *(r1 + r2), where r1, r2 are the radii of the points to be connected.  
    PARAMS.minNumPointsBr = 3; 		//minimum number of points in a branch for accepting. 
    PARAMS.zJumpFact = 1.0;			//factor for computing maximum z-jump allowed, zJumpFact * zfact * (r1 + r2).
    PARAMS.WxSmooth = 9;			//window size in pixels. 
    PARAMS.WySmooth = 9; 			//Window size for smoothing x, y, z and radius of te branches. 
    PARAMS.sigmaSmooth = 3.0;		//sigma of smoothing the branches. 
    PARAMS.maxDistFillGaps = 50;	//maximum distance in pixels for tempting to fill in the gaps. 		
    PARAMS.nSplit = 5;				//number of sub slabs.
    PARAMS.zfact = 8.0;				//ration between z distance and the xy pixel distance. 		
    PARAMS.minRadius = 2.0;     //minimum radius of swc points. 
  }
  //print parameters. 
  printf("The parameters are:\n");
  printf("sigma=%6.2f Wx=%d Wy=%d sparse=%6.2f expCompr=%6.2f lambdaRatioThr=%6.2f levelSetIter=%d smallArea=%d smallLen=%d mindp=%d distFact=%6.2f minNumPointsBr=%d zJumpFact=%6.2f nSplit=%d, zfact=%6.2f WxSmooth=%d WySmooth=%d sigmaSmooth=%6.2f maxDistFillGaps=%d minRadius=%6.2f\n",
      PARAMS.sigma,PARAMS.Wx,PARAMS.Wy,PARAMS.sparse,PARAMS.expCompr,
      PARAMS.lambdaRatioThr,PARAMS.levelSetIter,PARAMS.smallArea,PARAMS.smallLen,
      PARAMS.mindp,PARAMS.distFact,PARAMS.minNumPointsBr,PARAMS.zJumpFact,PARAMS.nSplit,PARAMS.zfact,
      PARAMS.WxSmooth,PARAMS.WySmooth,PARAMS.sigmaSmooth,PARAMS.maxDistFillGaps,PARAMS.minRadius);

}

//1d index of 2d index (i,j)
inline int getIndex(int i, int j, int nx, int ny)
{
	return i * ny + j;
}

//1d index of 3d index (i,j,k)
inline int getIndex3D(int i, int j, int k, int nx, int ny, int nz)
{
	return i * ny * nz + j * nz + k;
}

//save image arrays. 
void saveImageData(char *filename, int nx, int ny, int nz, float *im3d, float *imFlat, float *imFlatRGB) {
	FILE *fpt;
	fpt = fopen(filename,"wb");
	fwrite(im3d,sizeof(float),nx*ny*nz,fpt);
	fwrite(imFlat,sizeof(float),nx*ny,fpt);
	fwrite(imFlatRGB,sizeof(float),nx*ny*3,fpt);
	fclose(fpt);
}

//read image arrays. 
void readImageData(char *filename, int nx, int ny, int nz, float *im3d, float *imFlat, float *imFlatRGB) {
	FILE *fpt;
	fpt = fopen(filename,"rb");
	fread(im3d,sizeof(float),nx*ny*nz,fpt);
	fread(imFlat,sizeof(float),nx*ny,fpt);
	fread(imFlatRGB,sizeof(float),nx*ny*3,fpt);
	fclose(fpt);
}
#if 0
//probe tiff stack properties using Ting's program. 
void getTiffAttribute(char *filename, int *tiffinfo)
{
	Tiff *t;
	int maxDepth = 1000;
	int  j, type=-1, width=0, height=0, depth=0, nchan=0;
	t = Open_Tiff(filename,"r");
	if (t) {
		depth = 0;
		while ( ! Tiff_EOF(t)){ 
			depth += 1;
			if (depth > maxDepth) {
				printf("ERROR in getTiffAttribute, incorrect tif file %s\n",filename);
				tiffinfo[0] = height;
				tiffinfo[1] = width;
				tiffinfo[2] = depth;
				tiffinfo[3] = nchan;
				tiffinfo[4] = type;
				return;
			}		
			Advance_Tiff(t);
		}
		Rewind_Tiff(t);
		Get_IFD_Shape(t,&width,&height,&nchan);
		type = Get_IFD_Channel_Type(t,0);
		Close_Tiff(t);		
	}	
	tiffinfo[0] = height;
	tiffinfo[1] = width;
	tiffinfo[2] = depth;
	tiffinfo[3] = nchan;
	tiffinfo[4] = type;
}


//create gray scale tiff stack from float arrays
//im3d is the gray scale ndarray in python generated in NeuTu.py
void createGrayTiffStackFromFloatArray(int nx, int ny, int nz, float *im3d, char *outFilename)
{
	int i, nxy, k; 
	Array *plane;
	Tiff *stack;
	uint8 *dat;
	float mmax = -1e5,mmin=1e5;

	nxy = nx * ny;

	//get the maximum and mininum for scaling.
	for (i=0;i < nxy*nz; i++) {
		if (im3d[i] > mmax) mmax = im3d[i];
		if (im3d[i] < mmin) mmin = im3d[i];
	}		

	stack = Open_Tiff(outFilename,"w");
	if (stack == NULL) {
		printf("ERROR IN createGrayTiffStackFromFloatArray, failed to create file %s\n",outFilename);
		return;
	}
	
	for (k=0; k<nz; k++) {
		plane = Make_Array_With_Shape(PLAIN_KIND,UINT8_TYPE,Coord2(nx,ny));
		dat = AUINT8(plane);
		for (i=0; i < nxy; i++) {
			dat[i] = (uint8) ((im3d[k*nxy + i] - mmin)/(mmax-mmin) * 255.0);	// scale the values.  
		}
		Add_IFD_Channel(stack,plane,PLAIN_CHAN);
		Update_Tiff(stack,DONT_PRESS);
		Kill_Array(plane);
	}
	
	Close_Tiff(stack);
}

//create tiff stack from slices using libtiff.
void createTiffStackFromSlices(int nfiles, char **sliceFilenames, char *outFilename)
{
	int i, j, k, width, height, depth, nchan, type, kind;
	Array *plane;
	Tiff *stack, *slice;	
	
	stack = Open_Tiff(outFilename,"w");
	if (stack == NULL) {
		printf("ERROR IN createTiffStackFromSlices, failed to create file %s\n",outFilename);
		return;
	}
	//go through the slices and add to the output tiff.
	for (i=0; i<nfiles; i++) {
		slice = Open_Tiff(sliceFilenames[i],"r");
		if (slice) {
			if (!Get_IFD_Shape(slice,&width,&height,&nchan)) {
				type = Get_IFD_Channel_Type(slice,0);
				for (j=0;j<nchan;j++) {
					plane = Make_Array_With_Shape(PLAIN_KIND,type,Coord2(height,width));
					Get_IFD_Channel(slice,j,plane);
					kind = Get_IFD_Channel_Kind(slice,j);
					Add_IFD_Channel(stack,plane,kind);
					Kill_Array(plane);
				}
				Update_Tiff(stack,DONT_PRESS);
			} else {
				printf("WARNING IN createTiffStackFromSlices, failed to read slice file %s\n",sliceFilenames[i]);
			}
			Close_Tiff(slice);
		} else {
			printf("WARNING IN createTiffStackFromSlices, failed to read slice file %s\n",sliceFilenames[i]);
		}
	}
	Close_Tiff(stack);
}

//read tiff stack using Ting's program. 
#define _READ_TIFF_STACK(dst) \
for (i=0; i<depth;i++) {\
	for (j = 0; j < nchan; j++) { \
		Get_IFD_Channel(t,j,plane);\
		switch (Get_IFD_Channel_Kind(t, j)) {\
		case PLAIN_CHAN:\
			im3d = im3dR;\
			break;\
		case MAPPED_CHAN:\
			im3d = im3dR;\
			break;\
		case RED_CHAN:\
			im3d = im3dR;\
			break;\
		case GREEN_CHAN:\
			im3d = im3dG;\
			break;\
		case BLUE_CHAN:\
			im3d = im3dB;\
			break;\
		default:\
			im3d = im3dR;\
			break;\
		}\
		for (k=0;k<nxy;k++) {\
			im3d[i*nxy+k] = (float) dst[k];\
		}\
	}\
	Advance_Tiff(t);\
}

void readTiffStack(char *filename, int *tiffinfo, float *im3dR, float *im3dG, float *im3dB)
{
	int i, j, k, width, height, depth, nchan, type, nxy;
	Array *plane;
	Tiff *t;
	uint8 *duint8;
	uint16 *duint16;
	uint32 *duint32;
	uint64 *duint64;
	int8 *dint8;
	int16 *dint16;
	int32 *dint32;
	int64 *dint64;
	float32 *dfloat32;
	float64 *dfloat64;
	float *im3d;
	
	height = tiffinfo[0]; 
	width  = tiffinfo[1];
	depth  = tiffinfo[2];
	nchan  = tiffinfo[3];
	type   = tiffinfo[4];
	nxy    = height * width;
	
	t = Open_Tiff(filename,"r");
	if (t) {
		plane = Make_Array_With_Shape(PLAIN_KIND,type,Coord2(height,width));
		switch (type) {
		case UINT8_TYPE:
			duint8 = ((uint8 *) (plane)->data);
			_READ_TIFF_STACK(duint8);
			break;
		case UINT16_TYPE:
			duint16 = ((uint16 *) (plane)->data);
			_READ_TIFF_STACK(duint16);
			break;
		case UINT32_TYPE:
			duint32 = ((uint32 *) (plane)->data);		 
			_READ_TIFF_STACK(duint32);
			break;
		case UINT64_TYPE:
			duint64 = ((uint64 *) (plane)->data);		
			_READ_TIFF_STACK(duint64);
			break;
		case INT8_TYPE: 
			dint8 = ((int8 *) (plane)->data);
			_READ_TIFF_STACK(dint8);
			break;	
		case INT16_TYPE: 
			dint16 = ((int16 *) (plane)->data);
			_READ_TIFF_STACK(dint16);
			break;
		case INT32_TYPE:
			dint32 = ((int32 *) (plane)->data);
			_READ_TIFF_STACK(dint32);
			break;	 
		case INT64_TYPE:
			dint64 = ((int64 *) (plane)->data);
			_READ_TIFF_STACK(dint64);
			break;
		case FLOAT32_TYPE:
			dfloat32 = ((float32 *) (plane)->data);
			_READ_TIFF_STACK(dfloat32);
			break;
		case FLOAT64_TYPE:
			dfloat64 = ((float64 *) (plane)->data);
			_READ_TIFF_STACK(dfloat64);
			break;
		}	
		Kill_Array(plane);	
		Close_Tiff(t);
	}	
}
#endif

//Modified heap data structure code from http://www.thelearningpoint.net/computer-science/data-structures-heaps-with-c-program-source-code
/*Declaring heap globally so that we do not need to pass it as an argument every time*/
/* Heap used here is Min Heap */

/*Initialize Heap*/
void Init()
{
       heapSize = 0;
       heap[0].value = -FLT_MAX;
       heap[0].ind = -1;
}
/*Insert an element into the heap */
//if the element index exists, relpalce the old value and update the heap.
//assuming the updated value is smaller than before.  
void InsertOrUpdate(struct valInd element)
{
	int i, now, flag=0;
	for (i=1;i<=heapSize;i++) {
		if (heap[i].ind == element.ind) {
			flag = 1;
			break;
		}
	}
	if (flag == 0) {	//new index, increase the heap and adjust. 		
        heapSize++;
        heap[heapSize] = element; /*Insert in the last place*/
        now = heapSize;
    } else {	//existing index, replace value and adjust the heap. 
		now = i;   
	}
    /*Adjust its position*/
    while(heap[now/2].value > element.value) 
    {
         heap[now] = heap[now/2];
         now /= 2;
     }
     heap[now] = element;
}

struct valInd DeleteMin()
{
	/* heap[1] is the minimum element. So we remove heap[1]. Size of the heap is decreased. 
           Now heap[1] has to be filled. We put the last element in its place and see if it fits.
           If it does not fit, take minimum element among both its children and replaces parent with it.
           Again See if the last element fits in that place.*/
    struct valInd minElement,lastElement;
    int child,now;
    minElement = heap[1];
    lastElement = heap[heapSize--];
    /* now refers to the index at which we are now */
    for(now = 1; now*2 <= heapSize ;now = child) {
		/* child is the index of the element which is minimum among both the children */ 
        /* Indexes of children are i*2 and i*2 + 1*/
        child = now*2;
        /*child!=heapSize beacuse heap[heapSize+1] does not exist, which means it has only one 
        child */
        if(child != heapSize && heap[child+1].value < heap[child].value ) {
			child++;
        }
        /* To check if the last element fits ot not it suffices to check if the last element
           is less than the minimum element among both the children*/
        if(lastElement.value > heap[child].value) {
			heap[now] = heap[child];
        }
		else /* It fits there */
        {
			break;
        }
	}
    heap[now] = lastElement;
    return minElement;
}

float pixelDistance(int i, int j, float img, int ii, int jj, float imgFrom)
{
	float alpha = 20.0; //exponent in the distant measure
	//double dd = (exp(alpha*abs(img-imgFrom)) + exp(alpha*img))/2.0 * sqrt((i-ii)*(i-ii)+(j-jj)*(j-jj));
	float dd = exp(alpha*img) * sqrt((i-ii)*(i-ii)+(j-jj)*(j-jj));
	return dd;	
}

float pixelDistanceZ(int i, int j, float img, int ii, int jj, float imgFrom, float zfact)
{
	float alpha = 20.0; //exponent in the distant measure
	//double dd = (exp(alpha*abs(img-imgFrom)) + exp(alpha*img))/2.0 * sqrt((i-ii)*(i-ii)+(j-jj)*(j-jj));
	float dd = exp(alpha*img) * sqrt(zfact*zfact*(i-ii)*(i-ii)+(j-jj)*(j-jj));
	//double dd = img * sqrt(zfact*zfact*(i-ii)*(i-ii)+(j-jj)*(j-jj));
	return dd;	
}

//compute the shortest distance using Dijstra's algorithm. Computes shortest distance from one point to all other points. 
void dijstraComputeDists(int is, int js, int nx, int ny, float *img, float *dists) 
{
	int *visited, i, j, k, ii, jj, kk, ntot;
	float dd;
	struct valInd element;
	
	ntot = nx * ny;
	for (i=0;i<ntot;i++) dists[i] = FLT_MAX;
	visited = (int *) malloc(ntot * sizeof(int));
	for (i=0;i<ntot;i++) visited[i] = 0;

	k = getIndex(is,js,nx,ny);
	dists[k] = 0.0;
	visited[k] = 1;
	//allocate heat
	if (heap == NULL) heap = (struct valInd  *) malloc(nx *ny * sizeof(struct valInd));
	Init(); //heap structure for finding minimum.
	while (1) {
		//find the index of 
		ii = k / ny; 
		jj = k % ny;
		for (i=ii-1; i<ii+2; i++) {
			if (i < 0 || i >= nx) continue;
			for (j=jj-1; j<jj+2; j++) {
				if (j < 0 || j >= ny) continue;
				kk = getIndex(i,j,nx,ny);
				if (kk == k || visited[kk] == 1) continue;
				dd = pixelDistance(i,j,img[kk],ii,jj,img[k]) + dists[k];
				dists[kk] = fmin(dists[kk],dd);
				element.value = dists[kk];
				element.ind = kk;
				InsertOrUpdate(element);
			}
		}
		element = DeleteMin();
		k = element.ind;
		if (heapSize == 0) break;
		visited[k] = 1;
	}
	free(visited);
	if (heap != NULL) {
		free(heap); heap = NULL;
	} 
}

//compute the shortest path from (is,js) to (ie,je) given the distances from (is,js)
int dijstraComputePath(int ie, int je, int nx, int ny, float *dists, float *bw) 
{
	int i, j, ii, jj, k, kk, im, lp, ntot;
	float mmin;
	
	ntot = nx * ny;
	for (i=0;i<ntot;i++) bw[i] = 0;
	k = getIndex(ie,je,nx,ny);
	lp = 0;
	bw[k] = 1;
	while (lp < ntot) {
		ii = k / ny; 
		jj = k % ny;
		mmin = FLT_MAX;
		for (i=ii-1; i<ii+2; i++) {
			if (i < 0 || i >= nx) continue;
			for (j=jj-1; j<jj+2; j++) {
				if (j < 0 || j >= ny) continue;
				kk = getIndex(i,j,nx,ny);
				if (kk == k) continue;
				if (dists[kk] < mmin) {
					mmin = dists[kk];
					im = kk;
				}
			}
		}	
		if (mmin == FLT_MAX) break;
		k = im;
		bw[k] = 1;
		++lp;
		if (mmin == 0.0) break;
	}
	return lp;
}
     
float pixelDistance3D(int i, int j, int k, float img, int ii, int jj, int kk, float imgFrom, float zfact)
{
	float alpha = 20.0; //exponent in the distant measure
	//double dd = (exp(alpha*abs(img-imgFrom)) + exp(alpha*img))/2.0 * sqrt((i-ii)*(i-ii)+(j-jj)*(j-jj)+(k-kk)*(k-kk));
	float dd = exp(alpha*img) * sqrt((i-ii)*(i-ii)+(j-jj)*(j-jj)+zfact*zfact*(k-kk)*(k-kk));
	return dd;	
}

//dijstra shortest distance in 3D image, starting from point (is, js, ks), set heapAllocated = 1 if 
//the memory for heap is already allocated. useful if this function is repeated called. 
void dijstraComputeDists3D(int is, int js, int ks, int nx, int ny, int nz, float zfact, float *img3d, float *dists3d, int heapAllocated) 
{
	int *visited, i, j, k, ii, jj, kk, m, mm, irest, ntot;
	float dd;
	struct valInd element;
	ntot = nx * ny * nz;
	for (i=0;i<ntot;i++) dists3d[i] = FLT_MAX;
	visited = (int *) malloc(ntot * sizeof(int));
	for (i=0;i<ntot;i++) visited[i] = 0;

	m = getIndex3D(is,js,ks,nx,ny, nz);
	dists3d[m] = 0.0;
	visited[m] = 1;
	if (heapAllocated != 1 && heap == NULL) heap = (struct valInd *) malloc(ntot*27*sizeof(struct valInd));
	Init(); //heap structure for finding minimum.
	while (1) {
		//find the index of 
		ii = m / (ny * nz); 
		irest = m % (ny * nz);
		jj = irest / nz;
		kk = irest % nz;
		for (i=ii-1; i<ii+2; i++) {
			if (i < 0 || i >= nx) continue;
			for (j=jj-1; j<jj+2; j++) {
				if (j < 0 || j >= ny) continue;
				for (k=kk-1; k<kk+2; k++) {
					if (k< 0 || k >= nz) continue;
					mm = getIndex3D(i,j,k,nx,ny,nz);
					if (mm == m || visited[mm] == 1) continue;
					dd = pixelDistance3D(i,j,k,img3d[mm],ii,jj,kk,img3d[m],zfact) + dists3d[m];
					dists3d[mm] = fmin(dists3d[mm],dd);
					element.value = dists3d[mm];
					element.ind = mm;
					InsertOrUpdate(element);
				}
			}		
		}
		element = DeleteMin();
		m = element.ind;
		if (heapSize == 0) break;
		visited[m] = 1;
	}
	free(visited);
	if (heapAllocated != 1 && heap != NULL) {
		free(heap); heap = NULL;
	}
}

//compute the shortest path from (is,js,ks) to (ie,je,ke) given the distances from (is,js,ks)
//note that dists3d contains shortest distances to all points from (is,js,ks), and is computed using dijstraComputeDists3D.
//the path is stored in x, y, z. This array must be allocated when callig this function. 
int dijstraComputePath3D(int ie, int je, int ke, int nx, int ny, int nz, float *dists3d, float *x, float *y, float *z) 
{
	int i, j, k, ii, jj, kk, m, mm, im, lp, ntot, irest;
	float mmin;
	ntot = nx * ny * nz;
	m = getIndex3D(ie,je,ke,nx,ny,nz);
	lp = 0;
	x[lp] = ie; y[lp] = je; z[lp] = ke; lp++;
	while (lp < ntot) {
		ii = m / (ny * nz); 
		irest = m % (ny * nz);
		jj = irest / nz;
		kk = irest % nz;
		mmin = FLT_MAX;
		for (i=ii-1; i<ii+2; i++) {
			if (i < 0 || i >= nx) continue;
			for (j=jj-1; j<jj+2; j++) {
				if (j < 0 || j >= ny) continue;
				for (k=kk-1; k<kk+2; k++) {
					if (k < 0 || k >= nz) continue;
					mm = getIndex3D(i,j,k,nx,ny,nz);
					if (mm == m) continue;
					if (dists3d[mm] < mmin) {
						mmin = dists3d[mm];
						im = mm;
					}
				}	
			}
		}	
		if (mmin == FLT_MAX) break;
		m = im;
		ii = m / (ny * nz); 
		irest = m % (ny * nz);
		jj = irest / nz;
		kk = irest % nz;
		x[lp] = ii; y[lp] = jj; z[lp] = kk; 
		lp++;
		if (mmin == 0.0) break;
	}
	return lp;
}

//This function builds distance matrix starting from one point on the left.
void computeDists(int ix, int nx, int ny, float zfact, float *img,float *dists) {
	int i,j,iy,i1,i2,ii,jj,istart,iend;
	float ddmin,dd;
	for (i=0;i<nx;i++) {
		for (j=0;j<ny;j++) {
			dists[ny*i+j] = FLT_MAX;
		}
	}
	dists[ix*ny] = img[ix*ny];
	istart = ix;
	iend = ix+1;
	for (iy=1;iy<ny;iy++){
		istart -= 1;
		iend += 1;
		istart = fmax(istart,0);
		iend = fmin(iend,nx);
		for (ii=istart;ii<iend;ii++) {
			ddmin =FLT_MAX;
			i1 = fmax(0,ii-1); 
			i2 = fmin(nx,ii+2);
			for (jj=i1;jj<i2;jj++) {
				dd = pixelDistanceZ(ii,iy,img[ii*ny+iy],jj,iy-1,img[jj*ny+iy-1],zfact) + dists[jj*ny+iy-1];				
				ddmin = fmin(ddmin,dd);
			}
			dists[ii*ny+iy] = ddmin;
		}
	}
}

//This program computes the shortest path from all points from y=0 line to y = ny line. The path steps forward. from left to right
void shortestPathImageLeftToRight(int nx, int ny, float *img, float* ypath, float zfact)
{
    int i,ix,iy,ii;
    int iMin;
    float *minDists,ddmin,dd,*dists;

    dists = (float *)malloc(nx * ny * sizeof(float));
    minDists = (float *)malloc(nx * sizeof(float));

    //printf("Computing z using shortest distance...\n");
    for (ix=0;ix<nx;ix++) {
        computeDists(ix,nx,ny,zfact,img,dists);
        ddmin = FLT_MAX;
        for (ii=0; ii<nx; ii++) {
            if (dists[ii*ny+ny-1] < ddmin) {
                ddmin = dists[ii*ny+ny-1];
            }
        }
        minDists[ix] = ddmin;
    }

    //find the min distance path
    ddmin = FLT_MAX;
    for (ix=0;ix<nx;ix++) {
        if (ddmin > minDists[ix]) {
            ddmin = minDists[ix];
            iMin = ix;
        }
    }
    computeDists(iMin,nx,ny,zfact,img,dists);   
    for (iy=ny-1;iy >=0; iy--){
        ddmin = FLT_MAX;
        for (ix=0;ix<nx;ix++) {
            if (ddmin > dists[ix*ny+iy]) {
                ddmin = dists[ix*ny+iy];
                ii = ix;
            }
        }
        ypath[iy] = ii;
    }
    free(dists);
}

void MakeSet(int n, Node *nodes)
{
	int i;
	for (i=0;i<n;i++) {
		(nodes+i)->parent = nodes+i;	//pointing to itself.
		(nodes+i)->rank =0;				//initial rank 0. 
	}
}

Node* Find(Node *node) 					//find the root. 
{
	if (node->parent != node) {
		node->parent = Find(node->parent);
	}
	return node->parent;
}

void Union(Node *x, Node *y) 
{
	Node *rx, *ry;
	rx = Find(x);
	ry = Find(y);
	if (rx == ry) {
		return;
	}
	if (rx->rank < ry->rank) {		
		rx->parent = ry;
	} else if (rx->rank > ry->rank) {
		ry->parent = rx;
	} else {
		ry->parent = rx;
		rx->rank += 1;
	}	
}

//kruskal's minimum spanning tree algorithm
void kruskal(int n, float distThreshold, float *D, int *E)
{
	//n, number of verticies
	//distThreshold, if the distance is beyond this, the edge is not considered in the graph (open edge)
	//D, array of size n*n, distance matrix
	//E, array of size 2*n+1, edges of the spanning tree, the last number holds the number edges.
	
	int i,j,k,l,ip,ne;
	struct valInd edge;		//value holds the cost of the edge, ind holds the index of the edge.
							//index k = n * v + u, where the edge if between (v, u). 
	int heapAllocated = 0;						
							
	Node *nodes; 	//nodes represeting the verticies. 						
	nodes = (Node *)malloc(n * sizeof(struct Node));
	MakeSet(n,nodes);			//make initia disjoint trees. 
							
	//get the list of the ordered edges using heap data structure. 
	if (heap == NULL) {
		heap = (struct valInd *) malloc(n*n*sizeof(struct valInd));
		heapAllocated = 1;
	}
	Init(); //heap structure for finding minimum.
	for (i=0;i<n;i++) {
		for (j=i+1;j<n;j++) {
			k = i * n + j;
			if (D[k] > distThreshold) {
				continue;
			}
			edge.value = D[k];
			edge.ind = k;
			InsertOrUpdate(edge);
		}
	}
	
	ne = 0;
	while (heapSize > 0) {
		edge = DeleteMin();
		k = edge.ind;
		i = k / n;
		j = k % n;
		if (Find(nodes+i) != Find(nodes+j)) {	//add edge to the tree and merge the sets. this can be optimzed in future. 
			E[2*ne] = i;
			E[2*ne+1] = j;
			ne++;
			Union(nodes+i,nodes+j);	
		}
	}
	E[2*n] = ne;
	printf("Number of edges in the minimum spanning tree = %d\n",ne);	
	free(nodes); 
	if (heapAllocated == 1 && heap != NULL) {
		free(heap);
		heap = NULL;
	}
}


//Gradient vector flow functions. 

//1D case. 
//parameters, n, dimension of the vectors
//v, vector of n, gradient vector field
//Ix, gradient of the image. 
//mu, parameter for controlling the smoothness of v
//dt, time step, dx, lattice space, maxIter, maximum iteration number. 
void gvf1d(int n, float *v, float *Ix, float mu, float dt, float dx, int maxIter)
{
	int i,iter;
	float *v2, r, md;
	r = dt * mu/(dx *dx);
	v2 = (float *) malloc(n * sizeof(float));
	//printf("Last Ix = %f\n",Ix[n-1]);
	//printf("mu=%f dt=%f dx=%f maxIter=%d\n",mu,dt,dx,maxIter);
	for (i=0; i<n; i++) v[i] = 0.0;
	for (iter=0; iter<maxIter; iter++) {
		md = 0.0;
		for (i=1;i<n-1;i++) {
			v2[i] = r * (v[i-1] + v[i+1] - 2 * v[i]) - dt * Ix[i] *Ix[i] * (v[i] - Ix[i]);
			if (fabs(v2[i]) > md) md = fabs(v2[i]);
		}	
		for (i=1;i<n-1;i++) v[i] += v2[i];
		//printf("%f ",md);	
	}
	printf("Final error = %f\n",md);
	free(v2);
}

//2D case. 
//parameters, m, n, dimensions of the images
//v, m x n array, gradient vector field, x component
//u, m x n array, gradient vector field, y component
//Ix, m x n array, gradient of the image, x compoment 
//Iy, m x n array, gradient of the image, y compoment 
//mu, parameter for controlling the smoothness of v
//dt, time step, dx, dy lattice space, maxIter, maximum iteration number. 
//Important, I should be normalize such that its maximum value is close to 1. Otherwise the convergence can suffer. 
void gvf2d(int m, int n, float *v, float *u, float *Ix, float *Iy, float mu, float dt, float dx, float dy, int maxIter)
{
	int i, j, iter, nt, i1;
	float *v2, *u2, rx, ry, md, cm, gm;
	float tol = 1e-3;	//if relative error goes below tol the iteration stops. 
	nt = m * n;
	rx = dt * mu/(dx * dx);
	ry = dt * mu/(dy * dy);
	v2 = (float *) malloc(nt * sizeof(float));
	u2 = (float *) malloc(nt * sizeof(float));
	//printf("Last Ix = %f\n",Ix[nt-1]);
	//printf("Last Iy = %f\n",Iy[nt-1]);
	//printf("mu=%f dt=%f dx=%f dy=%f  maxIter=%d\n",mu,dt,dx,dy,maxIter);
	for (i=0;i<nt;i++) {
		v[i] = 0.0;
		u[i] = 0.0;
	}
	for (iter=0; iter<maxIter; iter++) {
		md = 0.0;
		for (i=1;i<m-1;i++) {
		for (j=1;j<n-1;j++) {	
			i1 = getIndex(i,j,m,n);
			gm = dt * (Ix[i1] * Ix[i1] + Iy[i1] * Iy[i1]);	
			cm =  rx * (v[getIndex(i-1,j,m,n)] + v[getIndex(i+1,j,m,n)] - 2 * v[i1]) 
				+ ry * (v[getIndex(i,j-1,m,n)] + v[getIndex(i,j+1,m,n)] - 2 * v[i1]); 
			v2[i1] = cm - gm * (v[i1] - Ix[i1]);
			cm =  rx * (u[getIndex(i-1,j,m,n)] + u[getIndex(i+1,j,m,n)] - 2 * u[i1]) 
				+ ry * (u[getIndex(i,j-1,m,n)] + u[getIndex(i,j+1,m,n)] - 2 * u[i1]); 
			u2[i1] = cm - gm * (u[i1] - Iy[i1]);			   
			if (fabs(v2[i1]) > md) md = fabs(v2[i1]);
			if (fabs(u2[i1]) > md) md = fabs(u2[i1]);		
		}}	
		cm = 0.0;
		for (i=1;i<m-1;i++) {
		for (j=1;j<n-1;j++) {
			i1 = getIndex(i,j,m,n);
			v[i1] += v2[i1];
			u[i1] += u2[i1];
			if (fabs(v[i1]) > cm) cm = fabs(v[i1]);
			if (fabs(u[i1]) > cm) cm = fabs(u[i1]);					
		}}
		if (md/cm < tol) break;
		//printf("%f ",md);	
	}
	printf("rel error = %f\n",md/cm);

	free(v2);
	free(u2);
}

//region growth method, turn on pixels using the growth method. 
void regionGrowth(int nx, int ny, float *bw, float *imFlat, float threshold)
{
	int i, j, flag, iter, ss, ii, jj;
	iter = 0;
	while (1) {
		++iter;
		flag = 0;
		printf("Region growth at iter = %d \n",iter);
		for (i=1;i<nx-1;i++) {
			for (j=1;j<ny-1;j++) {
				if (bw[i*ny+j] == 1) {
					continue;
				}
				ss = 0;
				for (ii = i-1; ii < i+2; ii++) {
					for (jj = j-1; jj < j+2; jj++) {
						ss += bw[ii*ny+jj];
					}
				}
				if (ss > 0 && ss < 9 && imFlat[i*ny+j] < threshold) {
					bw[i*ny+j]	= 1;
					flag = 1;
				}
			}
		}
		if (flag == 0) {
			break;
		}
	}
}	

//single linked list of integers
LinkedList* GetLastInList(LinkedList *list)
{
	LinkedList *p;
	p = list;
	while (p->next != NULL) {
		p = p->next;
	}
	return p;
}

void AppendToList(LinkedList **list, int val)
{
	LinkedList *newp,*last;
	newp = (LinkedList *) malloc(sizeof(LinkedList));
	newp->val = val;
	newp->next = NULL;
	if ((*list) == NULL) {
		(*list) = newp;
	} else {
		last = GetLastInList((*list));
		last->next = newp;
	}
}

void DeleteList(LinkedList *list)
{
	LinkedList *p, *next;
	p = list;
	while (p != NULL) {
		next = p->next;
		free(p);
		p = next;
	}
	list = NULL;
}

void DeleteFirstElem(LinkedList *list)
{
	LinkedList *p;
	p = list;
	if (list == NULL) return;
	list = list->next;
	free(p);
}

//double linked list of integers
DLinkedList* GetLastInDList(DLinkedList *dlist)
{
	DLinkedList *p;
	p = dlist;
	while (p->next != NULL) {
		p = p->next;
	}
	return p;
}

void AppendToDList(DLinkedList *dlist, int val)
{
	DLinkedList *newp,*last;
	newp = (DLinkedList *) malloc(sizeof(DLinkedList));
	newp->val = val;
	newp->next = NULL;
	newp->prev = NULL;
	last = GetLastInDList(dlist);
	last->next = newp;
	newp->prev = last;
}

void DeleteDList(DLinkedList *dlist)
{
	DLinkedList *p, *next;
	p = dlist->next;
	while (p != NULL) {
		next = p->next;
		free(p);
		p = next;
	}
	dlist = NULL;
}

void DeleteFromDList(DLinkedList *dlist, DLinkedList *pdel)
{
	if (pdel->prev == NULL && pdel->next == NULL) {
		dlist = NULL;
	} else {
		if (pdel->prev == NULL) {
			pdel->next->prev = NULL;
		} else if (pdel->next == NULL) {
			pdel->prev->next = NULL;
		} else {
			pdel->prev->next = pdel->next;
			pdel->next->prev = pdel->prev;
		}
	}		
	free(pdel);
}

//exact squared Euclidean distance transformation. 
//bw contains the binary image, and edt is the distance map. 
void sedt(int nx, int ny, float *bw, float *edt)
{
	int i, j, k, u, v, np, mp, iid, iid2, flag;
	float *h;
	int *ip, *jp; 
	LinkedList **bp, *list;
	//printf("Computing the square Euclidean distance transformation...\n"); 
	h = (float *) malloc(nx * ny * sizeof(float));
	ip = (int *) malloc(nx * ny * sizeof(int));
	bp = (LinkedList **) malloc(nx * sizeof(LinkedList *));
	for (i=0; i<nx; i++) {	// initialize the list for the boundary points. 
		bp[i] = NULL;
	}	
	//get the pixel points and the boundary points. 
	np = 0;		//pixel points
	for (i=1; i<nx-1; i++) {		//note here we ignore the edge of the image for simplicity of the code. 
		for (j=1; j<ny-1; j++) {
			iid = i * ny + j;
			h[iid] = 0;			//initialize h
			edt[iid] = 0;		//initialize edt
			if (bw[iid] > 0) {
				ip[np] = iid;
				++np;
			} else {	//see if this is a boundary point in j direction
				flag = 0;
				for (v=j-1; v <= j+1; v++) {
					if (v != j && bw[i*ny + v] > 0) {
						flag = 1;
						break;
					}
					if (flag == 1) break; 
				}
				if (flag == 1) {	// boundary point, add to the list. 
					AppendToList(&bp[i],iid);
				}	
			}	
		}
	}
	//sweep i - index
	for (k=0; k<np; k++) {
		iid = ip[k];
		i = iid/ny;
		j = iid - i*ny;
		h[iid] = 1e10;
		list = bp[i];
		while (list != NULL) {
			iid2 = list->val - (list->val/ny)*ny;
			h[iid] = fmin(h[iid],(j - iid2 )*(j - iid2));
			list = list->next;
		}
	}
	//sweep j - index
	for (k=0; k<np; k++) {
		iid = ip[k];
		i = iid/ny;
		j = iid - i*ny;
		edt[iid] = 1e10;
		for (u=1; u<nx; u++) {
			iid2 = u*ny + j;
			edt[iid] = fmin(edt[iid],h[iid2] + (u - i)*(u - i));
		}
	}	
	//convert to distance. 
	for (k=0; k<nx*ny; k++) edt[k] = sqrt(edt[k]);
	//delete memory
	free(h);
	free(ip);
	for (i=0; i<nx; i++) {	
		DeleteList(bp[i]);
	}	
	free(bp);
}

//gaussian filter in 2d, modified from Vaa3D plugin gaussianfilter. 
void gaussianFilter2D(int nx, int ny, unsigned int Wx, unsigned int Wy, float *imgIn, float *imgOut, float sigma)
{
	// for filter kernel
	float sigma_s2 = 0.5/(sigma*sigma); // 1/(2*sigma*sigma)
	float pi_sigma = 1.0/(sqrt(2*3.1415926)*sigma); // 1.0/(sqrt(2*pi)*sigma)
	float min_val = FLT_MAX, max_val = 0;
	int i,Weight,ix,iy;
	float  *WeightsX = 0,*WeightsY=0;
	float Half,x,y,k,sum;
	float  *extension_bufferX = 0,*extension_bufferY = 0;
	float  *extStop,*extIter,*stop_line,*arrLeft,*extLeft,*extRight,*arrRight,*resIter;
	float  *weightIter, *End,*arrIter;  
	unsigned int offset;
	float *img;

	//make Wx, Wy odd numbers
	Wx = Wx/2 * 2 + 1;
	Wy = Wy/2 * 2 + 1;

	img = (float *) malloc(nx*ny*sizeof(float));
	for (i=0; i<nx*ny; i++) img[i] = imgIn[i];
    //create Gaussian kernel
    WeightsX = (float *) malloc(Wx*sizeof(float));

    // Gaussian filter equation:
    // http://en.wikipedia.org/wiki/Gaussian_blur
    //   for (unsigned int Weight = 0; Weight < Half; ++Weight)
    //   {
    //        const float  x = Half* float (Weight) / float (Half);
    //         WeightsX[(int)Half - Weight] = WeightsX[(int)Half + Weight] = pi_sigma * exp(-x * x *sigma_s2); // Corresponding symmetric WeightsX
    //    }
	Half = (float)(Wx-1)/2.0;
    for (Weight = 0; Weight <= Half; ++Weight){
		x = Weight -Half;
        WeightsX[Weight]= pi_sigma * exp(-(x * x *sigma_s2)); // Corresponding symmetric WeightsX
		WeightsX[Wx-Weight-1] = WeightsX[Weight];
    }

    k = 0.;
    for (Weight = 0; Weight < Wx; ++Weight) k += WeightsX[Weight];
    for (Weight = 0; Weight < Wx; ++Weight) WeightsX[Weight] /= k;		 

    //   Allocate 1-D extension array
    extension_bufferX = (float *) malloc((nx + (Wx<<1))*sizeof(float)); //size, nx + 2*Wx.
    offset = Wx>>1;			//Half. 

    //	along x
    extStop = extension_bufferX + nx + offset;	
         
	for(iy = 0; iy < ny; iy++) {				//column
		extIter = extension_bufferX + Wx;		//copy the values of the column, starting point.
        for(ix = 0; ix < nx; ix++) {
			*(extIter++) = img[ix*ny + iy]; //copy img to extension_bufferX, starting from extension_bufferX + Wx
        }

        //   Extend image
        stop_line = extension_bufferX - 1;
        extLeft = extension_bufferX + Wx - 1;   //=extension_bufferX + Wx - 1
        arrLeft = extLeft + 2;					//=extension_bufferX + Wx + 1
        extRight = extLeft + nx + 1;			//=extension_bufferX + Wx + nx
        arrRight = extRight - 2;				//=extension_bufferX + Wx + nx - 2
        while (extLeft > stop_line){
			*(extLeft--) = *(arrLeft++);		//reflect around extension_bufferX + Wx
            *(extRight++) = *(arrRight--);		//reflect around extension_bufferX + Wx + nx - 1
		}

        //	Filtering
        extIter = extension_bufferX + Wx - offset;	//=extension_bufferX + Wx/2, original code had a bug!!
		resIter = &(img[iy]);
        while (extIter < extStop) {
			sum = 0.;
            weightIter = WeightsX;
            End = WeightsX + Wx;
            arrIter = extIter;
            while (weightIter < End)
				sum += *(weightIter++) * 1.0 * (*(arrIter++));
            extIter++;
            *(resIter) = sum;
            resIter += ny;

            //for rescale
            if(max_val<*arrIter) max_val = *arrIter;
            if(min_val>*arrIter) min_val = *arrIter;
        }
       
	 }
     //de-alloc
     free(WeightsX); WeightsX=0;
     free(extension_bufferX); extension_bufferX=0;

     //create Gaussian kernel
     WeightsY = (float *)malloc(Wy * sizeof(float));
	 Half = (float)(Wy-1)/2.0;
     for (Weight = 0; Weight <= Half; ++Weight) {
		y = Weight-Half;
        WeightsY[Weight] =  pi_sigma * exp(-(y * y *sigma_s2)); // Corresponding symmetric WeightsY
        WeightsY[Wy-Weight-1] = WeightsY[Weight];
     }

	 k = 0.;
     for (Weight = 0; Weight < Wy; ++Weight) k += WeightsY[Weight];
     for (Weight = 0; Weight < Wy; ++Weight) WeightsY[Weight] /= k;

     //	along y
     extension_bufferY = (float *) malloc((ny + (Wy<<1))*sizeof(float));
     offset = Wy>>1;
     extStop = extension_bufferY + ny + offset;

     for(ix = 0; ix < nx; ix++) {
		extIter = extension_bufferY + Wy;
        for(iy = 0; iy < ny; iy++) {
			*(extIter++) = img[ix*ny + iy];
        }

       //   Extend image
       stop_line = extension_bufferY - 1;
       extLeft = extension_bufferY + Wy - 1;
       arrLeft = extLeft + 2;
       extRight = extLeft + ny + 1;
       arrRight = extRight - 2;

       while (extLeft > stop_line) {
		 *(extLeft--) = *(arrLeft++);
         *(extRight++) = *(arrRight--);
       }

      //	Filtering
      extIter = extension_bufferY + Wy - offset;		//original code had a bug!!

      resIter = &(img[ix*ny]);

      while (extIter < extStop){
		sum = 0.;
        weightIter = WeightsY;
        End = WeightsY + Wy;
        arrIter = extIter;
        while (weightIter < End)
			sum += *(weightIter++) * 1.0 * (*(arrIter++));
        extIter++;
        *(resIter++) = sum;

        //for rescale
        if(max_val<*arrIter) max_val = *arrIter;
        if(min_val>*arrIter) min_val = *arrIter;
      }
               
	}
	for (i=0;i<nx*ny;i++) imgOut[i] = img[i];
    //de-alloc
    free(WeightsY); WeightsY=0;
    free(extension_bufferY); extension_bufferY=0;
    free(img);
}

//levelset method. 
void NeumannBoundCond(int nrow, int ncol, float *g)
{
    // Make a function satisfy Neumann boundary condition
	int i;
	
    g[getIndex(0,0,nrow,ncol)] = g[getIndex(2,2,nrow,ncol)];
    g[getIndex(0,ncol-1,nrow,ncol)] = g[getIndex(2,ncol-3,nrow,ncol)];
    g[getIndex(nrow-1,0,nrow,ncol)] = g[getIndex(nrow-3,2,nrow,ncol)];
    g[getIndex(nrow-1,ncol-1,nrow,ncol)] = g[getIndex(nrow-3,ncol-3,nrow,ncol)];

	for (i=1;i<ncol-1;i++) {
		g[getIndex(0,i,nrow,ncol)] = g[getIndex(2,i,nrow,ncol)];
		g[getIndex(nrow-1,i,nrow,ncol)] = g[getIndex(nrow-3,i,nrow,ncol)];
	}
	for (i=1;i<nrow;i++) {
		g[getIndex(i,0,nrow,ncol)] = g[getIndex(i, 2,nrow,ncol)];
		g[getIndex(i,ncol-1,nrow,ncol)] = g[getIndex(i,ncol-3,nrow,ncol)];
    }
}

float Dirac(float x, float sigma)
{
	float f;
	if (x <= sigma && x >= -sigma) {
		f =(1./2./sigma)*(1.+cos(3.14156*x/sigma));
	} else {
		f = 0.0;
	}	
	return f;	
}

void gradient2d(int nx, int ny, float* g, float* vx, float *vy)
{
	int i,j,ii,jj,kk;
	for (i=1;i<nx-1;i++) {
		for (j=0; j< ny; j++) {
			ii = getIndex(i,j,nx,ny);
			jj = getIndex(i+1,j,nx,ny);
			kk = getIndex(i-1,j,nx,ny);
			vx[ii] = (g[jj] - g[kk])/2.0;
		}
	}
	for (j=0; j<ny; j++) {
		ii = getIndex(0,j,nx,ny);
		jj = getIndex(1,j,nx,ny);
		vx[ii] = g[jj] - g[ii];
		ii = getIndex(nx-1,j,nx,ny);
		jj = getIndex(nx-2,j,nx,ny);
		vx[ii] = g[ii] - g[jj];
	}			
	for (j=1;j<ny-1;j++) {
		for (i=0; i< nx; i++) {
			ii = getIndex(i,j,nx,ny);
			jj = getIndex(i,j+1,nx,ny);
			kk = getIndex(i,j-1,nx,ny);
			vy[ii] = (g[jj] - g[kk])/2.0;
		}
	}
	for (i=0; i<nx; i++) {
		ii = getIndex(i,0,nx,ny);
		jj = getIndex(i,1,nx,ny);
		vy[ii] = g[jj] - g[ii];
		ii = getIndex(i,ny-1,nx,ny);
		jj = getIndex(i,ny-2,nx,ny);
		vy[ii] = g[ii] - g[jj];
	}			
}

void laplacian2d(int nx, int ny, float *u, float *lap)
{
	int i, j,ii,i1,i2,j1,j2;
	for (i=1; i<nx-1; i++) {
		for (j=1; j<ny-1; j++) {
			ii = getIndex(i,j,nx,ny);
			i1 = getIndex(i-1,j,nx,ny);
			i2 = getIndex(i+1,j,nx,ny);
			j1 = getIndex(i,j-1,nx,ny);
			j2 = getIndex(i,j+1,nx,ny);
			lap[ii] = u[i1] + u[i2] + u[j1] + u[j2] - 4.0*u[ii];
		}
	}
}

void curvature_central(int nx,int ny, float *ux, float *uy, float *K)
{
	int i, j, ii, jj, kk;
	float uxx, uyy;
	for (i=1; i<nx-1; i++) {
		for (j=1; j<ny-1; j++) {
			ii = getIndex(i,j,nx,ny);
			jj = getIndex(i,j+1,nx,ny);
			kk = getIndex(i,j-1,nx,ny);
			uyy = (uy[jj] - uy[kk])/2.0;
			jj = getIndex(i+1,j,nx,ny);
			kk = getIndex(i-1,j,nx,ny);
			uxx = (ux[jj] - ux[kk])/2.0;
			K[ii] = uxx + uyy;
		}
	}	
}

//u is the level set function, g is the gradient function. 
void levelSetEvolution(int nx, int ny, float *u, float *g, float lam, float mu, float alf, float epsilon, float delt, int numIter)
{
	int i, j, k, nxy;
	float *vx, *vy, *ux, *uy, *K, *Nx, *Ny, *lap;
	float normDu, diracU, terms, weightedLengthTerm,penalizingTerm,weightedAreaTerm;
	nxy = nx * ny;
	vx = (float *) malloc(nxy * sizeof(float));
	vy = (float *) malloc(nxy * sizeof(float));
	ux = (float *) malloc(nxy * sizeof(float));
	uy = (float *) malloc(nxy * sizeof(float));
	K = (float *) malloc(nxy * sizeof(float));
	lap = (float *) malloc(nxy * sizeof(float));
	
    gradient2d(nx, ny, g, vx, vy);
    
    for (k=0; k<numIter; k++){
        NeumannBoundCond(nx,ny,u);
		//compute laplacian
		laplacian2d(nx, ny, u, lap);
        gradient2d(nx,ny,u,ux,uy);        
        for (i=0; i<nxy; i++) {
			normDu=sqrt(ux[i]*ux[i] + uy[i]*uy[i] + 1e-10);
			ux[i]=ux[i]/normDu;
			uy[i]=uy[i]/normDu;
		}		
		//compute curvature. 
		curvature_central(nx,ny,ux,uy,K);
		for (i=0; i<nxy; i++) {
			diracU=Dirac(u[i],epsilon);
			weightedLengthTerm=lam*diracU*(vx[i]*ux[i] + vy[i]*uy[i] + g[i]*K[i]);   
			weightedAreaTerm=alf*diracU*g[i];			     
			penalizingTerm=mu*(lap[i]-K[i]);
			terms = weightedLengthTerm + weightedAreaTerm + penalizingTerm;
			u[i] = u[i]+delt*terms;  //update the level set function   
		}	
    }    
    free(vx);  
    free(vy);  
    free(ux);
    free(uy);
    free(K);
    free(lap);
}

void levelsetSmoothing(int nx, int ny, float *bw, float *edge,int iter)
{
	int i, nxy;
	float *u, *g;
	float mu, alf, epsilon, lam, delt, timestep, mmax, mmin;

	printf("Level set smoothing...\n");
	
	nxy = nx*ny;
	u = (float *)malloc(nxy * sizeof(float));
	g = (float *)malloc(nxy * sizeof(float));
	
	for (i=0; i<nxy; i++) u[i] = (bw[i] - 0.5) * 2.0;

	mmax = -FLT_MAX;
	mmin = FLT_MAX;
	for (i=0; i<nxy; i++) {
		if (mmax < edge[i]) mmax = edge[i];
		if (mmin > edge[i]) mmin = edge[i];
	}
	for (i=0; i<nxy; i++) {
		g[i]=1. / (1.e-5+pow((edge[i] - mmin)/(mmax - mmin),0.1));  	// edge indicator function.
	}	
	epsilon = 0.5;				// the papramater in the definition of smoothed Dirac function
	timestep = 5.0;  		 		// time step
	mu=0.1/timestep;  			// coefficient for keeping u a signed function. 
								// Note: the product timestep*mu must be less 
								// than 0.25 for stability!
	lam=0.2; 					// coefficient of the weighted length term Lg(\phi)
	alf=0.0; 					// coefficient of the weighted area term Ag(\phi);
								// Note: Choose a positive(negative) alf if the 
								// initial contour is outside(inside) the object.
	delt = 1.0;					// coefficient in updating u in each step. 
								
	levelSetEvolution(nx, ny, u, g, lam, mu, alf, epsilon, delt, iter);

	for (i=0; i<nxy; i++) {
		if (u[i] > 0) {
			bw[i] = 1.0;
		} else {
			bw[i] = 0.0;
		}
	}
	
	free(u);
	free(g);
}	

//compar function for sorting, for inverse sorting, used in getSparseThreshold.
int comp (const void * elem1, const void * elem2) 
{
    float f = *((float*)elem1);
    float s = *((float*)elem2);
    if (f > s) return  -1;
    if (f < s) return 1;
    return 0;
}

float getSparseThreshold(int nx, int ny,float *bw,float sparse)
{
	int i,j,nxy, nn=100,mj;
	float *Is, thrs[100],p;
	
	nxy = nx * ny;
	Is = (float *) malloc(nxy * sizeof(float));
	for (i=0; i<nxy; i++) Is[i] = bw[i];
	qsort (Is, nxy, sizeof(float), comp);
	for (i=0; i<nn; i++) {
		thrs[i] = Is[0] - (Is[0] - Is[nxy-1])/nn*(i+1);
	}
	j = 0;
	mj = 0;
	for (i=0; i<nxy; i++) {
		if (Is[i] < thrs[j]) {
			p = i*1.0/nxy;
			if (p > sparse) {
				mj = j;
				break;
			}
			j++;
		}
	}
	free(Is);
	return thrs[mj];
}	

float getSparseThresholdAdaptive(int nx, int ny,float *bw,float sparseUp)
{
	int i,j,nxy, nn=100,mj,mu, flag;
	float thr, *Is, thrs[100], p[100],ps[100],mmax,dd[100];
	
	nxy = nx * ny;
	Is = (float *) malloc(nxy * sizeof(float));
	for (i=0; i<nxy; i++) Is[i] = bw[i];
	qsort (Is, nxy, sizeof(float), comp);
	for (i=0; i<nn; i++) {
		thrs[i] = Is[0] - (Is[0] - Is[nxy-1])/nn*(i+1);
	}
	for (i=0; i<nn; i++) {
		p[i] = 0.0;
		ps[i] = 0.0;
	}
	j = 0;
	mu = -1;
	for (i=0; i<nxy; i++) {
		if (Is[i] < thrs[j]) {
			p[j] = i*1.0/nxy;
			if (mu==-1 & p[j] > sparseUp) mu = i;
			j++;
		}
	}
	//smooth
	gaussianFilter2D(1,nn,1,30,p,ps,10.0);
	//get second derivative.
	for (i=1;i<nn;i++) {
		dd[i] = (ps[i+1] + ps[i-1] - 2 * ps[i])/2.0;
	}
	//smooth
	gaussianFilter2D(1,nn,1,10,dd,ps,2.0);
	mmax = -FLT_MAX;
	flag = 1;
	for (i=1;i<nn;i++) {
		if (flag == 1 & ps[i] > mmax) {
			mmax = ps[i];
		} else {
			mj = i;
			break;
		}
	}		
	if (p[mj] > sparseUp) {
		thr = thrs[mu];
	} else {
		thr = thrs[mj];
	}
	free(Is);
	return thr;
}

//binary image operations. 
//labeling connected pixels in binary image using two-pass algorithm based on Wikipedia description.
//http://en.wikipedia.org/wiki/Connected-component_labeling
int labelObjectsBinaryImage(int nx, int ny, float *bw, float *labels)
{
	int i,j,ii,jj,nxy,lb,k,L[8],nl,ml;
	Node *nodes; 				//nodes represeting each pixel's label. 
	int *LB;
	
	nxy = nx * ny; 						
	nodes = (Node *)malloc(nxy * sizeof(struct Node));
	MakeSet(nxy,nodes);			//make initia disjoint trees. 
	
	for (i=0; i<nxy; i++) labels[i] = 0.0;	//initialize. 
	//First pass
	lb = 0;
	for (i=0; i<nx; i++) {
	   for (j=0; j<ny; j++) {
		   ii = getIndex(i,j,nx,ny);
		   if (bw[ii] > 0.0) {
			   //check neighbors. 
			   nl = 0;
			   ml = nxy + 1;
			   if (j>0) {
				   jj = getIndex(i,j-1,nx,ny);
				   if (bw[jj] > 0.0) {
					   L[nl] = jj;
					   nl++;
					   if (ml > labels[jj]) ml = labels[jj];
				   }
				}
				if (i>0) {				   
				   jj = getIndex(i-1,j,nx,ny);
				   if (bw[jj] > 0.0) {
					   L[nl] = jj;
					   nl++;
					   if (ml > labels[jj]) ml = labels[jj];
				   }
				}
				if (i > 0 && j > 0) {
				   jj = getIndex(i-1,j-1,nx,ny);
				   if (bw[jj] > 0.0) {
					   L[nl] = jj;
					   if (ml > labels[jj]) ml = labels[jj];
					   nl++;
				   }
				}	
				if (i > 0 && j < ny-1) {
				   jj = getIndex(i-1,j+1,nx,ny);
				   if (bw[jj] > 0.0) {
					   L[nl] = jj;
					   if (ml > labels[jj]) ml = labels[jj];
					   nl++;
				   }
				}	
				
				if (nl ==0) {               //neighbors is empty, new label.
					lb += 1;
					labels[ii] = lb;
					(nodes+ii)->data = lb;
				} else {
					labels[ii] = ml;
					(nodes+ii)->data = ml;
					for (k=0;k<nl;k++) {
						Union(nodes+ii,nodes+L[k]);
					}
				}	
			}
		}	
	}		
  
	LB = (int *) malloc(lb * sizeof(int));
    //Second pass
    nl = 0;
    for (i=0; i<nx; i++) {
		for (j=0; j<ny; j++) {
			ii = getIndex(i,j,nx,ny);
			if (bw[ii] > 0.0) {
				lb = Find(nodes+ii)->data;
				//check if this is a new label.
				if (nl == 0) {
					nl = 1;
					LB[0] = lb;
					k = 1;
				} else {
					for (k=0; k<nl; k++) {
						if (LB[k] == lb) {
							break;
						}
					}
					if (k==nl) {
						nl += 1;
						LB[nl-1] = lb;
						k = nl;
					} else {
						k = k+1;
					} 	
				}
				labels[ii] = k;
			}
		}
	}	
	
    free(nodes);
    free(LB);
    return nl;
}

//skeletonization. 
//Modified from the following source. 
/*
* Code for thinning a binary image using Zhang-Suen algorithm.
* Author: Nash (nash [at] opencv-code [dot] com)
* Website: http://opencv-code.com
*/
/*
* Perform one thinning iteration.
* Normally you wouldn't call this function directly from your code.
*
* Parameters:
* im Binary image with range = [0,1]
* iter 0=even, 1=odd
*/
void thinningIteration(int nx, int ny, float *img, int iter)
{
	int x, y, i, nxy, A, B, m1, m2;
	float *pAbove;
	float *pCurr;
	float *pBelow;
	float *nw, *no, *ne; // north (pAbove)
	float *we, *me, *ea;
	float *sw, *so, *se; // south (pBelow)
	float *pDst;
	float *marker;

	nxy = nx * ny;
	marker = (float *) malloc(nxy * sizeof(float));
	for (i=0; i<nxy; i++) marker[i] = 0.0;
	
	// initialize row pointers
	pAbove = NULL;
	pCurr = img;
	pBelow = img+ny;

	for (x = 1; x < nx-1; x++) {
		// shift the rows up by one
		pAbove = pCurr;
		pCurr = pBelow;
		pBelow = img + (x+1)*ny;
		pDst = marker + x*ny;
		// initialize col pointers
		no = &(pAbove[0]);
		ne = &(pAbove[1]);
		me = &(pCurr[0]);
		ea = &(pCurr[1]);
		so = &(pBelow[0]);
		se = &(pBelow[1]);

		for (y = 1; y < ny-1; y++) {
			// shift col pointers left by one (scan left to right)
			nw = no;
			no = ne;
			ne = &(pAbove[y+1]);
			we = me;
			me = ea;
			ea = &(pCurr[y+1]);
			sw = so;
			so = se;
			se = &(pBelow[y+1]);
			A = (*no == 0 && *ne == 1) + (*ne == 0 && *ea == 1) +
				(*ea == 0 && *se == 1) + (*se == 0 && *so == 1) +
				(*so == 0 && *sw == 1) + (*sw == 0 && *we == 1) +
				(*we == 0 && *nw == 1) + (*nw == 0 && *no == 1);
			B = *no + *ne + *ea + *se + *so + *sw + *we + *nw;
			m1 = iter == 0 ? (*no * *ea * *so) : (*no * *ea * *we);
			m2 = iter == 0 ? (*ea * *so * *we) : (*no * *so * *we);
			if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0) pDst[y] = 1;
		}
	}
	for (i=0; i<nxy; i++) {
		if (marker[i] > 0.0) img[i] = 0.0;
	}		
	free(marker);
}

/*
* Function for thinning the given binary image
*
* Parameters:
* src The source image, binary with range = [0,1.0]
* dst The destination image
*/
void skeletonizeZS(int nx, int ny, float *src, float *dst)
{
	int nxy, i, flag;
	float *prev, diff;
	
	nxy = nx * ny;
	prev = (float *) malloc(nxy * sizeof(float));	
	for (i=0; i<nxy; i++) {
		dst[i] = src[i];
		prev[i] = 0.0;
	}	
	while (1) {
		thinningIteration(nx, ny, dst, 0);
		thinningIteration(nx, ny, dst, 1);
		flag = 0;
		for (i=0; i<nxy; i++) {
			diff = abs(dst[i] - prev[i]);
			if (diff > 0.0) {
				flag = 1;
				break;
			}
		}
		for (i=0; i<nxy; i++) prev[i] = dst[i];
		if (flag == 0) break;
	}
	free(prev);
}

//prune small branches in a skeleton. 
void pruneSmallBranches(int nx, int ny, float *bw, float smallLen)
{
	int i,j,i1,j1,i2,j2,j3,ii,jj,kk,nxy,np,ne,iRemoved,nnp,jjp[8],fl;
	float *flags;
	int *x, *y, *endPoints;
	
	nxy = nx * ny;
	flags = (float *) malloc(nxy * sizeof(float));
	x = (int *) malloc(nxy * sizeof(int));
	y = (int *) malloc(nxy * sizeof(int));
	endPoints = (int *) malloc(nxy * sizeof(int));
	while(1) {
		iRemoved = 0;
		ne = 0;
		//get end points.
		for (ii=0; ii<nxy; ii++) {
			flags[ii] = bw[ii];
			if (bw[ii] > 0.0) {
				i = ii/ny;
				j = ii - i*ny;
				nnp = 0;		
				for (i1 = i-1; i1 < i+2; i1++) {
				for (j1 = j-1; j1 < j+2; j1++) {
					jj = getIndex(i1,j1,nx,ny);
					if (jj == ii || jj < 0 || jj >nxy-1) continue;
					if (bw[jj] > 0.0) {
						jjp[nnp++] = jj;
					}
				}}
				fl = 0;
				if (nnp <= 1) {
					fl = 1;
				} else if (nnp == 2) {
					i1 = jjp[0]/ny; j1 = jjp[0] - i1 * ny;
					i2 = jjp[1]/ny; j2 = jjp[1] - i2 * ny;
					if (sqrt((i1 - i2)*(i1 - i2) + (j1 - j2)*(j1 - j2)) <= sqrt(2.0)) fl = 1;
				}
				if (fl == 1) {
					endPoints[ne++] = ii;
				}
			}
		}
		// construct path from the end points to the next
		for (j3=0;j3<ne; j3++) {
			ii = endPoints[j3];
			if (bw[ii] == 0.0) {//this point is already erased. 
				continue;
			}
			np = 0;
			i = ii/ny; j = ii - i*ny;
			x[np] = i; y[np] = j; np++;
			while (1) { //follow the skeleton.
				flags[ii] = 0;
				//select next point
				nnp = 0;		
				for (i1 = i-1; i1 < i+2; i1++) {
				for (j1 = j-1; j1 < j+2; j1++) {
					jj = getIndex(i1,j1,nx,ny);
					if (jj == ii || jj < 0 || jj >nxy-1) continue;
					if (flags[jj] > 0.0) {
						jjp[nnp++] = jj;
					}
				}}
				fl = 0;
				if (nnp == 1) {
					kk = jjp[0];	//this is the next point. 
					fl = 1;
				} else if (nnp == 2) {
					i1 = jjp[0]/ny; j1 = jjp[0] - i1 * ny;
					i2 = jjp[1]/ny; j2 = jjp[1] - i2 * ny;
					if (sqrt((i1 - i2)*(i1 - i2) + (j1 - j2)*(j1 - j2)) == 1.0) {
						fl = 1;
						if (i1 == i || j1 == j) {
							kk = jjp[0];
						} else if (i2 == i || j2 == j) {
							kk = jjp[1];
						}
					}
				}
				if (fl == 1) {	//get to the next point. 
					ii = kk;
					i = ii/ny; j = ii - i*ny;
					x[np] = i; y[np] = j; np++;
					if (np > smallLen) break;	// this is a long branch. 
				} else {	//end point or cross point, stop. 
					break;
				}
			}
			if (np > 0 && np < smallLen) { // erase this branch. 
				iRemoved = 1;
				for (i1=0; i1<np; i1++) {
					jj = getIndex(x[i1],y[i1],nx,ny);
					bw[jj] = 0.0;
				}
			}
		}
		if (iRemoved == 0) break;
	}
	free(flags);
	free(x);
	free(y);
	free(endPoints);
}

//functions for autotracing neuron, NeuTuAutoTrace

//1d index of 3d index (i,j,k), used for dealing with tif image read in. 
int getIndex3DZfirst(int i, int j, int k, int nx, int ny, int nz)
{
	return k * nx * ny + i * ny + j;
}
 
//create mask bw from image I.    
void createMask2D(int nx, int ny, float *I, float *bw)
{
	int i,j,nxy,maxIter=1000,ii,jj,kk,smallArea;
	float mu, sigma,dt,expCompr, sigmaBack;
	float *Is, *Ix, *Iy, *Ixy, *v, *u, *edge, dx=1.0, dy=1.0;
	unsigned int Wx, Wy, WxBack, WyBack;
	float sparse,thrSparse,lambdaRatioThr;
	int nthrs = 100, iter;
	float thrs[100];
	int *labelCounts, *delLabs;

	printf("Creating mask....\n");

	sigma = PARAMS.sigma;	
	Wx = PARAMS.Wx; 
	Wy = PARAMS.Wy;			
	sparse = PARAMS.sparse;			
	expCompr = PARAMS.expCompr;			
	lambdaRatioThr = PARAMS.lambdaRatioThr;	
	iter = PARAMS.levelSetIter;		
	smallArea = PARAMS.smallArea;		
	
	nxy = nx * ny;
	Is = (float *)malloc(nxy * sizeof(float));
	Ix = (float *)malloc(nxy * sizeof(float));
	Iy = (float *)malloc(nxy * sizeof(float));
	Ixy = (float *)malloc(nxy * sizeof(float));
	u = (float *)malloc(nxy * sizeof(float));
	v = (float *)malloc(nxy * sizeof(float));
	edge = (float *)malloc(nxy * sizeof(float));
	//createGrayTiffStackFromFloatArray(nx,ny,1,I,"temp.tif");		
	gaussianFilter2D(nx, ny, Wx, Wy, I, Is, sigma);	
	// compute Ix and Iy. 
	for (i=1; i<nx-1; i++) {
		for (j=0; j<ny; j++) {
			ii = getIndex(i,  j,nx,ny);
			jj = getIndex(i+1,j,nx,ny);
			kk = getIndex(i-1,j,nx,ny);
			Ix[ii] = (Is[jj] - Is[kk])/(2.0*dx);
		}
	}
	for (j=0; j<ny; j++){	//boundary
		ii = getIndex(0,j,nx,ny);
		jj = getIndex(1,j,nx,ny);
		kk = getIndex(0,j,nx,ny);
		Ix[ii] = (Is[jj] - Is[kk])/(1.0*dx);
		ii = getIndex(nx-1,j,nx,ny);
		jj = getIndex(nx-1,j,nx,ny);
		kk = getIndex(nx-2,j,nx,ny);
		Ix[ii] = (Is[jj] - Is[kk])/(1.0*dx);
	}	
	for (j=1; j<ny-1; j++) {
		for (i=0; i<nx; i++) {
			ii = getIndex(i,j,  nx,ny);
			jj = getIndex(i,j+1,nx,ny);
			kk = getIndex(i,j-1,nx,ny);			
			Iy[ii] = (Is[jj] - Is[kk])/(2.0*dy);
		}
	}	
	for (i=0; i<nx; i++) {	//boundary.
		ii = getIndex(i,0,nx,ny);
		jj = getIndex(i,1,nx,ny);
		kk = getIndex(i,0,nx,ny);
		Iy[ii] = (Is[jj] - Is[kk])/(1.0*dy);
		ii = getIndex(i,ny-1,nx,ny);
		jj = getIndex(i,ny-1,nx,ny);
		kk = getIndex(i,ny-2,nx,ny);
		Iy[ii] = (Is[jj] - Is[kk])/(1.0*dy);
	}	
	gaussianFilter2D(nx, ny, Wx, Wy, Ix, v, sigma);	
	gaussianFilter2D(nx, ny, Wx, Wy, Iy, u, sigma);	
	
	for (i=0;i<nxy;i++) {
		edge[i] = v[i]*v[i] + u[i]*u[i];
	}
	// calculate the Hessian matrix
	for (i=1; i<nx-1; i++) {
		for (j=0; j<ny; j++){
			ii = getIndex(i,  j,nx,ny);
			jj = getIndex(i+1,j,nx,ny);
			kk = getIndex(i-1,j,nx,ny);			
			Ix[ii] = (v[jj] - v[kk])/(2.0*dx);
			Ixy[ii] =(u[jj] - u[kk])/(2.0*dx)/2.0;
		}
	}
	for (j=0; j<ny; j++){	//boundary. 
		ii = getIndex(0,j,nx,ny);
		jj = getIndex(1,j,nx,ny);
		kk = getIndex(0,j,nx,ny);
		Ix[ii] = (v[jj] - v[kk])/(1.0*dx);
		Ixy[ii] = (u[jj] - u[kk])/(1.0*dx)/2.0;
		ii = getIndex(nx-1,j,nx,ny);
		jj = getIndex(nx-1,j,nx,ny);
		kk = getIndex(nx-2,j,nx,ny);
		Ix[ii] = (v[jj] - v[kk])/(1.0*dx);
		Ixy[ii] = (u[jj] - u[kk])/(1.0*dx)/2.0;
	}	
	for (j=1; j<ny-1; j++) {
		for (i=0; i<nx; i++) {
			ii = getIndex(i,j,  nx,ny);
			jj = getIndex(i,j+1,nx,ny);
			kk = getIndex(i,j-1,nx,ny);						
			Iy[ii] = (u[jj] - u[kk])/(2.0*dy);		
			Ixy[ii] += (v[jj] - v[kk])/(2.0*dy)/2.0; 		
		}
	}
	for (i=0; i<nx; i++) {
		ii = getIndex(i,0,nx,ny);
		jj = getIndex(i,1,nx,ny);
		kk = getIndex(i,0,nx,ny);
		Iy[ii] = (u[jj] - u[kk])/(1.0*dy);
		Ixy[ii] += (v[jj] - v[kk])/(2.0*dy)/2.0; 		
		ii = getIndex(i,ny-1,nx,ny);
		jj = getIndex(i,ny-1,nx,ny);
		kk = getIndex(i,ny-2,nx,ny);
		Iy[ii] = (u[jj] - u[kk])/(1.0*dy);
		Ixy[ii] += (v[jj] - v[kk])/(2.0*dy)/2.0; 		
	}	
		
	gaussianFilter2D(nx, ny, Wx, Wy, Ix, Ix, sigma);	
	gaussianFilter2D(nx, ny, Wx, Wy, Iy, Iy, sigma);	
	gaussianFilter2D(nx, ny, Wx, Wy, Ixy,Ixy, sigma);	
	
	for (j=0; j<ny; j++){
		ii = getIndex(0,j,nx,ny);
		Ix[ii] = 0.0;
		Ixy[ii] = 0.0;
		ii = getIndex(nx-1,j,nx,ny);
		Ix[ii] = 0.0;
		Ixy[ii] = 0.0;
	}	
	for (i=0; i<nx; i++) {
		ii = getIndex(i,0,nx,ny);
		Iy[ii] = 0.0;
		Ixy[ii] = 0.0;		
		ii = getIndex(i,ny-1,nx,ny);
		Iy[ii] = 0.0;
		Ixy[ii] = 0.0;		
	}	
		
	// compute the eigen values of the Hessian matrix. 	
	for (i=0; i<nxy; i++) {
		v[i] = 0.5 * (Ix[i] + Iy[i] + sqrt((Ix[i] - Iy[i])*(Ix[i] - Iy[i]) + 4 * Ixy[i] * Ixy[i])); //lambda1
		u[i] = 0.5 * (Ix[i] + Iy[i] - sqrt((Ix[i] - Iy[i])*(Ix[i] - Iy[i]) + 4 * Ixy[i] * Ixy[i]));			//lambda2
	}
	//createGrayTiffStackFromFloatArray(nx,ny,1,v,"temp.tif");	
	gaussianFilter2D(nx, ny, Wx, Wy, v, v, sigma);	
	gaussianFilter2D(nx, ny, Wx, Wy, u, u, sigma);	

	for (i=0;  i<nxy; i++) {
		if (v[i] > lambdaRatioThr*abs(u[i])) {
			//bw[i] = pow(v[i],expComp) * exp(-5.0*abs(u[i])/(v[i]+1e-10));	
			bw[i] = pow(v[i],expCompr);
		} else {
			bw[i] = 0.0;
		}
	}		
	//createGrayTiffStackFromFloatArray(nx,ny,1,bw,"temp.bw0.tif");	
	
	//get the threshold using the sparse critiria. 
	thrSparse = getSparseThreshold(nx,ny,bw,sparse);
	
	for (i=0;i<nxy;i++) {
		if (bw[i] > thrSparse) {
			bw[i] = 1.0;	
		} else {
			bw[i] = 0.0;
		}
	}
	
	/*
	//get soma and thick areas using internsity. 
	gaussianFilter2D(nx, ny, WxBack, WyBack, I, Ixy, sigmaBack);	
	for (i=0; i<nxy; i++) {
		Ixy[i] = Ixy[i] - Is[i];
		if (Ixy[i] < 0.0) {
			Ixy[i] = 0.0;
		} else {
			Ixy[i] = pow(Ixy[i],expCompr);
		}
	}
	thrSparse = getSparseThreshold(nx,ny,Ixy,sparse);
	for (i=0;i<nxy;i++) {
		if (Ixy[i] > thrSparse) {
			bw[i] = 1.0;	
		} 
	}
	*/
		
	//smooth with levelset. 
	levelsetSmoothing(nx, ny, bw, edge, iter);
	
	//remove small areas. 
	printf("Removing small areas ...\n");
	kk = labelObjectsBinaryImage(nx, ny, bw, Is);
	labelCounts = (int *) malloc(kk * sizeof(int));
	for (i=0; i<kk; i++) labelCounts[i] = 0;
	delLabs = (int *) malloc(kk * sizeof(int));
	for (ii=0; ii<nxy; ii++) {
		if (Is[ii] < 1.0) continue;
		for (i=0; i<kk; i++) {
			if ((int)Is[ii] == i+1) labelCounts[i] += 1;
		}
	}
	jj = 0;
	for (i=0; i<kk; i++) {
		if (labelCounts[i] < smallArea) {
			delLabs[jj] = i+1;
			jj += 1;
		}
	}
	for (ii=0; ii<nxy; ii++) {
		if (Is[ii] < 1.0) continue;
		for (i=0; i<jj; i++) {
			if ((int)Is[ii] == delLabs[i]) {
				bw[ii] = 0.0;
				break;
			}
		}
	}
	
	//get rid of boundary effect.
	for (i=0;i<5; i++) {
		for (j=0; j<ny; j++) {
			bw[getIndex(i,j,nx,ny)] = 0.0;
			bw[getIndex(nx-1-i,j,nx,ny)] = 0.0;
		}
	}
	for (j=0;j<5; j++) {
		for (i=0; i<nx; i++) {
			bw[getIndex(i,j,nx,ny)] = 0.0;
			bw[getIndex(i,ny-1-j,nx,ny)] = 0.0;
		}
	}
				
	//createGrayTiffStackFromFloatArray(nx,ny,1,bw,"temp.bw1.tif");	

	free(Is);
	free(Ix); 
	free(Iy); 
	free(u);
	free(v);
	free(Ixy);
	free(edge);
	free(labelCounts);
	free(delLabs);
}  

//create a linked point. 
LinkedPoint *CreateLinkedPoint(float x, float y, float z, float r, int ID, int Type)
{
	LinkedPoint *p;
	p = (LinkedPoint *)malloc(sizeof(LinkedPoint));
	p->x = x;
	p->y = y;
	p->z = z;
	p->r = r;
	p->ID = ID;
	p->Type = Type;
	p->conn = (LinkedList *) malloc(sizeof(LinkedList));
	p->conn->val =0;		//the first element in the conn list contains the number of elements in the list.
	p->conn->next = NULL;
	return p;
}

void DeleteLinkedPoint(LinkedPoint *p) 
{
	if (p == NULL) return;
	if (p->conn != NULL) {
		DeleteList(p->conn);
	}
	free(p);
	p = NULL;
}

void AddConnLinkedPoint(LinkedPoint *p, int ID)
{
	if (p==NULL) return;
	AppendToList(&(p->conn),ID);
	p->conn->val += 1;
}

void DelConnLinkedPoint(LinkedPoint *p, int ID)
{
	if (p == NULL) return;
	LinkedList *cp,*cpp;
	cpp = p->conn;
	cp = p->conn->next;
	while (cp != NULL) {
		if (cp->val == ID) {
			cpp->next = cp->next;
			free(cp);
			break;
		} else {
			cpp = cp;
			cp = cp->next;
		}
	}	
}

int NumConnLinkedPoint(LinkedPoint *p)
{	
	return p->conn->val;
}

//check whether a LinkedPoint is valid. 
int checkValdityOfLinkedPoint(int x, int y, int z, int r, int nx, int ny, int nz, float *im3d)
{
	int i,j,ii,jj,xx,yy;
	int nmax = 300,nr=10;	//maximum radius is assumed to be nmax/nr. 
	int nbox,ntot,nnx,nny;
	float bw[nmax*nmax],distMap[nmax*nmax],r2;
	int im,flag; //nr*r is the box size. 
	float mmax = -FLT_MAX, mmean = 0.0, threshold;
	//get the local image. 
	nbox = nr*r;	//size of the box. 
	ntot = nx*ny*nz;
	if (nbox > nmax) return -1;	//something is wrong, the radius is too big. 
	for (ii=0; ii<nbox*nbox; ii++) bw[ii] = 1.0;
	//copy the image.
	jj = 0; 
	im = 0;
	for (i=fmax(0,x-nr*r); i<fmin(nx,x+nr*r); i++) {
		for (j=fmax(0,y-nr*r); j<fmin(ny,y+nr*r); j++) {
			ii = getIndex3DZfirst(i,j,z,nx,ny,nz);
			bw[jj++] = im3d[ii];
			if (sqrt((i-x)*(i-x)+(j-y)*(j-y)) < r) {
				if (im3d[ii] > mmax) mmax = im3d[ii];
				mmean += im3d[ii]; im++;
			}
		}
	}
	nnx = (int)(fmin(nx,x+nr*r) - fmax(0,x-nr*r));
	nny = (int)(fmin(ny,y+nr*r) - fmax(0,y-nr*r));
	xx = (int) (x - fmax(0,x-nr*r));
	yy = (int) (y - fmax(0,y-nr*r));
	mmean /= im;
	threshold = mmean + (mmax - mmean) * 0.9;	//threshold for binary. 
	for (ii=0; ii<nnx*nny; ii++) {
		if (bw[ii] < threshold) {
			bw[ii] = 1;
		} else {
			bw[ii] = 0;
		}
	}
	sedt(nnx,nny,bw,distMap);
	r2 = distMap[getIndex(xx,yy,nnx,nny)];
	if (r2 < PARAMS.minRadius) {
		flag = 0;
	} else {
		if (abs(r2- r) <=2.0 || abs(r2 - r)/((r2 + r)/2.0) < 0.5) {
			flag = 1;
		} else {
			flag = 0;
		}
	}
//	printf("(%d,%d,%d) ",(int)distMap[getIndex(xx,yy,nnx,nny)],r,flag);
	return flag;
}

//get Linked points from the skeleton.
//bw contains the skeleton in the binary image, distMap contains the distance map.  
void getLinkedPointsFromSkeletonAnd3D(int nx,int ny,int nz,float *bw, float *distMap, float *im3d,int z1,int z2,int z1ext,int z2ext,int *npoints,LinkedPoint **points, float zfact)
{	
	float *zmap;
	int i,j,i1,j1,i2,j2,j3,ii,jj,kk,k2,dk,nxy,np,ne,nnp,jjp[8],fl,iDone,k3;
	float *flags, mr, *z;
	int *x, *y, *endPoints, mindp, maxdp;
	int *iidList, niid, flag, nzz;
	float r1,r2,zJumpFact,*r,dd,xs,ys,xe,ye,dotp;
	int nxym;
	int nw,*iidw,ni,iip[8],ii1,jj1;
	
	mindp = PARAMS.mindp;	
	zJumpFact = PARAMS.zJumpFact;
	nxy = nx * ny;
	nxym = (int)(nxy * 0.2);
	flags = (float *) malloc(nxy * sizeof(float));	
	x = (int *) malloc(nxym * sizeof(int));
	y = (int *) malloc(nxym * sizeof(int));
	z = (float *) malloc(nxym * sizeof(float));
	r = (float *) malloc(nxym * sizeof(float));	
	endPoints = (int *) malloc(nxym * sizeof(int));
	zmap = (float *) malloc(nxym * nz * sizeof(float));
	iidList = (int *) malloc(nxym * sizeof(int));
	iidw = (int *) malloc(nxym * sizeof(int));
	
	//get rid of boundary points to help judging the end points.
	for (i=0; i<nx; i++) {
		bw[getIndex(i,0,nx,ny)] = 0;
		bw[getIndex(i,1,nx,ny)] = 0;
	}	
	for (j=0; j<ny; j++) {
		bw[getIndex(0,j,nx,ny)] = 0;
		bw[getIndex(1,j,nx,ny)] = 0;
	}	
	for (ii=0; ii<nxy; ii++) flags[ii] = bw[ii];
	nzz = z2ext - z1ext;
	
	//get the ids of white pixels.
	nw = 0; 
	for (ii=0; ii<nxy; ii++) if (flags[ii] > 0) iidw[nw++] = ii;
	
	//get rid of kinks.
	for (kk=0; kk<nw; kk++) {
		ii = iidw[kk];
		i = ii/ny;
		j = ii - i*ny;
		nnp = 0;		
		for (i1 = i-1; i1 < i+2; i1++) {
		for (j1 = j-1; j1 < j+2; j1++) {
			jj = getIndex(i1,j1,nx,ny);
			if (jj == ii || jj < 0 || jj >nxy-1) continue;
			if (flags[jj] > 0.0) {
				jjp[nnp++] = jj;
			}
		}}
		if (nnp >= 2) {
			//see if these points are connected. 
			iip[0] = jjp[0];
			ni = 1;
			while (1) {
				fl = 0;
				for (ii1=0; ii1<nnp; ii1++) {
					flag = 0;
					for (jj1=0; jj1<ni; jj1++) {
						if (jjp[ii1] == iip[jj1]) {
							flag = 1;
							break;
						} 
					}
					if (flag == 1) continue;
					flag = 0;
					for (jj1=0; jj1<ni; jj1++) {
						i1 = jjp[ii1]/ny; j1 = jjp[ii1] - i1 * ny;
						i2 = iip[jj1]/ny; j2 = iip[jj1] - i2 * ny;
						if (sqrt((i1 - i2)*(i1 - i2) + (j1 - j2)*(j1 - j2)) <= sqrt(2.0)) {
							iip[ni++] = jjp[ii1];
							flag = 1;
							break;
						}
					}	
					if (flag == 1) fl = 1;
				}
				if (fl == 0) break;			
			}
			if (ni == nnp) {	//connected, the center point is redundent.
				flags[ii]= 0;			
			}	
		}
	}
			
    //break crossing points. 
    for (kk=0; kk<nw; kk++) {
		ii = iidw[kk];
		i = ii/ny;
		j = ii - i*ny;
		nnp = 0;		
		for (i1 = i-1; i1 < i+2; i1++) {
		for (j1 = j-1; j1 < j+2; j1++) {
			jj = getIndex(i1,j1,nx,ny);
			if (jj == ii || jj < 0 || jj >nxy-1) continue;
			if (flags[jj] > 0.0) {
				jjp[nnp++] = jj;
			}
		}}
		if (nnp > 2) { //this is the crossing point, create end points by erasing. 
			flags[ii] = 0;
		}
	}		
	nw = 0; 
	for (ii=0; ii<nxy; ii++) if (flags[ii] > 0) iidw[nw++] = ii;
	
	//create linked points. 
	//get end points. 
	ne = 0;
	//get end points.
    for (kk=0; kk<nw; kk++) {
		ii = iidw[kk];
		i = ii/ny;
		j = ii - i*ny;
		nnp = 0;		
		for (i1 = i-1; i1 < i+2; i1++) {
		for (j1 = j-1; j1 < j+2; j1++) {
			jj = getIndex(i1,j1,nx,ny);
			if (jj == ii || jj < 0 || jj >nxy-1) continue;
			if (flags[jj] > 0.0) {
				jjp[nnp++] = jj;
			}
		}}
		if (nnp <= 1) {
			endPoints[ne++] = ii;
		}
	}		
	
	// construct path from the end points to the next
	for (j3=0;j3<ne; j3++) {
		ii = endPoints[j3];
		if (flags[ii] == 0.0) {//this point is already erased. 
			continue;
		}
		np = 0;
		i = ii/ny; j = ii - i*ny;
		x[np] = i; y[np] = j; r[np] = distMap[ii]; np++;
		mr = 0;
		while (1) { //follow the skeleton.
			flags[ii] = 0;
			mr += distMap[ii];
			//select next point
			nnp = 0;		
			for (i1 = i-1; i1 < i+2; i1++) {
			for (j1 = j-1; j1 < j+2; j1++) {
				jj = getIndex(i1,j1,nx,ny);
				if (jj == ii || jj < 0 || jj >nxy-1) continue;
				if (flags[jj] > 0.0) {
					jjp[nnp++] = jj;
				}
			}}
			fl = 0;
			if (nnp == 1) {
				kk = jjp[0];	//this is the next point. 
				fl = 1;
			} else if (nnp == 2) {
				i1 = jjp[0]/ny; j1 = jjp[0] - i1 * ny;
				i2 = jjp[1]/ny; j2 = jjp[1] - i2 * ny;
				if (sqrt((i1 - i2)*(i1 - i2) + (j1 - j2)*(j1 - j2)) == 1.0) {
					fl = 1;
					if (i1 == i || j1 == j) {
						kk = jjp[0];
						flags[jjp[1]] = 0;
					} else if (i2 == i || j2 == j) {
						kk = jjp[1];
						flags[jjp[0]] = 0;
					}
				}
			}
			if (fl == 1) {	//get to the next point. 
				ii = kk;
				i = ii/ny; j = ii - i*ny;
				x[np] = i; y[np] = j; r[np] = distMap[ii]; np++;
			} else {	//end point or cross point, stop. 
				break;
			}
		}	
		if (np > 2*mindp) { // get LinkedPoints from this branch. 
			//printf("creating branch np=%d ",np);
			//compute z.
			kk = 0;
			for (i=z1ext; i<z2ext; i++) {
				for (j=0; j<np; j++) {
					jj = getIndex3DZfirst(x[j],y[j],i,nx,ny,nz);
					zmap[kk++] = im3d[jj];
				}
			} 
			shortestPathImageLeftToRight(nzz,np,zmap,z,zfact);
			for (i=0; i<np; i++) z[i] += z1ext;
			
			//smooth x, y, z
			gaussianFilter2D(1,np,1,PARAMS.WySmooth,(float *)x,(float *)x,PARAMS.sigmaSmooth);
			gaussianFilter2D(1,np,1,PARAMS.WySmooth,(float *)y,(float *)y,PARAMS.sigmaSmooth);
			gaussianFilter2D(1,np,1,PARAMS.WySmooth,(float *)z,(float *)z,PARAMS.sigmaSmooth);
			gaussianFilter2D(1,np,1,PARAMS.WySmooth,r,r,PARAMS.sigmaSmooth);
			
			//get the list of points to create LinkedPoints. 
			niid = 0;
			kk=0;
			r1 = 0;
			xs = x[0]; ys = y[0]; 
			while (1) {
				flag = 0;
				for (k3 = kk; k3 < np; k3++) {
					r2 = r[k3];
					dd = sqrt((x[k3] - xs)*(x[k3] - xs) + (y[k3] - ys)*(y[k3] - ys));
					if (dd > r1 + r2) {
						kk = k3;
						iidList[niid++] = kk;
						flag = 1;
						r1 = r2;
						xs = x[kk]; ys = y[kk];
						break;
					}
				}
				if (flag == 0) break;
			}
						
			//create linked points. 
			kk = iidList[0];
			flag = 0;				
			if (r[kk] > PARAMS.minRadius && z[kk] >=z1 && z[kk] < z2 &&
				checkValdityOfLinkedPoint(x[kk],y[kk],z[kk],r[kk],nx,ny,nz,im3d) == 1) {	
				points[(*npoints)] = CreateLinkedPoint(x[kk],y[kk],z[kk],r[kk],(*npoints),3);
				(*npoints)++; 
				flag = 1;
				nnp = 1;
			} else {
				flag = 0;
				nnp = 0;
			}
			for (i=1; i<niid; i++) {
				kk = iidList[i];
				if (r[kk] > PARAMS.minRadius && z[kk] >=z1 && z[kk] < z2 && 
					checkValdityOfLinkedPoint(x[kk],y[kk],z[kk],r[kk],nx,ny,nz,im3d) == 1) {	
					points[(*npoints)] = CreateLinkedPoint(x[kk],y[kk],z[kk],r[kk],(*npoints),3);
					//check the z jump and angle. 
					dotp = 1.0;
					if (nnp >= 2) {
						xs = points[(*npoints)-1]->x - points[(*npoints)-2]->x; 
						ys = points[(*npoints)-1]->y - points[(*npoints)-2]->y;
						dd = fmax(1e-5,sqrt(xs*xs + ys*ys));
						xs /= dd; ys /= dd; 
						xe = points[(*npoints)]->x - points[(*npoints)-1]->x;
						ye = points[(*npoints)]->y - points[(*npoints)-1]->y; 
						dd = fmax(1e-5,sqrt(xe*xe + ye*ye));
						xe /= dd; ye /= dd; 
						dotp = xs * xe + ys * ye;
					}
					if (flag == 1  && dotp > cos(45.0/180.0*3.1415926) //if the angle is more than 45 degrees do not connect. This prevents noisy side branches. 
					&& zfact * abs(points[(*npoints)-1]->z - points[(*npoints)]->z) <= zJumpFact * (points[(*npoints)-1]->r + points[(*npoints)]->r)) {					
						AddConnLinkedPoint(points[(*npoints)-1],(*npoints));
						AddConnLinkedPoint(points[(*npoints)],(*npoints)-1);
					}
					(*npoints)++;
					nnp++;
					flag =1;
				} else {
					flag = 0;
					nnp = 0;
				}	
			}		
		} 
	}
 
	free(flags);
	free(x);
	free(y);
	free(z);
	free(r);
	free(endPoints);	
	free(zmap);
	free(iidList);
	free(iidw);
}

//return 1 if LinkedPoint p is connected with a point with ID.  
int checkConnected(LinkedPoint *p, int ID, int npoints, LinkedPoint **points, int *IDInds, int *flags)
{
	int i, flag;
	LinkedPoint *pnext;
	LinkedList *iter;
	if (p == NULL) return 0;
	flags[p->ID] = 1;
	iter = p->conn->next;
	while (iter != NULL) {
		if (iter->val == ID) {
			return 1;
		}
		iter = iter->next;
	}
	iter = p->conn->next;
	while (iter != NULL) {
		i = IDInds[iter->val];
		pnext = points[i];
		if (pnext == NULL) break;
		if (flags[pnext->ID] == 0) { //not visited yet.
			flag = checkConnected(pnext, ID, npoints, points, IDInds, flags);
			if (flag == 1) {
				return 1;
			} 
		}
		iter = iter->next;
	}		
	return 0;
}

//connect end points in the linked points. 
void connectEndPoints(int nx, int ny, int nz, float *im3d, int npoints, LinkedPoint **points, float zfact)
{
	int i,j,ne,ii,jj,kk;
	float x1,x2,y1,y2,z1,z2,r1,r2,dd;
	int *eps, nE;
	int *flags,mmax,*IDInds;	
	int xs,xe,ys,ye,zs,ze,nxx,nyy,nzz,i1,j1,k1;
	float *epsVec;
	struct valInd element;
	int *visited; 
	
	mmax = 0;
	for (i=0;i<npoints;i++) {
		if (mmax < points[i]->ID) mmax = points[i]->ID;
	}
	IDInds = (int *) malloc((mmax+1) * sizeof(int));
	for (i=0; i<npoints; i++) IDInds[points[i]->ID] = i;
	eps = (int *) malloc(npoints * sizeof(int));
	epsVec = (float *) malloc(2*npoints * sizeof(int));
	//get the end points. 
	ne = 0;
	for (i=0; i<npoints; i++) {
		if (NumConnLinkedPoint(points[i]) == 1) {
			eps[ne] = i;
			j = IDInds[points[i]->conn->next->val];
			xs = points[i]->x - points[j]->x;
			ys = points[i]->y - points[j]->y;
			dd = fmax(1e-5,sqrt(xs*xs + ys*ys));
			epsVec[2*ne] = xs/dd;
			epsVec[2*ne+1] = ys/dd;			
			ne++;
		}
	}
	if (ne == 0) {
		free(eps);
		free(IDInds); 
		return;
	}
	//compute distances between the end points. 
	flags = (int *) malloc((mmax+1) * sizeof(int));
	
	//allocate heat
	if (heap == NULL) free(heap); 
	heap = (struct valInd  *) malloc(ne *ne * sizeof(struct valInd));
	Init(); //heap structure for finding minimum.
	
	for (i=0; i<ne; i++) {
		ii = eps[i];
		x1 = points[ii]->x;
		y1 = points[ii]->y;
		z1 = points[ii]->z;
		r1 = points[ii]->r;
		for (j=i+1; j<ne; j++) {			
			jj = eps[j];
			for (kk=0;kk<mmax+1;kk++) flags[kk] = 0;
			if (checkConnected(points[ii], points[jj]->ID, npoints, points, IDInds, flags) == 1) continue;
			
			x2 = points[jj]->x;
			y2 = points[jj]->y;
			z2 = points[jj]->z;
			r2 = points[jj]->r;
			
			if (zfact * abs(z2 - z1) > PARAMS.zJumpFact * (r1 + r2)) continue; //z difference is too large. 		
			
			dd = -(epsVec[2*i]*epsVec[2*j] + epsVec[2*i+1]*epsVec[2*j+1]);
			if ( dd < cos(60.0/180.0*3.14159)) continue; //angle between the end points are too large.  
			
			dd = sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)); 			
			if (dd < (r1 + r2) * PARAMS.distFact) { //candidate for connection, insert to the heap.  
				element.value = dd;
				element.ind = getIndex(i,j,ne,ne);
				InsertOrUpdate(element);
			}
		}
	}
	visited = (int *) malloc(ne * sizeof(int));
	for (i=0; i<ne; i++) visited[i] = 0;
	while (1) {
		if (heapSize == 0) break;
		element = DeleteMin();
		i = element.ind/ne;
		j = element.ind - i * ne;
		if (visited[i] == 1 || visited[j] == 1) continue;
		ii = eps[i];
		jj = eps[j];
		visited[i] = 1; visited[j] = 1;	
		AddConnLinkedPoint(points[ii],points[jj]->ID);
		AddConnLinkedPoint(points[jj],points[ii]->ID);
	}
	free(IDInds);
	free(eps);
	free(epsVec);
	free(flags);	
	free(heap); heap = NULL;
	free(visited); 
}


//fill gaps between end points using 3D shortest path. 
void fillGapsEndPoints(int nx, int ny, int nz, float *im3d, int *npoints, LinkedPoint **points, float zfact)
{
	float *im3dS, *dists3d;
	int i, j, k, ii, jj, kk, nxx, nyy, nzz, ntot, ne, i1, j1, k1, np;
	float x1,x2,y1,y2,z1,z2,r1,r2,dd;
	int *eps;
	int maxDistFillGaps; 
	int xs,ys,zs,xe,ye,ze;
	float *x,*y,*z;
	float *r;
	int *flags, flag, *IDInds;
	float mm,mmax,mmin,vv,threshold;
	int heapAllocated = 1;

	printf("Filling gaps between end points...\n");
		
	eps = (int *) malloc((*npoints) * sizeof(int));	
	//get the end points. 
	ne = 0;
	for (i=0; i<(*npoints); i++) {
		if (NumConnLinkedPoint(points[i]) == 1) {
			eps[ne++] = i;
		}
	}
	if (ne == 0) {
		free(eps); 
		return;
	}

	maxDistFillGaps = PARAMS.maxDistFillGaps;	 
	ntot = 8 * maxDistFillGaps * maxDistFillGaps * maxDistFillGaps;
	im3dS = (float *) malloc(ntot * sizeof(float));
	dists3d = (float *) malloc(ntot * sizeof(float));
	x  = (float *) malloc(ntot * sizeof(float));
	y  = (float *) malloc(ntot * sizeof(float));
	z  = (float *) malloc(ntot * sizeof(float));
	r  = (float *) malloc(ntot * sizeof(float));
	heap = (struct valInd *) malloc(ntot*27*sizeof(struct valInd));
	
	//get the pixel value distributions of the points. 
	//get the pixel threshold. 
	mm = 0.0;
	mmax = -FLT_MAX;
	mmin = FLT_MAX;
	for (kk=0; kk<(*npoints); kk++) {
		vv = im3d[getIndex3D(points[kk]->x,points[kk]->y,points[kk]->z,nx,ny,nz)];
		mm += vv;
		if (vv > mmax) mmax = vv;
		if (vv < mmin) mmin = vv;
	}
	mm /= (*npoints);
	threshold = mm + (mmax - mm) * 0.8;
	
	for (i=0; i<ne; i++) {
		ii = eps[i];
		x1 = points[ii]->x;
		y1 = points[ii]->y;
		z1 = points[ii]->z;
		r1 = points[ii]->r;
		for (j=i+1; j<ne; j++) {			
			jj = eps[j];
			mmax = 0;
			for (i=0;i<*npoints;i++) {
				if (mmax < points[i]->ID) mmax = points[i]->ID;
			}
			IDInds = (int *) malloc((mmax+1) * sizeof(int));
			flags = (int *) malloc((mmax+1) * sizeof(int));
			for (kk=0; kk<(mmax+1); kk++) flags[kk] = 0;
			for (i=0; i<*npoints; i++) IDInds[points[i]->ID] = i;
			if (checkConnected(points[ii], points[jj]->ID, (*npoints), points, IDInds, flags) == 1) continue; //these end points are connected. 
			free(flags);
			free(IDInds);
			x2 = points[jj]->x;
			y2 = points[jj]->y;
			z2 = points[jj]->z;
			r2 = points[jj]->r;
			dd = sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2)*zfact*zfact);
			if (dd > maxDistFillGaps) continue; //these points are too far. 	
			//try to connect these two points. 
			//get a small volume containing the end points. 
			xs = (int)fmax(0,fmin(x1,x2)-5);
			xe = (int)fmin(nx,fmax(x1,x2)+5);
			ys = (int)fmax(0,fmin(y1,y2)-5);
			ye = (int)fmin(ny,fmax(y1,y2)+5);
			zs = (int)fmax(0,fmin(z1,z2)-5);
			ze = (int)fmin(nz,fmax(z1,z2)+5);
			nxx = xe - xs;
			nyy = ye - ys;
			nzz = ze - zs;
			for (i1=0; i1<nxx; i1++) {
				for (j1=0; j1<nyy; j1++) {
					for (k1=0; k1<nzz; k1++) {
						kk = getIndex3D(i1,j1,k1,nxx,nyy,nzz);
						im3dS[kk] = im3d[getIndex3D(i1+xs,j1+ys,k1+zs,nx,ny,nz)];
						dists3d[kk] = 0.0;
					}
				}
			}
			//get the distance map. 
			dijstraComputeDists3D((int)(x1-xs), (int)(y1-ys), (int)(z1-zs), nxx, nyy, nzz, zfact, im3dS, dists3d, heapAllocated);
			//get the shortest path
			np = dijstraComputePath3D((int)(x2-xs), (int)(y2-ys), (int)(z2-zs), nxx, nyy, nzz, dists3d, x, y, z); 
			//check validity of the path.
			flag = 0;
			for (kk=0; kk<np; kk++) {
				if (im3d[getIndex3D(x[kk],y[kk],z[kk],nx,ny,nz)] > threshold) {
					flag = 1;
					break;
				}
			}
			flag = 1;
			if (flag == 1) continue;
			
			//connect the end points. 
			//smooth x, y, z
			gaussianFilter2D(1,np,1,PARAMS.WySmooth,(float *)x,(float *)x,PARAMS.sigmaSmooth);
			gaussianFilter2D(1,np,1,PARAMS.WySmooth,(float *)y,(float *)y,PARAMS.sigmaSmooth);
			gaussianFilter2D(1,np,1,PARAMS.WySmooth,(float *)z,(float *)z,PARAMS.sigmaSmooth);

			//get the list of points to create LinkedPoints. 
			float mr = (r1+r2)/2;		//average radius along the path
			int maxdp = (int) 2 * mr;
			kk = 0;
			int ipr = jj;
			while (1) {
				kk += maxdp;
				if (kk > np) break;
				//create linked points. 
				points[(*npoints)] = CreateLinkedPoint((int)x[kk]+xs,(int)y[kk]+ys,(int)z[kk]+zs,mr,(*npoints),3);
				AddConnLinkedPoint(points[ipr],(*npoints));
				AddConnLinkedPoint(points[(*npoints)],points[ipr]->ID);
				ipr = (*npoints);
				(*npoints)++;
			}
			AddConnLinkedPoint(points[ii],points[ipr]->ID);
			AddConnLinkedPoint(points[ipr],points[ii]->ID);	
			break; //allow only one possible path to be connected. 
		}
	}	
	free(eps);
	free(im3dS);
	free(dists3d);
	free(x);
	free(y);
	free(z);
	free(r);						
	free(heap); heap = NULL;
}
 
DLinkedListSWC* GetLastInDListSWC(DLinkedListSWC *dlist)
{
	DLinkedListSWC *p;
	p = dlist;
	if (p == NULL) return NULL;
	while (p->next != NULL) {
		p = p->next;
	}
	return p;	
}

void AppendToDListSWC(DLinkedListSWC **dlist, LinkedPoint *P) 
{
	DLinkedListSWC *newp,*last;
	newp = (DLinkedListSWC *) malloc(sizeof(DLinkedListSWC));	
	newp->P.x = P->x;
	newp->P.y = P->y;
	newp->P.z = P->z;
	newp->P.r = P->r;
	newp->P.ID = P->ID;
	newp->P.Type = P->Type;
	newp->next = NULL;
	newp->prev = NULL;
	if ((*dlist) == NULL) {
		newp->P.parentID = -1;
		(*dlist) = newp;
	} else {
		last = GetLastInDListSWC((*dlist));
		last->next = newp;
		newp->prev = last;
		newp->P.parentID = last->P.ID;
	}
	return;
}
	
void DeleteDListSWC(DLinkedListSWC *dlist)
{
	DLinkedListSWC *p, *next;
	if (dlist == NULL) return;
	p = dlist;
	while (p != NULL) {
		next = p->next;
		free(p);
		p = next;
	}
	dlist = NULL;
}

void DeleteFromListSWC(DLinkedListSWC *dlist, DLinkedListSWC *pdel)
{
	if (pdel->prev == NULL && pdel->next == NULL) {
		dlist = NULL;
	} else {
		if (pdel->prev == NULL) {
			pdel->next->prev = NULL;
		} else if (pdel->next == NULL) {
			pdel->prev->next = NULL;
		} else {
			pdel->prev->next = pdel->next;
			pdel->next->prev = pdel->prev;
		}
	}		
	free(pdel);	
}


// this function gets all branches in the linked points starting from jj.
int getAllChildBranchesLinkedPoints(int jj,int npoints,LinkedPoint **points,int *IDInds,int *flags, int *nbrp, DLinkedListSWC **branches, LinkedList **branchConnIDs)
{
	DLinkedListSWC *br, *biter;
	int i,kk,iid,bID;
	LinkedList *iter;
	DLinkedListSWC *last;
	LinkedList *brIDs;
	int nn=0, njj[1000];	//maximum number of connections is assumed to be 1000.  
	
	int nInBr; 
	
	br = NULL;
	last = NULL;
	brIDs = NULL;
	biter = NULL;
	iter = NULL;
	
	while (1) {
		AppendToDListSWC(&br,points[jj]);
		flags[jj] = 1;
		// find the next point.
		nn = 0;
		iter = points[jj]->conn->next;
		while (iter != NULL) {
			kk = iter->val;
			jj = IDInds[kk];
			if (flags[jj] == 0) {
				njj[nn++] = jj;
			}				
			iter = iter->next;
		}	
		if (nn ==0) {
			break;
		} else if (nn == 1) {
			jj = njj[0];
		} else {
			for (i=0; i<nn; i++) {
				iid = njj[i];
				bID = getAllChildBranchesLinkedPoints(iid,npoints,points,IDInds,flags,nbrp,branches,branchConnIDs);
				if (bID != -1) {
					AppendToList(&brIDs,bID);
				}
			}
			break;
		}
	}
	//check the branch length. 
	nInBr = 0;
	biter = br;
	while (biter != NULL) {
		++nInBr;
		biter = biter->next;
	}
	if (nInBr < PARAMS.minNumPointsBr) {
		DeleteList(brIDs);
		DeleteDListSWC(br);
		return -1;
	} else {
		branches[*nbrp] = br;
		branchConnIDs[*nbrp] = brIDs;
		*nbrp += 1;			
		return *nbrp-1;
	}
}

int compBranchObjSize(int ii,LinkedList **branchConnIDs,int *brLen, int no)
{
  int jj;
  LinkedList *brID;

  no += brLen[ii];
  brID = branchConnIDs[ii];
  while (brID != NULL) {
    jj = brID->val;
    no = compBranchObjSize(jj,branchConnIDs,brLen,no);
    brID = brID->next;
  }
  return no;
}

void markDeleteBranches(int ii,LinkedList **branchConnIDs,int *delMark)
{
  int jj;
  LinkedList *brID;

  delMark[ii] = 1;
  brID = branchConnIDs[ii];
  while (brID != NULL) {
    jj = brID->val;
    markDeleteBranches(jj,branchConnIDs,delMark);
    brID = brID->next;
  }
}

//create SWC from linked points. 
void createSWC(int npoints, LinkedPoint **points, const char *outName)
{
  int *IDInds, *numConn, *flags, i, ii, jj, nbr, mmin;
  DLinkedListSWC **branches, *br, *iter;
  LinkedList **branchConnIDs, *brID;
  int maxID = 0;
  FILE *fid;
  int *brLen, no, *delMark;

  numConn = (int *)malloc(npoints * sizeof(int));
  flags = (int *)malloc(npoints * sizeof(int));
  branchConnIDs = (LinkedList **)malloc(npoints * sizeof(LinkedList *));
  branches = (DLinkedListSWC **) malloc(npoints * sizeof(DLinkedListSWC *));
  for (i=0; i<npoints; i++) {
    if (maxID < points[i]->ID) maxID = points[i]->ID;
    branches[i] = NULL;
    branchConnIDs[i] = NULL;
  }
  IDInds = (int *)malloc((maxID+1) * sizeof(int));

  for (i=0; i<npoints; i++) {
    IDInds[points[i]->ID] = i;
    numConn[i] = NumConnLinkedPoint(points[i]);
    flags[i] = 0;
  }
  nbr = 0;        
  while (1) {
    jj = -1;
    mmin = 1e5;        
    for (i=0; i<npoints; i++) {
      if (flags[i] == 0 && mmin > numConn[i]) {
        mmin = numConn[i];
        jj = i;
      }
    }
    if (jj == -1) break;
    getAllChildBranchesLinkedPoints(jj,npoints,points,IDInds,flags,&nbr,branches,branchConnIDs);
  }

  /* connect branches*/
  for (ii=0; ii<nbr; ii++) {
    br = branches[ii]; 
    brID = branchConnIDs[ii];
    while (brID != NULL) {
      jj = brID->val;
      if (branches[jj] == NULL) continue;
      branches[jj]->P.parentID = GetLastInDListSWC(br)->P.ID;
      brID = brID->next;
    }
  }

  /*delete small objects. */
  brLen = (int *)malloc(nbr * sizeof(int));
  delMark = (int *)malloc(nbr * sizeof(int));    
  for (ii=0; ii<nbr; ii++) {
    brLen[ii] = 0;
    delMark[ii] = 0;
    iter = branches[ii];
    while (iter != NULL) {
      brLen[ii]  += 1;
      iter = iter->next;
    }
  }
  /*get objtects*/
  for (ii=0; ii<nbr; ii++) {
    if (branches[ii]->P.parentID == -1) {
      no = compBranchObjSize(ii,branchConnIDs,brLen,0);
      if (no < PARAMS.minNumPointsBr) { //mark delete branches. 
        markDeleteBranches(ii,branchConnIDs,delMark);
      }
    }
  }

  printf("Saving SWC to %s\n",outName);
  fid = fopen(outName,"w");
  no = 0;
  for (i=0; i<nbr; i++) {
    if (delMark[i] == 1) continue;    //this branch is marked to be deleted.
    no++; 
    iter = branches[i];
    while (iter != NULL) {
      fprintf(fid,"%d %d %f %f %f %f %d\n",iter->P.ID,iter->P.Type,iter->P.y,iter->P.x,iter->P.z,iter->P.r,iter->P.parentID);
      iter = iter->next;
    }
  }
  fclose(fid);
  printf("There %d branches\n",no);

  free(IDInds);
  free(numConn);
  free(flags);
  for (i=0; i<nbr; i++) {
    DeleteDListSWC(branches[i]);
    DeleteList(branchConnIDs[i]);
  }
  free(branchConnIDs);    
  free(branches);
  free(brLen);
  free(delMark);
}

void setParams(float *params)
{
	int ind;
	ind = 0;
	PARAMS.nSplit = (int) params[ind++];
	PARAMS.zfact = params[ind++];
	PARAMS.sparse= params[ind++];
	PARAMS.zJumpFact = params[ind++];
	PARAMS.expCompr = params[ind++];
	PARAMS.lambdaRatioThr = params[ind++];
	PARAMS.levelSetIter = (int) params[ind++];
	PARAMS.sigma = params[ind++];
	PARAMS.Wx = (unsigned int) params[ind++];
	PARAMS.Wy = (unsigned int) params[ind++];
	PARAMS.smallArea = (int) params[ind++];
	PARAMS.smallLen = (int) params[ind++];
	PARAMS.mindp = (int) params[ind++];
	PARAMS.distFact = params[ind++];
	PARAMS.minNumPointsBr = (int) params[ind++];
	PARAMS.WxSmooth = (unsigned int) params[ind++];
	PARAMS.WySmooth = (unsigned int) params[ind++];
	PARAMS.sigmaSmooth = params[ind++];
	PARAMS.maxDistFillGaps = (int) params[ind++];

	//print parameters. 
	printf("The parameters are:\n");
	printf("sigma=%f Wx=%d Wy=%d sparse=%f expCompr=%f lambdaRatioThr=%f levelSetIter=%d smallArea=%d smallLen=%d mindp=%d distFact=%f minNumPointsBr=%d zJumpFact=%f nSplit=%d, zfact=%f WxSmooth=%d WySmooth=%d sigmaSmooth=%f maxDistFillGaps=%d\n",
			PARAMS.sigma,PARAMS.Wx,PARAMS.Wy,PARAMS.sparse,PARAMS.expCompr,
			PARAMS.lambdaRatioThr,PARAMS.levelSetIter,PARAMS.smallArea,PARAMS.smallLen,
			PARAMS.mindp,PARAMS.distFact,PARAMS.minNumPointsBr,PARAMS.zJumpFact,PARAMS.nSplit,PARAMS.zfact,
			PARAMS.WxSmooth,PARAMS.WySmooth,PARAMS.sigmaSmooth,PARAMS.maxDistFillGaps);

}

//this function traces neuron in one tile. Called from the python code. 
//im3d contains the image stack. 
//ntot, size of the array linkedP.  
//linkedP must have enough storage allocated when calling from python. 
//The required storage is [7+(number of connections each point)]*number of linked points. 
//A safe bet is nx*ny*nz*(7 + 5), although this is probably a over kill. 
//parameters are read from file NeuTuAutoParameters.dat. 
//returns number of items in linkedP.
int autoTraceOneStack(int nx, int ny, int nz, float *im3d, int ntot, float *linkedP, float *params)
{
	char *line;
	int i,j,k,nxy,ii;
	float *imFlat,maxI,minI;
	int islab,z1,z2,z1ext,z2ext;		
	float *bw;
	LinkedPoint **points;
	int npoints;
	FILE *fpt;
	ssize_t read;	
	size_t len;
	int ns;
	LinkedList *iter;
	char outFilename[1000];

	setParams(params);

	heap = NULL; //initialize the heap pointer. 			 
	ntot = nx * ny * nz;	
	nxy = nx * ny;
	imFlat = (float *) malloc(nxy * sizeof(float));
	bw = (float *) malloc(nxy * sizeof(float));

	//pointer arrary of LinkedPoints used in creating swc file. 
	points = (LinkedPoint **) malloc(ntot * sizeof(LinkedPoint *));
	npoints = 0;

	//rescale.
	maxI = -FLT_MAX;
	minI = FLT_MAX;
	for (i=0; i<ntot; i++) {
		if (maxI < im3d[i]) maxI = im3d[i];
		if (minI > im3d[i]) minI = im3d[i];
	}
	for (i=0; i<ntot; i++) {
		im3d[i] = (im3d[i] - minI)/(maxI - minI + 1e-10); 
	}	
	
	for (islab=0;islab <PARAMS.nSplit;islab++) {
		z1 = nz/PARAMS.nSplit * islab;
		z2 = (int) fmin(nz,nz/PARAMS.nSplit * (islab+1));	//work on sub-slab
		z1ext = (int) fmax(0,z1 - 10);				//extended sub-slab
		z2ext = (int) fmin(nz,z2 + 10);
		//minimum intensity projection. 
		for (i=0; i<nx; i++) {
			for (j=0; j<ny; j++) {
				minI = FLT_MAX;
				for (k=z1ext; k<z2ext; k++) {
					ii = getIndex3DZfirst(i,j,k,nx,ny,nz);
					if (minI > im3d[ii]) minI = im3d[ii];
				}
				ii = getIndex(i,j,nx,ny);
				imFlat[ii] = minI;
			}
		}
		
		//create mask. 
		createMask2D(nx,ny,imFlat,bw);
		//sprintf(outFilename,"temp%d.tif",islab);
		//createGrayTiffStackFromFloatArray(nx,ny,1,imFlat,outFilename);
		
		//get the distance map, save in imFlat.
		sedt(nx, ny, bw, imFlat);
				
		//get skeleton. 
		//printf("Getting skeleton...\n");
		skeletonizeZS(nx,ny,bw,bw);
		//prune small branches. 
		printf("Pruning small branches...\n");
		pruneSmallBranches(nx,ny,bw,PARAMS.smallLen); 	
		
		//get Linked points from the skeleton.
		//getLinkedPoints
		printf("Getting linked points...\n");
		getLinkedPointsFromSkeletonAnd3D(nx,ny,nz,bw,imFlat,im3d,z1,z2,z1ext,z2ext,&npoints,points,PARAMS.zfact);
		printf("npoints = %d\n",npoints);	
	}
	//free some memory. 
	free(imFlat);
	free(bw);	
	
	//connect end points using distance. 
	//printf("Connecting end points...\n");
	//connectEndPoints(nx, ny, nz, im3d, npoints,points,PARAMS.zfact);
	
	//fill gaps between end points using 3D shortest path. 
	//fillGapsEndPoints(nx,ny,nz,im3d,&npoints,points,PARAMS.zfact);

	//package linked points to linkedP.
	ns = 0;
	for (i=0; i<npoints; i++) {
		linkedP[ns++] = points[i]->ID;
		linkedP[ns++] = points[i]->x;
		linkedP[ns++] = points[i]->y;
		linkedP[ns++] = points[i]->z;
		linkedP[ns++] = points[i]->r;
		linkedP[ns++] = points[i]->Type;
		iter = points[i]->conn;
		linkedP[ns++] = iter->val;
		iter = iter->next;
		while (iter != NULL) {
			linkedP[ns++] = iter->val;
			iter = iter->next;
		}
	}
	//final release memeory	
	for (i=0; i<npoints; i++) {
		DeleteLinkedPoint(points[i]);
	}
	free(points);	
	return ns;
}

void NeuTuAutoTraceOneStack(float *im3d, int nx, int ny, int nz,
    const char *paramFile, const char *outName)
{
  readParam(paramFile);

  int nSplit = PARAMS.nSplit;
  float zfact = PARAMS.zfact;

  int ntot = nx * ny * nz;
  int nxy = nx * ny;

  float *imFlat = new float[nxy];
  float *bw = new float[nxy];

  int npointsTot = (int)(ntot * 0.2);
  typedef LinkedPoint* LinkedPointPtr;
  LinkedPoint **points = new LinkedPointPtr[npointsTot];

  points = (LinkedPoint **) malloc(npointsTot * sizeof(LinkedPoint *));
  for (int i=0; i<npointsTot; i++) {
    points[i] = NULL;
  }
  int npoints = 0;

  //smooth each plane in the tiff stack
  float sigma = 1.0;

  unsigned int Wx = 5; 
  unsigned int Wy = 5;
  for (int k=0; k<nz; k++) {
    int i = k * nxy;
    gaussianFilter2D(nx, ny, Wx, Wy, im3d+i, imFlat, sigma);
    for (int i=0;i<nxy;i++) im3d[k*nxy+i] = imFlat[i];
  }		

  for (int islab=0;islab <nSplit;islab++) {
    int z1 = nz/nSplit * islab;
    int z2 = (int) fmin(nz,nz/nSplit * (islab+1));	//work on sub-slab
    int z1ext = (int) fmax(0,z1 - 10);				//extended sub-slab
    int z2ext = (int) fmin(nz,z2 + 10);
    //maximum intensity projection. 
    for (int i=0; i<nx; i++) {
      for (int j=0; j<ny; j++) {
        float minI = FLT_MAX;
        for (int k=z1ext; k<z2ext; k++) {
          int ii = getIndex3DZfirst(i,j,k,nx,ny,nz);
          if (minI > im3d[ii]) minI = im3d[ii];
        }
        int ii = getIndex(i,j,nx,ny);
        imFlat[ii] = minI;
      }
    }
    //sprintf(outName,"%s.imFlat%d.tif",filename,islab);
    //printf("Saving maximum projection for slab %d to %s...\n",islab,outName);
    //createGrayTiffStackFromFloatArray(nx,ny,1,imFlat,outName);	

    //create mask. 
    createMask2D(nx,ny,imFlat,bw);
    //sprintf(outName,"%s.imFlat%d.Mask.tif",filename,islab);
    //printf("Saving mask for slab %d to %s...\n",islab,outName);
    //createGrayTiffStackFromFloatArray(nx,ny,1,bw,outName);	

    //get the distance map, save in imFlat.
    sedt(nx, ny, bw, imFlat);

    //get skeleton. 
    //printf("Getting skeleton...\n");
    skeletonizeZS(nx,ny,bw,bw);
    //prune small branches. 
    printf("Pruning small branches...\n");
    pruneSmallBranches(nx,ny,bw,PARAMS.smallLen); 	

    //sprintf(outName,"%s.imFlat%d.Skeleton.tif",filename,islab);
    //printf("Saving skeleton for slab %d to %s...\n",islab,outName);
    //createGrayTiffStackFromFloatArray(nx,ny,1,bw,outName);	

    //get Linked points from the skeleton.
    //getLinkedPoints
    printf("Getting linked points...\n");
    getLinkedPointsFromSkeletonAnd3D(nx,ny,nz,bw,imFlat,im3d,z1,z2,z1ext,z2ext,&npoints,points,zfact);
  }

  printf("npoints=%d\n",npoints);

  //connect end points using distance. 
  printf("Connecting end points...\n");
  connectEndPoints(nx, ny, nz, 0, npoints,points,zfact);

  //fill gaps between end points using 3D shortest path. 
  //fillGapsEndPoints(nx,ny,nz,im3d,&npoints,points,zfact);

  printf("Connecting end points...\n");
  connectEndPoints(nx, ny, nz, 0, npoints,points,zfact);

  //create swc from the linked points. 
  printf("Creating swc file...\n");
  //sprintf(outName,"%s.auto.swc",filename);
  createSWC(npoints, points, outName);

  //final release memeory	
  delete []imFlat;
  delete []bw;

  for (int i=0; i<npoints; i++) {
    DeleteLinkedPoint(points[i]);
  }
  delete []points;
}


