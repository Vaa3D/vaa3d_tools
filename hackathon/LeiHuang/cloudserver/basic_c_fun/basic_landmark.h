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




//basic_landmark.h
//
// Copyright: Hanchuan Peng (Howard Hughes Medical Institute, Janelia Farm Research Campus).
// The License Information and User Agreement should be seen at http://penglab.janelia.org/proj/v3d .
//
// Last edit. 2009-Aug-21
//



#ifndef __PT_LOCATION_H__
#define __PT_LOCATION_H__

#include <time.h>

#include <string>
using std::string;

#include "color_xyz.h"
#include "v3d_basicdatatype.h"

#define VAL_INVALID -9999

enum PxLocationUsefulness
{
	pxUnknown, pxLocaNotUseful, pxLocaUseful, pxLocaUnsure, pxTemp
};
enum PxLocationMarkerShape
{
	pxUnset,
	pxSphere,
	pxCube,
	pxCircleX,
	pxCircleY,
	pxCircleZ,
	pxSquareX,
	pxSquareY,
	pxSquareZ,
	pxLineX,
	pxLineY,
	pxLineZ,
	pxTriangle,
	pxDot
};

struct LocationSimple
{
	float x, y, z;
	float radius;
	PxLocationUsefulness inputProperty;
	PxLocationMarkerShape shape;

	double pixval;
	double ave, sdev, skew, curt;
	double size, mass, pixmax;
        double ev_pc1, ev_pc2, ev_pc3; //the eigen values of principal components
        XYZ mcenter; //mass center

	string name; //the name of a landmark
	string comments; //other info of the landmark
	int category; //the type of a particular landmark
	RGBA8 color;
	bool on;

	//public:
	void init()
	{
		x = 0;
		y = 0;
		z = 0;
		radius = 5;
		shape = pxSphere;
		inputProperty = pxLocaUseful;
		name = "";
		comments = "";
		category = 0;
		//color.r = color.g = color.b = color.a = 255;

		srand(clock()); //time(NULL));
		color = random_rgba8(255);

		ave = sdev = skew = curt = 0;
		size = mass = 0;
		pixmax = 0;

		ev_pc1=ev_pc2=ev_pc3=VAL_INVALID; //set as invalid value

                mcenter.x=0; mcenter.y=0; mcenter.z=0;

		on=true;
	}
    LocationSimple(float xx, float yy, float zz)
	{
		init();
		x = xx;
		y = yy;
		z = zz;
	}
	LocationSimple()
	{
		init();
	}
	void getCoord(int &xx, int &yy, int& zz)
	{
		xx = (int) x;
		yy = (int) y;
		zz = (int) z;
	}
	void getCoord(float &xx, float &yy, float& zz)
	{
		xx = x;
		yy = y;
		zz = z;
	}
	int getPixVal()
	{
		return pixval;
	}
	double getAve()
	{
		return ave;
	}
	double getSdev()
	{
		return sdev;
	}
	double getSkew()
	{
		return skew;
	}
	double getCurt()
	{
		return curt;
	}
	PxLocationUsefulness howUseful()
	{
		return inputProperty;
        }
        // == operator for comparison of landmarks
        bool operator==(const LocationSimple& rhs) const {
            return
                    (  (x == rhs.x)
                    && (y == rhs.y)
                    && (z == rhs.z) );
        }
};

struct PtIndexAndParents
{
	V3DLONG nodeInd;
	V3DLONG nodeParent;
	PtIndexAndParents()
	{
		nodeInd = -1;
		nodeParent = -1;
	}
	PtIndexAndParents(V3DLONG n, V3DLONG np)
	{
		nodeInd = n;
		nodeParent = np;
	}
};

#endif

