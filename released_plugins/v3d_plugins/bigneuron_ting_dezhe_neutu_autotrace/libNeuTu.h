/*
These are C-functions used in NeuTuRelease.py
 
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

#ifndef LIBNEUTU_H_INCLUDED
#define LIBNEUTU_H_INCLUDED

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <stdint.h>
//#include "image.h"

//save image arrays. 
void saveImageData(char *filename, int nx, int ny, int nz, float *im3d, float *imFlat, float *imFlatRGB);

//read image arrays. 
void readImageData(char *filename, int nx, int ny, int nz, float *im3d, float *imFlat, float *imFlatRGB);

//probe tiff stack properties using Ting's program. 
void getTiffAttribute(char *filename, int *tiffinfo);

//create gray scale tiff stack from float arrays
//im3d is the gray scale ndarray in python generated in NeuTu.py
void createGrayTiffStackFromFloatArray(int nx, int ny, int nz, float *im3d, char *outFilename);

//create tiff stack from slices using libtiff.
void createTiffStackFromSlices(int nfiles, char **sliceFilenames, char *outFilename);

//read tiff stack using Ting's program. 
void readTiffStack(char *filename, int *tiffinfo, float *im3dR, float *im3dG, float *im3dB);

//1d index of 2d index (i,j)
int getIndex(int i, int j, int nx, int ny);

//1d index of 2d index (i,j)
int getIndex3D(int i, int j, int k, int nx, int ny, int nz);

float pixelDistance(int i, int j, float img, int ii, int jj, float imgFrom);

float pixelDistanceZ(int i, int j, float img, int ii, int jj, float imgFrom, float zfact);

//compute the shortest distance using Dijstra's algorithm. Computes shortest distance from one point to all other points. 
void dijstraComputeDists(int is, int js, int nx, int ny, float *img, float *dists); 
//compute the shortest path from (is,js) to (ie,je) given the distances from (is,js)
int dijstraComputePath(int ie, int je, int nx, int ny, float *dists, float *bw); 
     
float pixelDistance3D(int i, int j, int k, float img, int ii, int jj, int kk, float imgFrom, float zfact);

//dijstra shortest distance in 3D image, starting from point (is, js, ks)
//set healAllocated = 1 if heap memeory is allocated before calling this function. Useful when repeated called. 
void dijstraComputeDists3D(int is, int js, int ks, int nx, int ny, int nz, float zfact, float *img3d, float *dists3d, int heapAllocated); 

//compute the shortest path from (is,js,ks) to (ie,je,ke) given the distances from (is,js,ks)
//note that dists3d contains shortest distances to all points from (is,js,ks), and is computed using dijstraComputeDists3D.
//the path is stored in x, y, z. This array must be allocated when callig this function. 
int dijstraComputePath3D(int ie, int je, int ke, int nx, int ny, int nz, float *dists3d, float *x, float *y, float *z); 

//This function builds distance matrix starting from one point on the left.
void computeDists(int ix, int nx, int ny, float zfact, float *img, float *dists);

//This program computes the shortest path from all points from y=0 line to y = ny line. The path steps forward. from left to right
void shortestPathImageLeftToRight(int nx, int ny, float *img, float* ypath, float zfact);

//kruskal's minimum spanning tree algorithm
void kruskal(int n, float distThreshold, float *D, int *E);

//Gradient vector flow functions. 
//1D case. 
//parameters, n, dimension of the vectors
//v, vector of n, gradient vector field
//Ix, gradient of the image. 
//mu, parameter for controlling the smoothness of v
//dt, time step, dx, lattice space, maxIter, maximum iteration number. 
void gvf1d(int n, float *v, float *Ix, float mu, float dt, float dx, int maxIter);
//2D case. 
//parameters, m, n, dimensions of the images
//v, m x n array, gradient vector field, x component
//u, m x n array, gradient vector field, y component
//Ix, m x n array, gradient of the image, x compoment 
//Iy, m x n array, gradient of the image, y compoment 
//mu, parameter for controlling the smoothness of v
//dt, time step, dx, dy lattice space, maxIter, maximum iteration number. 
//Important, I should be normalize such that its maximum value is close to 1. Otherwise the convergence can suffer. 
void gvf2d(int m, int n, float *v, float *u, float *Ix, float *Iy, float mu, float dt, float dx, float dy, int maxIter);

//region growth method, turn on pixels using the growth method. 
void regionGrowth(int nx, int ny, float *bw, float *imFlat, float threshold);

//exact squared Euclidean distance transformation. 
//bw contains the binary image, and edt is the distance map. 
void sedt(int nx, int ny, float *bw, float *edt);

//gaussian filter in 2d, modified from Vaa3D plugin gaussianfilter. 
void gaussianFilter2D(int nx, int ny, unsigned int Wx, unsigned int Wy, float *imgIn, float *img, float sigma);

//level set smoothing of a binary mask bw based on the edge strength. 
void levelsetSmoothing(int nx, int ny, float *bw, float *edge,int iter);

//get threshold by getting the point of fractions of sparse pixels with large intensity are selected. 
float getSparseThreshold(int nx, int ny,float *bw,float sparse);

//more fancy thresholding based in the how the sparseness changes with the threshold. 
float getSparseThresholdAdaptive(int nx, int ny,float *bw,float sparseUp);

//binary image operations. 
//labeling connected pixels in binary image using two-pass algorithm based on Wikipedia description.
//http://en.wikipedia.org/wiki/Connected-component_labeling
int labelObjectsBinaryImage(int nx, int ny, float *bw, float *labels);

// Zhang-Suen skeletonization 
// src The source image, binary with range = [0,1.0]
// dst The destination image
//
void skeletonizeZS(int nx, int ny, float *src, float *dst);

//prune small branches in a skeleton. 
void pruneSmallBranches(int nx, int ny, float *bw, float smallLen); 

//data structures /////

//Modified heap data structure code from http://www.thelearningpoint.net/computer-science/data-structures-heaps-with-c-program-source-code
/*Declaring heap globally so that we do not need to pass it as an argument every time*/
/* Heap used here is Min Heap */
struct valInd
{
	float value;
	int ind;
};

