/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




//by Hanchuan Peng
//090705


#include "apo_xforms.h"

//basic neuron coordinate manipulation functions

void proc_apo_add_offset(QList <CellAPO> *p, double xs, double ys, double zs) //add the offset to each node's coordinates
{
	if (!p) return;
	if (xs==0 && ys==0 && zs==0) return;
	CellAPO * tp;
	for (V3DLONG i=0;i<p->size();i++)
	{
		tp = (CellAPO *)(&(p->at(i)));
		tp->x += xs;
		tp->y += ys;
		tp->z += zs;
	}
}

void proc_apo_multiply_factor(QList <CellAPO> *p, double fx, double fy, double fz) //add the scaling factor sf to each node's coordinates
{
	if (!p) return;
	if (fx==1 && fy==1 && fz==1) return;
	CellAPO * tp;
	V3DLONG i;
	double mmx, mmy, mmz, minv, maxv;
	
	if (fx!=1) 
	{
		minv = maxv = p->at(0).x;
		for (i=1;i<p->size();i++)
		{
			tp = (CellAPO *)(&(p->at(i)));
			if (minv > tp->x)  minv = tp->x;
			else if (maxv < tp->x)  maxv = tp->x;
		}
		mmx = (minv+maxv)/2.0;
		for (i=0;i<p->size();i++)
		{
			tp = (CellAPO *)(&(p->at(i)));
			tp->x -= mmx;
		}	
	}
	if (fy!=1) 
	{
		minv = maxv = p->at(0).y;
		for (i=1;i<p->size();i++)
		{
			tp = (CellAPO *)(&(p->at(i)));
			if (minv > tp->y)  minv = tp->y;
			else if (maxv < tp->y)  maxv = tp->y;
		}
		mmy = (minv+maxv)/2.0;
		for (i=0;i<p->size();i++)
		{
			tp = (CellAPO *)(&(p->at(i)));
			tp->y -= mmy;
		}	
	}
	if (fz!=1) 
	{
		minv = maxv = p->at(0).z;
		for (i=1;i<p->size();i++)
		{
			tp = (CellAPO *)(&(p->at(i)));
			if (minv > tp->z)  minv = tp->z;
			else if (maxv < tp->z)  maxv = tp->z;
		}
		mmz = (minv+maxv)/2.0;
		for (i=0;i<p->size();i++)
		{
			tp = (CellAPO *)(&(p->at(i)));
			tp->z -= mmz;
		}	
	}
	
	for (i=0;i<p->size();i++)
	{
		tp = (CellAPO *)(&(p->at(i)));
		if (fx!=1) {tp->x *= fx; tp->x += mmx;}
		if (fy!=1) {tp->y *= fy; tp->y += mmy;}
		if (fz!=1) {tp->z *= fz; tp->z += mmz;}
	}
}

void proc_apo_gmultiply_factor(QList <CellAPO> *p, double fx, double fy, double fz) 
{
	if (!p) return;
	if (fx==1 && fy==1 && fz==1) return;
	CellAPO * tp;
	V3DLONG i;
	
	for (i=0;i<p->size();i++)
	{
		tp = (CellAPO *)(&(p->at(i)));
		if (fx!=1) {tp->x *= fx; }
		if (fy!=1) {tp->y *= fy; }
		if (fz!=1) {tp->z *= fz; }
	}
}

void proc_apo_mirror(QList <CellAPO> *p, bool b_flip_x, bool b_flip_y, bool b_flip_z) //flip the neuron around its center for a dimension or all three dimensions
{
	if (!p) return;
	if (p->size()<=1) return;
	if (b_flip_x==false && b_flip_y==false && b_flip_z==false) return; 
	
	V3DLONG i;
	double mm, minv, maxv;
	
	CellAPO * tp;
	
	if (b_flip_x) 
	{
		minv = maxv = p->at(0).x;
		for (i=1;i<p->size();i++)
		{
			tp = (CellAPO *)(&(p->at(i)));
			if (minv > tp->x)  minv = tp->x;
			else if (maxv < tp->x)  maxv = tp->x;
		}
		mm = (minv+maxv);
		for (i=0;i<p->size();i++)
		{
			tp = (CellAPO *)(&(p->at(i)));
			tp->x = mm - tp->x;
		}	
	}
	if (b_flip_y) 
	{
		minv = maxv = p->at(0).y;
		for (i=1;i<p->size();i++)
		{
			tp = (CellAPO *)(&(p->at(i)));
			if (minv > tp->y)  minv = tp->y;
			else if (maxv < tp->y)  maxv = tp->y;
		}
		mm = (minv+maxv);
		for (i=0;i<p->size();i++)
		{
			tp = (CellAPO *)(&(p->at(i)));
			tp->y = mm - tp->y;
		}	
	}
	if (b_flip_z) 
	{
		minv = maxv = p->at(0).z;
		for (i=1;i<p->size();i++)
		{
			tp = (CellAPO *)(&(p->at(i)));
			if (minv > tp->z)  minv = tp->z;
			else if (maxv < tp->z)  maxv = tp->z;
		}
		mm = (minv+maxv);
		for (i=0;i<p->size();i++)
		{
			tp = (CellAPO *)(&(p->at(i)));
			tp->z = mm - tp->z;
		}	
	}
}

