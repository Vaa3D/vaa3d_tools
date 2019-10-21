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




/*******************************************************************************************
 *
 * basic_4dimage_create.cpp
 *
 * separate from basic_4dimage.cpp and .h for for better modularity
 * by Hanchuan Peng, 20120413.
 *
 *******************************************************************************************
 */

#include "basic_4dimage.h"

#include <stdio.h>

bool Image4DSimple::createImage(V3DLONG mysz0, V3DLONG mysz1, V3DLONG mysz2, V3DLONG mysz3, ImagePixelType mytype)
{
	if (mysz0<=0 || mysz1<=0 || mysz2<=0 || mysz3<=0) return false; //note that for this sentence I don't change b_error flag
	if (data1d) {delete []data1d; data1d=0; sz0=0; sz1=0; sz2=0;sz3=0; datatype=V3D_UNKNOWN;}
	try //081001
	{
		switch (mytype)
		{
			case V3D_UINT8:
				data1d = new unsigned char [mysz0*mysz1*mysz2*mysz3];
				if (!data1d) {b_error=1;return false;}
				break;
			case V3D_UINT16:
				data1d = new unsigned char [mysz0*mysz1*mysz2*mysz3*2];
				if (!data1d) {b_error=1;return false;}
				break;
			case V3D_FLOAT32:
				data1d = new unsigned char [mysz0*mysz1*mysz2*mysz3*4];
				if (!data1d) {b_error=1;return false;}
				break;
			default:
				b_error=1;
				return false;
				break;
		}
		sz0=mysz0; sz1=mysz1; sz2=mysz2;sz3=mysz3; datatype=mytype; b_error=0; //note that here I update b_error
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in My4DImageSimple::createImage();\n");
		data1d = 0;
		b_error=1;
		return false;
	}
	return true;
}

void Image4DSimple::createBlankImage(V3DLONG imgsz0, V3DLONG imgsz1, V3DLONG imgsz2, V3DLONG imgsz3, int imgdatatype)
{
	if (imgsz0<=0 || imgsz1<=0 || imgsz2<=0 || imgsz3<=0)
	{
		v3d_msg("Invalid size parameters in createBlankImage(). Do nothing. \n");
		return;
	}
	if (imgdatatype!=1 && imgdatatype!=2 && imgdatatype!=4)
	{
		v3d_msg("Invalid image datatype parameter in createBlankImage(). Do nothing. \n");
		return;
	}
	
	//otherwise good to go
	
	cleanExistData(); /* note that this variable must be initialized as NULL. */
	
	strcpy(imgSrcFile, "Untitled_blank.v3draw");
	
	//==============
	
	switch (imgdatatype)
	{
		case 1:
			datatype = V3D_UINT8;
			break;
			
		case 2:
			datatype = V3D_UINT16;
			break;
			
		case 4:
			datatype = V3D_FLOAT32;
			break;
			
		default:
			v3d_msg("Something wrong with the program in My4DImageSimple::createBlankImage(), -- should NOT display this message at all. Check your program. \n");
			b_error=1;
			return;
	}
	
	sz0 = imgsz0;
	sz1 = imgsz1;
	sz2 = imgsz2;
	sz3 = imgsz3;
	
	V3DLONG totalbytes = sz0*sz1*sz2*sz3*imgdatatype;
	try { //081001
		data1d = new unsigned char [totalbytes];
	}catch (...) {data1d=0;}
	
	if (!data1d)
	{
		v3d_msg("Fail to allocate memory in My4DImageSimple::createBlankImage(). Check your program. \n");
		sz0=sz1=sz2=sz3=0; datatype=V3D_UNKNOWN;
		b_error=1;
		return;
	}
	
	for (V3DLONG i=0;i<totalbytes;i++) data1d[i] = 0;
	
	return;
}

bool convert_data_to_8bit(void * &img, V3DLONG * sz, int datatype)
{
	if (!img || !sz)
	{
		fprintf(stderr, "The input to convert_data_to_8bit() are invalid [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}
    
	if (datatype!=2 && datatype!=4)
	{
		fprintf(stderr, "This function convert_type2uint8_3dimg_1dpt() is designed to convert 16 bit and single-precision-float only [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}
    
	if (sz[0]<1 || sz[1]<1 || sz[2]<1 || sz[3]<1)
	{
		fprintf(stderr, "Input image size is not valid [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}
    
	V3DLONG totalunits = sz[0] * sz[1] * sz[2] * sz[3];
	unsigned char * outimg = new unsigned char [totalunits];
	if (!outimg)
	{
		fprintf(stderr, "Fail to allocate memory. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}
    
	if (datatype==2) //following is new method 090718, PHC
	{
		unsigned short int * tmpimg = (unsigned short int *)img;
		V3DLONG i; double maxvv=tmpimg[0];
		for (i=0;i<totalunits;i++)
		{
			maxvv = (maxvv<tmpimg[i]) ? tmpimg[i] : maxvv;
		}
		if (maxvv>255.0)
		{
			maxvv = 255.0/maxvv;
            for (/*V3DLONG*/ i=0;i<totalunits;i++)
			{
				outimg[i] = (unsigned char)(double(tmpimg[i])*maxvv);
			}
		}
		else
		{
            for (/*V3DLONG*/ i=0;i<totalunits;i++)
			{
				outimg[i] = (unsigned char)(tmpimg[i]); //then no need to rescale
			}
		}
	}
	else
	{
		float * tmpimg = (float *)img;
		V3DLONG i; double maxvv=tmpimg[0], minvv=tmpimg[0];
		for (i=0;i<totalunits;i++)
		{
			if (tmpimg[i]>maxvv) maxvv = tmpimg[i];
			else if (tmpimg[i]<minvv) minvv = tmpimg[i];
		}
		if (maxvv!=minvv)
		{
			double w = 255.0/(maxvv-minvv);
            for (/*V3DLONG*/ i=0;i<totalunits;i++)
			{
				outimg[i] = (unsigned char)(double(tmpimg[i]-minvv)*w);
			}
		}
		else
		{
            for (/*V3DLONG*/ i=0;i<totalunits;i++)
			{
				outimg[i] = (unsigned char)(tmpimg[i]); //then no need to rescale. If the original value is small than 0 or bigger than 255, then let it be during the type-conversion
			}
		}
	}
    
	//copy to output data
    
	delete [] ((unsigned char *)img); //as I know img was originally allocated as (unsigned char *)
	img = outimg;
    
	return true;
}