/*Initialize Heap*/
void Init();
/*Insert an element into the heap */
//if the element index exists, relpalce the old value and update the heap.
//assuming the updated value is smaller than before.  
void InsertOrUpdate(struct valInd element);
struct valInd DeleteMin();

//single linked list of integers
typedef struct LinkedList LinkedList;
struct LinkedList {
	LinkedList *next; //pointer to the next in the list. 
	int val;		//rank of tree for this node. 
};
LinkedList* GetLastInList(LinkedList *list);
void AppendToList(LinkedList **list, int val);
void DeleteList(LinkedList *list);
void DeleteFirstElem(LinkedList *list);

//double linked list of integers. 
typedef struct DLinkedList DLinkedList;
struct DLinkedList {
	DLinkedList *next; //pointer to the next in the lits. 
	DLinkedList *prev; //pointer to the parent. 
	int val;		//rank of tree for this node. 
};
DLinkedList* GetLastInDList(DLinkedList *dlist);
void AppendToDList(DLinkedList *dlist, int val);
void DeleteDList(DLinkedList *list);
void DeleteFromList(DLinkedList *dlist, DLinkedList *pdel);

//disjoint tree data structure of kruskal's minimum spanning algorithm.
//following wikipedia's description. 
typedef struct Node Node;
struct Node {
	Node *parent; //pointer to the parent. 
	int rank;	  //rank of tree for this node. 
	int data;	  //data attached to the node. 
};
void MakeSet(int n, Node *nodes);
Node* Find(Node *node); 					//find the root. 
void Union(Node *x, Node *y); 

//functions for autotracing neuron. 
struct NeuTuAutoParams //all parameters used for autotracing neuron. 
{
	    //PARAMETERS FOR GETTING THE 2D MASK.
    float sparse;                    //IMPOARTANT, upper limit of sparseness in determining the threshold. 
    float expCompr;                    //compression exponent. 
    float lambdaRatioThr;            //threshold for the ratio lambda1/lambda2 in Hessian to supress blob.  
    int levelSetIter;                //levelset smoothing number of iterations. 
    int smallArea;                    //number of pixels of small areas to be removed in the mask. 
    float sigma;                    //sigma for gaussian smoothing image
    unsigned int Wx, Wy;            //window size for gaussian filter.

