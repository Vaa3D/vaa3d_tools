/*
 *  This file is for automatically tracing neurons using 2D project and tracing method. 
 *  Used for tracing neurites in one tiff stack.
 *  Written by Dezhe Z. Jin, dzj2@psu.edu
 *  06/02/2015, (C) Dezhe Jin
 * 
 * 	This program is free software: you can redistribute it and/or modify
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
 
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <stdint.h>
#include "image.h"
#include "libNeuTu.h"

#define OUTPUTMASK 1	//if set to 1, the mask will be saved. 

//trace neurites in a single tif stack. first argument, tif filename; second argument, parameter filename.   
int main(int argc, char *argv[])
{
	char *filename, *filenameParam, outName[1000],*line;
	int tiffinfo[5],nx,ny,nz,nchan,type,i,j,k,ntot,nxy,ii;
	float *im3d, *im3dG, *im3dB, *imFlat,maxI,minI;
	int nSplit,islab,z1,z2,z1ext,z2ext;		
	float *bw,sigma,smallLen,zfact;
	unsigned int Wx, Wy;
	LinkedPoint **points;
	int npoints;
	FILE *fpt;
	ssize_t read;	
	size_t len;
	int ind, npointsTot;
	#ifdef OUTPUTMASK
	float *bwCombined;
	#endif 
	
	if (argc > 1) {
		filename = argv[1];
		printf("Using  tiff image filename = %s\n",filename);
	} else {
		printf("ERROR: please sepcify the tiff stack filename as the first commandline argument.\n");
		return 1;
	}	  
	if (argc > 2) {				//read parameters from file
		filenameParam = argv[2];
		printf("Reading parameters from file %s\n",filenameParam);
		fpt = fopen(filenameParam,"r");
		ind = 0;
		line = 0;
		len = 0;
        while ((read = getline(&line, &len, fpt)) != -1) {
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
		PARAMS.sparse = 0.15;			//IMPOARTANT, upper limit of sparseness in determining the threshold. 
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
		PARAMS.nSplit = 3;				//number of sub slabs.
		PARAMS.zfact = 8.0;				//ration between z distance and the xy pixel distance. 		
	}
	//print parameters. 
	printf("The parameters are:\n");
	printf("sigma=%6.2f Wx=%d Wy=%d sparse=%6.2f expCompr=%6.2f lambdaRatioThr=%6.2f levelSetIter=%d smallArea=%d smallLen=%d mindp=%d distFact=%6.2f minNumPointsBr=%d zJumpFact=%6.2f nSplit=%d, zfact=%6.2f WxSmooth=%d WySmooth=%d sigmaSmooth=%6.2f maxDistFillGaps=%d\n",
			PARAMS.sigma,PARAMS.Wx,PARAMS.Wy,PARAMS.sparse,PARAMS.expCompr,
			PARAMS.lambdaRatioThr,PARAMS.levelSetIter,PARAMS.smallArea,PARAMS.smallLen,
			PARAMS.mindp,PARAMS.distFact,PARAMS.minNumPointsBr,PARAMS.zJumpFact,PARAMS.nSplit,PARAMS.zfact,
			PARAMS.WxSmooth,PARAMS.WySmooth,PARAMS.sigmaSmooth,PARAMS.maxDistFillGaps);

	nSplit = PARAMS.nSplit;			
	zfact = PARAMS.zfact;	
	
	heap = NULL; //initialize the heap pointer. 		
	 
	//read image. 
	getTiffAttribute(filename, tiffinfo);
	nx = tiffinfo[0];
	ny = tiffinfo[1];
	nz = tiffinfo[2];
	nchan = tiffinfo[3];
	type = tiffinfo[4];    
	printf("Image dimension=(%d %d %d) type=%d number of channels=%d\n",nx,ny,nz,type,nchan);
	ntot = nx * ny * nz;	
	nxy = nx * ny;

	im3d = (float *) malloc(ntot*sizeof(float));	
	imFlat = (float *) malloc(nxy * sizeof(float));
	bw = (float *) malloc(nxy * sizeof(float));
	#ifdef OUTPUTMASK
	bwCombined = (float *) malloc(nxy * sizeof(float));
	#endif

	//pointer arrary of LinkedPoints used in creating swc file. 
	
	npointsTot = (int)(ntot * 0.2);
	points = (LinkedPoint **) malloc(npointsTot * sizeof(LinkedPoint *));
	for (i=0; i<npointsTot; i++) points[i] = NULL;
	npoints = 0;

	if (nchan == 1) {
		printf("One chanel stack, assume dark field...\n");
		readTiffStack(filename, tiffinfo, im3d, NULL, NULL);
		//reschale. 
		maxI = -FLT_MAX;
		minI = FLT_MAX;
		for (i=0; i<ntot; i++) {
			if (maxI < im3d[i]) maxI = im3d[i];
			if (minI > im3d[i]) minI = im3d[i];
		}
		for (i=0; i<ntot; i++) {
			im3d[i] = (maxI - im3d[i])/(maxI - minI + 1e-10); //invert to bright field. 
			//im3d[i] = (im3d[i] - minI)/(maxI - minI + 1e-10); //invert to bright field. 
		}	
	} else if (nchan == 3) {
		printf("Color stack, assuming biocytin bright field...\n");
		im3dG = (float *) malloc(ntot*sizeof(float));	
		im3dB = (float *) malloc(ntot*sizeof(float));			
		readTiffStack(filename, tiffinfo, im3d, im3dG, im3dB);
		maxI = -FLT_MAX;
		minI = FLT_MAX;
		for (i=0; i<ntot; i++) {
			im3d[i] = 0.2126 * im3d[i] + 0.7152 * im3dG[i] + 0.0722 * im3dB[i];
			if (maxI < im3d[i]) maxI = im3d[i];
			if (minI > im3d[i]) minI = im3d[i];
		}
		for (i=0; i<ntot; i++) {
			im3d[i] = (im3d[i] - minI)/(maxI - minI + 1e-10); 
		}	
		free(im3dG);
		free(im3dB);
	} else {
		printf("ERROR: The number of channels in the image is currently assumed to be 1 or 3. \n");
		return 1;
	}	
	
	//smooth each plane in the tiff stack
	sigma = 1.0;
	Wx = 5; Wy = 5;
	for (k=0; k<nz; k++) {
		i = k * nxy;
		gaussianFilter2D(nx, ny, Wx, Wy, im3d+i, imFlat, sigma);
		for (i=0;i<nxy;i++) im3d[k*nxy+i] = imFlat[i];
	}		
	
	#ifdef OUTPUTMASK
	//initialze bwCombined
	for (i=0; i<nxy; i++) bwCombined[i] = 0.0;
	#endif
	
	for (islab=0;islab <nSplit;islab++) {
		z1 = nz/nSplit * islab;
		z2 = (int) fmin(nz,nz/nSplit * (islab+1));	//work on sub-slab
		z1ext = (int) fmax(0,z1 - 10);				//extended sub-slab
		z2ext = (int) fmin(nz,z2 + 10);
		//maximum intensity projection. 
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

		#ifdef OUTPUTMASK
		for (i=0; i<nxy; i++) {
			if (bw[i] > 0.0) bwCombined[i] = 1.0;
		}		
		#endif
	}

	printf("npoints=%d\n",npoints);
 
	#ifdef OUTPUTMASK
	sprintf(outName,"%s.imFlat.Mask.tif",filename);
	printf("Saving the combined mask to %s...\n",outName);
	createGrayTiffStackFromFloatArray(nx,ny,1,bwCombined,outName);
	free(bwCombined);
	#endif	

	//free some memory. 
	free(imFlat);
	free(bw);	

	//connect end points using distance. 
	printf("Connecting end points...\n");
	connectEndPoints(nx, ny, nz, 0, npoints,points,zfact);
	
	//fill gaps between end points using 3D shortest path. 
	fillGapsEndPoints(nx,ny,nz,im3d,&npoints,points,zfact);
	
	//create swc from the linked points. 
	printf("Creating swc file...\n");
	sprintf(outName,"%s.auto.swc",filename);
	createSWC(npoints, points, outName);
			
	//final release memeory	
	free(im3d);
	for (i=0; i<npoints; i++) {
		DeleteLinkedPoint(points[i]);
	}
	free(points);	
	return 0;
 }
 
 
 
