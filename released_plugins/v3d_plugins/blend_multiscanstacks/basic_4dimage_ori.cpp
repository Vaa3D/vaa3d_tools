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




/*
 * basic_4dimage.cpp
 * last update: 100819: Hanchuan Peng. use MYLIB only for Llinux and Mac, but not WIN32. FIXME: add VC support later.
 */

#include "v3d_message.h"

#include "stackutil.h"
#include "basic_4dimage.h"

typedef unsigned short int USHORTINT16;


void Image4DSimple::loadImage(char filename[])
{
	return this->loadImage(filename, false); //default don't use MYLib
}

void Image4DSimple::loadImage(char filename[], bool b_useMyLib)
{
	cleanExistData(); // note that this variable must be initialized as NULL. 

	strcpy(imgSrcFile, filename);

	V3DLONG * tmp_sz = 0; // note that this variable must be initialized as NULL. 
	int tmp_datatype = 0;
	int pixelnbits=1; //100817

	//060815, 060924, 070805
	char * curFileSurfix = getSurfix(imgSrcFile);
	printf("The current input file has the surfix [%s]\n", curFileSurfix);

	if (strcasecmp(curFileSurfix, "tif")==0 || strcasecmp(curFileSurfix, "tiff")==0 ||
		strcasecmp(curFileSurfix, "lsm")==0 ) //read tiff/lsm stacks
	{
            printf("Image4DSimple::loadImage loading filename=%s\n", filename);

#if defined _WIN32 		
		{
			v3d_msg("(Win32) Now try to use LIBTIFF (slightly revised by PHC) to read the TIFF/LSM...\n",0);
			if (strcasecmp(curFileSurfix, "tif")==0 || strcasecmp(curFileSurfix, "tiff")==0)
			{
				if (loadTif2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
				{
					v3d_msg("Error happens in TIF file reading (using libtiff). \n", false);
					b_error=1;
				}
			}	
			else //if ( strcasecmp(curFileSurfix, "lsm")==0 ) //read lsm stacks
			{
				if (loadLsm2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
				{
					v3d_msg("Error happens in LSM file reading (using libtiff, slightly revised by PHC). \n", false);
					b_error=1;
				}
			}
		}
		
#else
		if (b_useMyLib)
		{
			v3d_msg("Now try to use MYLIB to read the TIFF/LSM again...\n",0);
			if (loadTif2StackMylib(imgSrcFile, data1d, tmp_sz, tmp_datatype, pixelnbits))
			{
				v3d_msg("Error happens in TIF/LSM file reading (using MYLIB). Stop. \n", false);
				b_error=1;
				return;
			}
			else
				b_error=0; //when succeed then reset b_error
		}
		else
		{
			v3d_msg("Now try to use LIBTIFF (slightly revised by PHC) to read the TIFF/LSM...\n",0);
			if (strcasecmp(curFileSurfix, "tif")==0 || strcasecmp(curFileSurfix, "tiff")==0)
			{
				if (loadTif2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
				{
					v3d_msg("Error happens in TIF file reading (using libtiff). \n", false);
					b_error=1;
				}
			}	
			else //if ( strcasecmp(curFileSurfix, "lsm")==0 ) //read lsm stacks
			{
				if (loadLsm2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
				{
					v3d_msg("Error happens in LSM file reading (using libtiff, slightly revised by PHC). \n", false);
					b_error=1;
				}
			}
		}
                printf("Image4DSimple::loadImage finished\n");

#endif
		
	}
	else if ( strcasecmp(curFileSurfix, "mrc")==0 ) //read mrc stacks
	{
		if (loadMRC2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
		{
			v3d_msg("Error happens in MRC file reading. Stop. \n", false);
			b_error=1;
			return;
		}
	}
	else //then assume it is Hanchuan's RAW format
	{
		v3d_msg("The data is not with a TIF surfix, -- now this program assumes it is RAW format defined by Hanchuan Peng. \n", false);
		if (loadRaw2Stack(imgSrcFile, data1d, tmp_sz, tmp_datatype))
		{
			printf("The data doesn't look like a correct 4-byte-size RAW file. Try 2-byte-raw. \n");
			if (loadRaw2Stack_2byte(imgSrcFile, data1d, tmp_sz, tmp_datatype))
			{
				v3d_msg("Error happens in reading 4-byte-size and 2-byte-size RAW file. Stop. \n", false);
				b_error=1;
				return;
			}
		}
	}

	//080302: now convert any 16 bit or float data to the range of 0-255 (i.e. 8bit)
	switch (tmp_datatype)
	{
		case 1:
			datatype = V3D_UINT8;
			break;

		case 2: //080824
			//convert_data_to_8bit((void *&)data1d, tmp_sz, tmp_datatype);
			//datatype = UINT8; //UINT16;
			datatype = V3D_UINT16;
			break;

		case 4:
			//convert_data_to_8bit((void *&)data1d, tmp_sz, tmp_datatype);
			datatype = V3D_FLOAT32; //FLOAT32;
			break;

		default:
			v3d_msg("The data type is not UINT8, UINT16 or FLOAT32. Something wrong with the program, -- should NOT display this message at all. Check your program. \n");
			if (tmp_sz) {delete []tmp_sz; tmp_sz=0;}
			return;
	}

	sz0 = tmp_sz[0];
	sz1 = tmp_sz[1];
	sz2 = tmp_sz[2];
	sz3 = tmp_sz[3]; //no longer merge the 3rd and 4th dimensions

	/* clean all workspace variables */

	if (tmp_sz) {delete []tmp_sz; tmp_sz=0;}

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
		USHORTINT16 * tmpimg = (USHORTINT16 *)img;
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

	strcpy(imgSrcFile, "Untitled_blank.raw");

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

bool Image4DSimple::saveImage(const char filename[])
{
	if (!data1d || !filename)
	{
		v3d_msg("This image data is empty or the file name is invalid. Nothing done.\n");
		return false;
	}

	V3DLONG mysz[4];
	mysz[0] = sz0;
	mysz[1] = sz1;
	mysz[2] = sz2;
	mysz[3] = sz3;

	int dt;
	switch (datatype)
	{
		case V3D_UINT8:  dt=1; break;
		case V3D_UINT16:  dt=2; break;
		case V3D_FLOAT32:  dt=4; break;
		default:
			v3d_msg("The data type is unsupported. Nothing done.\n");
			return false;
			break;
	}

	//061009
	char * curFileSurfix = getSurfix((char *)filename);
	printf("The current output file has the surfix [%s]\n", curFileSurfix);
	if (strcasecmp(curFileSurfix, "tif")==0 || strcasecmp(curFileSurfix, "tiff")==0) //read tiff stacks
	{
		if (saveStack2Tif(filename, data1d, mysz, dt)!=0)
		{
			v3d_msg("Error happens in TIF file writing. Stop. \n");
			b_error=1;
			return false;
		}
	}
	else //then assume it is Hanchuan's RAW format
	{
		printf("The data is not with a TIF surfix, -- now this program assumes it is RAW format defined by Hanchuan Peng. \n");
		if (saveStack2Raw(filename, data1d, mysz, dt)!=0)   //0 is no error //note that as I updated the saveStack2Raw to RAW-4-byte, the actual mask file cannot be read by the old wano program, i.e. the wano must be updated on Windows machine as well. 060921
			//if (saveStack2Raw_2byte(filename, data1d, mysz, dt)!=0)   //for compatability save it to 2-byte raw //re-commented on 081124. always save to 4-byte raw
		{
			v3d_msg("Fail to save data to file [%s].\n", filename);
			b_error=1;
			return false;
		}
	}

	return true;
}