    //PARAMETERS FOR CREATING SWC. 
    int smallLen;                    //length smaller than this in skeleton are pruned. 
    int mindp;                        //minimum distance between the points in pixel
    float distFact;                    //sets the minimum distance for connecting points, distFact *(r1 + r2), where r1, r2 are the radii of the points to be connected.          
    int minNumPointsBr;             //minimum number of points in a branch for accepting. 
    float zJumpFact;                 //the factor for judging two larg z jump, zJumpFact * zfact * (points[npoints-1]->r + points[npoints]->r)
    unsigned int WxSmooth, WySmooth;//Window size for smoothing x, y, z and radius of te branches. 
    float sigmaSmooth;                //sigma of smoothing the branches. 
    int maxDistFillGaps;            //maximum distance in pixels for filling in gaps between end points. 
    float minRadius;                //in pixels, minimum radius for accepting SWC point. Useful for deleting noisy points. 

    //PARAMETERS FOR OVERALL PROCESS STRUCTURE. 
    int nSplit;                        //number of sub slabs for getting 2D mask. 
    float zfact;                    //ratio between z distance and the xy pixel distance, zDist/xyDist. 
};


int getIndex3DZfirst(int i, int j, int k, int nx, int ny, int nz);

//create mask bw from image I.    
void createMask2D(int nx, int ny, float *I, float *bw);

//data structure and functions for createing SWC file from skeletons. 
struct LinkedPoint
{
	float x, y, z, r;
	int ID, Type;
	LinkedList *conn;	
};

typedef struct LinkedPoint LinkedPoint;

//create a linked point. 
LinkedPoint *CreateLinkedPoint(float x, float y, float z, float r, int ID, int Type);
//delete linked point.
void DeleteLinkedPoint(LinkedPoint *p);
void AddConnLinkedPoint(LinkedPoint *p, int ID);
void DelConnLinkedPoint(LinkedPoint *p, int ID);
int NumConnLinkedPoint(LinkedPoint *p);

//get Linked points from the skeleton.
//bw contains the skeleton in the binary image, distMap contains the distance map.  
void getLinkedPointsFromSkeletonAnd3D(int nx,int ny,int nz,float *bw, float *distMap, float *im3d,int z1,int z2,int z1ext,int z2ext,int *npoints,LinkedPoint **points, float zfact);

//return 1 if LinkedPoint p is connected with a point with ID.  
int checkConnected(LinkedPoint *p, int ID, int npoints, LinkedPoint **points, int *IDInds, int*flags);

//connect end points in the linked points. 
void connectEndPoints(int nx, int ny, int nz, float *im3d, int npoints, LinkedPoint **points, float zfact);

//fill gaps between end points using 3D shortest path. 
void fillGapsEndPoints(int nx, int ny, int nz, float *im3d, int *npoints, LinkedPoint **points, float zfact);

//SWC Points, and double linked list for SWC points.

struct SWCP
{
	float x, y, z, r;
	int parentID, ID, Type;
};

typedef struct SWCP SWCP;
typedef struct DLinkedListSWC DLinkedListSWC;
struct DLinkedListSWC {
	DLinkedListSWC *next; //pointer to the next in the lits. 
	DLinkedListSWC *prev; //pointer to the parent. 
	SWCP P;			//value of the SWC point. 
};
DLinkedListSWC* GetLastInDListSWC(DLinkedListSWC *dlist);
void AppendToDListSWC(DLinkedListSWC **dlist, LinkedPoint *P);
void DeleteDListSWC(DLinkedListSWC *dlist);
void DeleteFromListSWC(DLinkedListSWC *dlist, DLinkedListSWC *pdel);
// this function gets all branches in the linked points starting from jj.
int getAllChildBranchesLinkedPoints(int jj,int npoints,LinkedPoint **points,int *IDs,int *flags, int *nbrp, DLinkedListSWC **branches, LinkedList **branchConnIDs);
//create SWC from linked points. 
void createSWC(int npoints, LinkedPoint **points, const char *outName);

void NeuTuAutoTraceOneStack(float *im3d, int nx, int ny, int nz,
    const char *paramFile, const char *outName);

#endif //LIBNEUTU_H_INCLUDED


