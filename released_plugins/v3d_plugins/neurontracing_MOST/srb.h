/**************************************************************************

Rayburst Sampling Algorithm
This code is used to cast rays inside a 3D volumetric dataset.

Copyright (C) 2006 Computational Neurobiology and Imaging Center
Mount Sinani School of Medicine, New York NY
www.mssm.edu/cnic

Software Development: Douglas Ehlenberger and Alfredo Rodriguez

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

Please send any questions or comments to douglas.ehlenberger@mssm.edu


To compute diameter of tubular structure, call srb_run_2d().
To compute surface area and volume call srb_run_3d().

The SRBPARAMS structure contains the following input members:
unsigned char *image;          //pointer to volume data in row major order (one byte per voxel.)
int            image_width;    //width of data set or number of voxels in x dimension.
int            image_height;   //height of dataset or number of voxels in y dimension.
int            image_length;   //length of dataset or number of voxels in z dimension.
double         voxel_width;    //physical dimension of voxel (possibly in microns) along x.
double         voxel_height;   //physical dimension of voxel (possibly in microns) along y.
double         voxel_length;   //physical dimension of voxel (possibly in microns) along z.
double         threshold;      //intensity value used to identify boundary of structure
double         origin_x;       //position along x, in physical coordinates, where Rayburst is run.
double         origin_y;       //position along y, in physical coordinates, where Rayburst is run.
double         origin_z;       //position along z, in physical coordinates, where Rayburst is run.
double         fit_percent;    //amount of change (volume for 3d, area for 2d) to converge. 
int            max_iterations; //number of subdivisions in surface (3d) or perimeter (2d).

The srb_run3d() routine returns the volume and surface area of the Rayburst as part
of the SRBMESH structure. This structure has the following members:

int      vertex_count;   //number of vertices used to approximate surface
double  *vertices;       //pointer to xyz triplets with physical coordinates of all vertices.
int      triangle_count; //number of triagles used to approximate surface
int     *triangles;      //pointer to array of indices into vertices array to define triangles.
double   volume;         //computed volume of Rayburst.
double   surface_area;   //computed surface area of Rayburst.

The srb_run2d() routine returns the area and tubular diameter of the Rayburst as part
of the SRBFAN structure. This structure has the following members:

int      vertex_count; //number of vertices used to approximate fan.
double  *vertices;     //pointer to xyz triplets with physical coordinates of all vertices.
double   diameter;     //computed tubular diameter of Rayburst.
double   area;         //computed 2-dimensional area of Rayburst.

The srb_mesh_destroy() and srb_fan_destroy() free allocated memory associated
with each structure.

***************************************************************************/

#ifndef __SRB_H__
#define __SRB_H__


#include <stdlib.h>
#include <math.h>


typedef struct _SRBMESH
	{
	int      vertex_count;
	double  *vertices;
	int      triangle_count;
	int     *triangles;
	double   volume;
	double   surface_area;
	}
	SRBMESH;


typedef struct _SRBFAN
	{
	int      vertex_count;
	double  *vertices;
	double   diameter;
	double   area;
	}
	SRBFAN;


typedef struct _SRBPARAMS
	{
	unsigned char  *image;
    long             image_width;
    long             image_height;
    long             image_length;
	double          voxel_width;
	double          voxel_height;
	double          voxel_length;
	double          threshold;
	double          origin_x;
	double          origin_y;
	double          origin_z;
	double          fit_percent;
	int             max_iterations;
	}
	SRBPARAMS;


SRBMESH* srb_run_3d( SRBPARAMS* );
SRBFAN* srb_run_2d( SRBPARAMS* );

void srb_mesh_destroy( SRBMESH* );
void srb_fan_destroy( SRBFAN* );


#endif/* __SRB_H__ */
