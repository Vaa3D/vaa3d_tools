// *****************************************************************************************
// generate cell statistics based on original gray scale image and segmenation mask
// function compute_cellStat is a copy of function compute_statistics_objects in ../v3d/my4dimage.cpp
// written by H. Peng
// 20081219
// *****************************************************************************************

#include "../../v3d_main/v3d/v3d_core.h"

bool compute_cellStat(My4DImage *grayimg, long c, My4DImage * maskimg, LocationSimple * & p_ano, long & n_objects)
{
	if (!grayimg || !grayimg->valid() || !maskimg || !maskimg->valid() || p_ano) //p_ano MUST be 0 as this function need to alocate memory for it
	{
		printf("The inputs of compute_statistics_objects() are invalid.\n");
		return false;
	}
	
	if (grayimg->getDatatype()!=V3D_UINT8 || (maskimg->getDatatype()!=V3D_UINT16))
	{
		printf("The input datatypes are invalid. The grayimg must be UINT8 and the maskimg must be UINT16.\n");
		return false;
	}
	
	if (grayimg->getXDim()!=maskimg->getXDim() || grayimg->getYDim()!=maskimg->getYDim() || grayimg->getZDim()!=maskimg->getZDim())
	{
		printf("The sizes of the grayimg and maskimg do not match in compute_statistics_objects().\n");
		return false;
	}
	
	if (c<0 || c>=grayimg->getCDim() || maskimg->getCDim()!=1)
	{
		printf("The input channel of grayimg is invalid or the maskimg has more than 1 channel.\n");
		return false;
	}
	
	//first find the largest index
	long i,j,k;
	
	unsigned short int *p_maskimg_1d = (unsigned short int *)(maskimg->getRawData());
	unsigned short int ***p_maskimg_3d = (unsigned short int ***)(((unsigned short int ****)maskimg->getData())[0]);
	unsigned char *p_grayimg_1d = (unsigned char *)(grayimg->getRawData()) + grayimg->getTotalUnitNumberPerChannel()*c;
	unsigned char ***p_grayimg_3d = (unsigned char ***)(((unsigned char ****)grayimg->getData())[c]);

	n_objects = 0;
	for (i=0;i<maskimg->getTotalUnitNumberPerChannel();i++)
		n_objects = (p_maskimg_1d[i]>n_objects)?p_maskimg_1d[i]:n_objects;
	
	n_objects += 1; //always allocate one more, as the object index starts from 1
	if (n_objects==1)
	{
		printf("The maskimg is all 0s. Nothing to generate!.\n");
		return false;
	}
	
	//then allocate memory and collect statistics
	
	try
	{
		p_ano = new LocationSimple [n_objects];
	}
	catch(...)
	{
		printf("Fail to allocate memory in compute_statistics_objects().\n");
		return false;
	}
	
	for (k=0;k<maskimg->getZDim();k++)
		for (j=0;j<maskimg->getYDim();j++)
			for (i=0;i<maskimg->getXDim();i++)
			{
				long cur_ind = p_maskimg_3d[k][j][i];
				if (p_grayimg_3d[k][j][i]==0) continue; //do not process 0 values, as it is background
				
				double cur_pix = double(p_grayimg_3d[k][j][i]);
				
				
				p_ano[cur_ind].size += 1;
				p_ano[cur_ind].mass += cur_pix;
				p_ano[cur_ind].sdev += cur_pix*cur_pix; //use the incremental formula
				if (cur_pix > p_ano[cur_ind].pixmax) p_ano[cur_ind].pixmax =  cur_pix;
				
				p_ano[cur_ind].x += i*cur_pix;
				p_ano[cur_ind].y += j*cur_pix;
				p_ano[cur_ind].z += k*cur_pix;
			}
		
	for (k=0;k<n_objects;k++)
	{
		if (p_ano[k].size>0)
		{
			p_ano[k].ave = p_ano[k].mass / p_ano[k].size;
			p_ano[k].sdev = sqrt(p_ano[k].sdev/p_ano[k].size - p_ano[k].ave*p_ano[k].ave); //use the incremental formula
			
			p_ano[k].x /= p_ano[k].mass;
			p_ano[k].y /= p_ano[k].mass;
			p_ano[k].z /= p_ano[k].mass;
		}
	}
	return true;
}