void proc_apo_affine(QList <CellAPO> *p, double afmatrix[16]) //affine transform using a 4x4 matrix
{
	if (!p || !afmatrix) return;
	
	//the matrix structure is
	//
	// a[0] a[1] a[2] a[3]
	// a[4] a[5] a[6] a[7]
	// a[8] a[9] a[10] a[11]
	// a[12] a[13] a[14] a[15]
	//
	// (a[12] to a[15] should be [0,0,0,1], but as they are not really used, does not matter)
	//
	// the transform is a * [x y z 1]'
	//
	
	CellAPO * tp;
	double x,y,z;
	for (V3DLONG i=0;i<p->size();i++)
	{
		tp = (CellAPO *)(&(p->at(i)));
		
		x = afmatrix[0] * tp->x + afmatrix[1] * tp->y + afmatrix[2] * tp->z + afmatrix[3];
		y = afmatrix[4] * tp->x + afmatrix[5] * tp->y + afmatrix[6] * tp->z + afmatrix[7];
		z = afmatrix[8] * tp->x + afmatrix[9] * tp->y + afmatrix[10] * tp->z + afmatrix[11];
		
		//should I also transform the radius as well? (because affine can make the radius change as well)?
		
		//now update
		tp->x = x;	tp->y = y; tp->z = z;
	}	
}

void getAPOCellListCenter(QList <CellAPO> *p, double &cx, double &cy, double &cz)
{
	if (!p || p->size()<=0) return;
	
	CellAPO * tp;
	V3DLONG i;
	double minv, maxv;
	{
		minv = maxv = p->at(0).x;
		for (i=1;i<p->size();i++)
		{
			tp = (CellAPO *)(&(p->at(i)));
			if (minv > tp->x)  minv = tp->x;
			else if (maxv < tp->x)  maxv = tp->x;
		}
		cx = (minv+maxv)/2.0;
	}
	{
		minv = maxv = p->at(0).y;
		for (i=1;i<p->size();i++)
		{
			tp = (CellAPO *)(&(p->at(i)));
			if (minv > tp->y)  minv = tp->y;
			else if (maxv < tp->y)  maxv = tp->y;
		}
		cy = (minv+maxv)/2.0;
	}
	{
		minv = maxv = p->at(0).z;
		for (i=1;i<p->size();i++)
		{
			tp = (CellAPO *)(&(p->at(i)));
			if (minv > tp->z)  minv = tp->z;
			else if (maxv < tp->z)  maxv = tp->z;
		}
		cz = (minv+maxv)/2.0;
	}
}

void proc_apo_affine_around_center(QList <CellAPO> *p, double afmatrix[16], double cx, double cy, double cz) //affine transform using a 4x4 matrix, but affine-transfrom around the center
{
	if (!p || !afmatrix) return;
	
	//first move the center of neuron volume to (0,0,0)
	
	CellAPO * tp;
	V3DLONG i;
	{
		for (i=0;i<p->size();i++)
		{
			tp = (CellAPO *)(&(p->at(i)));
			tp->x -= cx;
		}	
	}
	{
		for (i=0;i<p->size();i++)
		{
			tp = (CellAPO *)(&(p->at(i)));
			tp->y -= cy;
		}	
	}
	{
		for (i=0;i<p->size();i++)
		{
			tp = (CellAPO *)(&(p->at(i)));
			tp->z -= cz;
		}	
	}
	
	//the matrix structure is
	//
	// a[0] a[1] a[2] a[3]
	// a[4] a[5] a[6] a[7]
	// a[8] a[9] a[10] a[11]
	// a[12] a[13] a[14] a[15]
	//
	// (a[12] to a[15] should be [0,0,0,1], but as they are not really used, does not matter)
	//
	// the transform is a * [x y z 1]'
	//
	
	double x,y,z;
	for (i=0;i<p->size();i++)
	{
		tp = (CellAPO *)(&(p->at(i)));
		
		x = afmatrix[0] * tp->x + afmatrix[1] * tp->y + afmatrix[2] * tp->z + afmatrix[3];
		y = afmatrix[4] * tp->x + afmatrix[5] * tp->y + afmatrix[6] * tp->z + afmatrix[7];
		z = afmatrix[8] * tp->x + afmatrix[9] * tp->y + afmatrix[10] * tp->z + afmatrix[11];
		
		//should I also transform the radius as well? (because affine can make the radius change as well)?
		
		//now update
		tp->x = x;	tp->y = y; tp->z = z;
	}
	
	//now move the center back
	for (i=0;i<p->size();i++)
	{
		tp = (CellAPO *)(&(p->at(i)));
		{tp->x += cx;}
		{tp->y += cy;}
		{tp->z += cz;}
	}
	
	return;
}


void proc_apo_multiply_factor_radius(QList <CellAPO> *p, double sf) //scale the diameter of a neuron
{
	if (!p) return;
	CellAPO * tp;
	for (V3DLONG i=0;i<p->size();i++)
	{
		tp = (CellAPO *)(&(p->at(i)));
		tp->volsize *= sf*sf*sf;
	}	
}

