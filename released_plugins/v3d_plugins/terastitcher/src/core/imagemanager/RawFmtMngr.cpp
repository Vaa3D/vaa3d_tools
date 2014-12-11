//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).  
// All rights reserved.
//------------------------------------------------------------------------------------------------

/*******************************************************************************************************************************************************************************************
*    LICENSE NOTICE
********************************************************************************************************************************************************************************************
*    By downloading/using/running/editing/changing any portion of codes in this package you agree to this license. If you do not agree to this license, do not download/use/run/edit/change
*    this code.
********************************************************************************************************************************************************************************************
*    1. This material is free for non-profit research, but needs a special license for any commercial purpose. Please contact Alessandro Bria at a.bria@unicas.it or Giulio Iannello at 
*       g.iannello@unicampus.it for further details.
*    2. You agree to appropriately cite this work in your related studies and publications.
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

/*
 * This file is a modified version of code extracted from the files stackutil.cpp and
 * stackutil-11.cpp of the V3D project. See the following licence notice for more details.
 *
 * modified by Giulio Iannello, Centro Integrato di Ricerca, Universita' Campus Bio-Medico di Roma
 * December 2012
 */


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
 *  stackutil.cpp
 *
 *
 *  Created (extracted from earlier codes) by Hanchuan Peng on 7/31/06.
 *  Modified on 060803 by Hanchuan Peng: change the size information to be 4 V3DLONG (4-byte) instead of 4-short-int (2byte).
 *  Updated 060806: correct a bug in 4-byte swapping. And notice the compatibility of 32-bit system and 64-bit system about the length of int/V3DLONG types
 *  Copyright 2006 __Hanchuan Peng__. All rights reserved.
 *
 *  060816: add tif support
 *  060828: add surfix extraction function here
 *
 *  060920: add 2-byte formatted raw format support back to increase the compatibility of existing data in the worm project
 *
 *  070214: add two simple interface to read any file formats supported (raw or tif) based on the surfix of filename
 *  070220: add b_VERBOSE_PRINT
 *  070306: add tif file exist verification
 * 070713: George tried to add lsm file reading functions but unsucecssful
 * 070806: Hanchuan added lsm 8-bit reading functions
 * 070807/08: try to handle the imcomplete data generated by the Zeiss lsm writer. The current solution is to change the a function in libtiff tif_lzw.c file
 *            and force it return the value 2, instead of 0, when the input strip size is bigger than real. In this way, the tif function TIFFReadEncodedStrip()
 *            will continue to do postprocessing of the already read data.
 *
 *            Special note: in this way, I will have to build a customary libtiff library and install at my own location ~/work/3rdsoft/lib instead of
 *                          the default /usr/local/lib. When I will have to reset the .bashrc/.profile file and use LD_LIBRARY_PATH (and probably also DYLD_LIBRARY_PATH)
 *                          for Mac to link them. This works for the case of V3D already. However, interestingly, I read an article arguing that using otool/ldd/nm, the dependency
 *                          of objects files can be seen, and that Mac uses the absolute path in library dependency, and that LD_LIBRARY_PATH does not work. The
 *                          interesting thing is that after I tried both, I found otool does show the absolute path, but LD_LIBARY_PATH/DYLD_LIBRARY_PATH works!
 *
 * 070819: add several functions to read LSM file thumbnails and middle slice.
 * 071125: update the copy on my G4 Mac so that it use CPP interface to Gene's codes instead of the C interface
 * 080103: add some further verbose printing
 * 20080213: add interface functions to read single slice of a tiff stack, or the respective thumbnail: Note that the lsm and tif interfaces are DIfferent!
 * 20080301: add handling of 12/16 bit LSM
 * 20080302: change the include "basic_memory.h" to .cpp, due to the template instantiation in V3D
 * 20080401: add the ifndef tag as this will often be included in other files
 * 20080828: add file size limitation, so avoid loading too big file that may cause the tif reader to crash
 * 20080930: return the original tiff pixel value but not the automatically scaled down version
 * 20081204: add overload functions to read only a channel of a stack
 * 20090413: add simple MRC file reading and writing
 * 20090802: add raw5d read/write
 * 20100520: try to revise ZZBIG to allow big file, also add fstat check for windows large file size , instead of using fseek and ftell
 * 20100816: add mylib interface, by PHC.
 * 20110708: fix a memory leak (non-crashing) of redudnant memory allocation of a sz variable
 * 20110804: fix a new Mac fread issue to read more than 2G data in one call (I don't believe I saw this on the old Leopard 64 bit Mac Pro desktop, as we (both Zongcai and I should have) 
 *           benched tested the >3G v3draw file read. This problem is not found for Linux 64bit. Is that a new Mac std libc bug?? 
 *           Anyway, I have now used a 2G buffer to read >2G data. I have not changed the saveStack2Raw functions. It seems they work in the Matlab mex functions. Thus I assumed
 *           they don't need to change. Need tests anyway.
 * 20120410: fix a bug when strcasecmp_l() taking a NULL parameter so that it crashes
 */


// 64-bit and 32-bit checking
// Windows
#if _WIN32 || _WIN64
#if _WIN64
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

// GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

//#include "../elementmexheader.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "RawFmtMngr.h"

#include "IM_config.h"
using namespace iim;

//#include "basic_memory.cpp" //change basic_memory.h to basic_memory.cpp, 080302

/*
extern "C" {
*/
//#include "mg_image_lib.h"
/*
};
*/

#define b_VERBOSE_PRINT 0
#define ZZBIG 10000 //previous I define it as 1500, so that to limit the size of an image is at most 1.5G //change 2010-05-21 // hang 2011-08-25 6000->10000

#ifdef _MSC_VER       //2010-05-21, by PHC
#include <sys/stat.h>
#include <io.h>
#endif

#define DEFINE_NBYTE2G \
  V3DLONG nBytes2G = (V3DLONG(1024)*V3DLONG(1024)*V3DLONG(1024)-1)*V3DLONG(2);

static char err_message[5000]; // a pointer to this message is returned when there is an exception


/****************************************************
 * AUXILIARY FUNCTIONS
 ****************************************************/

/* function to check endianness of the machine 
 * returns: 
 *  L for little endian machines
 *  B for big endian machines
 *  M for machines that swap 16-bits words
 *  N for unknown endianness
 */
static 
char checkMachineEndian()
{
    char e='N'; //for unknown endianness

    V3DLONG int a=0x44332211;
    unsigned char * p = (unsigned char *)&a;
    if ((*p==0x11) && (*(p+1)==0x22) && (*(p+2)==0x33) && (*(p+3)==0x44))
        e = 'L';
    else if ((*p==0x44) && (*(p+1)==0x33) && (*(p+2)==0x22) && (*(p+3)==0x11))
        e = 'B';
    else if ((*p==0x22) && (*(p+1)==0x11) && (*(p+2)==0x44) && (*(p+3)==0x33))
        e = 'M';
    else
        e = 'N';

    //printf("[%c] \n", e);
    return e;
}


/* functions to swap 2-bytes and 4-bytes words */

static
void swap2bytes(void *targetp)
{
    unsigned char * tp = (unsigned char *)targetp;
    unsigned char a = *tp;
    *tp = *(tp+1);
    *(tp+1) = a;
}

static
void swap4bytes(void *targetp)
{
    unsigned char * tp = (unsigned char *)targetp;
    unsigned char a = *tp;
    *tp = *(tp+3);
    *(tp+3) = a;
    a = *(tp+1);
    *(tp+1) = *(tp+2);
    *(tp+2) = a;
}

/* This function opens 2-4D image stack from raw data and returns metadata contained in the header
 * assuming that sz elements are stored as 4-bytes integers.
 * The input parameters sz, and datatype should be empty, especially the pointer "sz". 
 * The file is closed.
 */
static
char *loadMetadata ( char * filename, V3DLONG * &sz, int &datatype, int &b_swap, int &header_len ) {

	FILE * fid = fopen(filename, "rb");
	if (!fid)
		return ((char *)"Fail to open file for reading");

	fseek (fid, 0, SEEK_END);
	V3DLONG fileSize = ftell(fid);
	rewind(fid);

	/* Read header */

	char formatkey[] = "raw_image_stack_by_hpeng";
	V3DLONG lenkey = strlen(formatkey);

	// for 4 byte integers: datatype has 2 bytes, and sz has 4*4 bytes and endian flag has 1 byte
	// WARNINIG: this should still be valid even for 2 byte integres assuming that there are at least 8 data bytes
	if (fileSize<lenkey+2+4*4+1)  
		return ((char *)"The size of your input file is too small and is not correct, -- it is too small to contain the legal header");

	char * keyread = new char [lenkey+1];
	if (!keyread)
		return ((char *)"Fail to allocate memory");

	V3DLONG nread = fread(keyread, 1, lenkey, fid);
	if (nread!=lenkey)
		return ((char *)"File unrecognized or corrupted file");

	keyread[lenkey] = '\0';

	V3DLONG i;
	if (strcmp(formatkey, keyread)) { /* is non-zero then the two strings are different */
		if (keyread) {delete []keyread; keyread=0;}
		return ((char *)"Unrecognized file format");
	}

	char endianCodeData;
	size_t dummy = fread(&endianCodeData, 1, 1, fid);
	if (endianCodeData!='B' && endianCodeData!='L')
	{
		if (keyread) {delete []keyread; keyread=0;}
		return ((char *)"This program only supports big- or little- endian but not other format. Check your data endian");
	}

	char endianCodeMachine;
	endianCodeMachine = checkMachineEndian();
	if (endianCodeMachine!='B' && endianCodeMachine!='L')
	{
		if (keyread) {delete []keyread; keyread=0;}
		return ((char *)"This program only supports big- or little- endian but not other format. Check your data endian");
	}

	b_swap = (endianCodeMachine==endianCodeData)?0:1;

	short int dcode = 0;
	dummy = fread(&dcode, 2, 1, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
	if (b_swap)
		swap2bytes((void *)&dcode);

	switch (dcode)
	{
		case 1:
			datatype = 1; /* temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. */
			break;

		case 2:
			datatype = 2;
			break;

		case 4:
			datatype = 4;
			break;

		default:
			if (keyread) {delete []keyread; keyread=0;}
			return ((char *)"Unrecognized data type code. The file type is incorrect or this code is not supported in this version");
	}

	V3DLONG unitSize = datatype; // temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. 

	BIT32_UNIT mysz[4]; // actual buffer 
	mysz[0]=mysz[1]=mysz[2]=mysz[3]=0;

	int tmpn=(int)fread(mysz, 4, 4, fid); // because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. 
	if (tmpn!=4) {
		if (keyread) {delete []keyread; keyread=0;}
		return ((char *)"This program only reads [4] units");
	}
	if (b_swap) {
		for (i=0;i<4;i++) 
			swap4bytes((void *)(mysz+i));
	}

	if (sz) {delete []sz; sz=0;}
	sz = new V3DLONG [4]; // reallocate the memory if the input parameter is non-null. Note that this requests the input is also an NULL point, the same to img. 
	if (!sz)
	{
		if (keyread) {delete []keyread; keyread=0;}
		return ((char *)"Fail to allocate memory");
	}

	for (i=0;i<4;i++)
	{
		sz[i] = (V3DLONG)mysz[i];
	}

	// clean and return 
	if (keyread) {delete [] keyread; keyread = 0;}

	header_len = (int)ftell(fid);

	fclose(fid); 

	return ((char *) 0);
}


/****************************************************
 * EXTERNAL FUNCTIONS
 ****************************************************/


char *loadRaw2Metadata ( char * filename, V3DLONG * &sz, int &datatype, int &b_swap, void * &fhandle, int &header_len ) {
    /* This function opens 2-4D image stack from raw data generated by the program "saveStack2Raw.m". */
	/* The input parameters sz, and datatype should be empty, especially the pointer "sz". */

	FILE * fid = fopen(filename, "rb");
	if (!fid)
		return ((char *)"Fail to open file for reading");

	fseek (fid, 0, SEEK_END);
	V3DLONG fileSize = ftell(fid);
	rewind(fid);

	/* Read header */

	char formatkey[] = "raw_image_stack_by_hpeng";
	V3DLONG lenkey = strlen(formatkey);

	// for 4 byte integers: datatype has 2 bytes, and sz has 4*4 bytes and endian flag has 1 byte
	// WARNINIG: this should still be valid even for 2 byte integres assuming that there are at least 8 data bytes
	if (fileSize<lenkey+2+4*4+1) {
		fclose(fid);
		return ((char *)"The size of your input file is too small and is not correct, -- it is too small to contain the legal header");
	}

	char * keyread = new char [lenkey+1];
	if (!keyread) {
		fclose(fid);
		return ((char *)"Fail to allocate memory");
	}

	V3DLONG nread = fread(keyread, 1, lenkey, fid);
	if (nread!=lenkey) {
		fclose(fid);
		return ((char *)"File unrecognized or corrupted file");
	}

	keyread[lenkey] = '\0';

	V3DLONG i;
	if (strcmp(formatkey, keyread)) { /* is non-zero then the two strings are different */
		fclose(fid);
		if (keyread) {delete []keyread; keyread=0;}
		return ((char *)"Unrecognized file format");
	}

	char endianCodeData;
	int dummy = (int)fread(&endianCodeData, 1, 1, fid);
	if (endianCodeData!='B' && endianCodeData!='L')
	{
		fclose(fid);
		if (keyread) {delete []keyread; keyread=0;}
		return ((char *)"This program only supports big- or little- endian but not other format. Check your data endian");
	}

	char endianCodeMachine;
	endianCodeMachine = checkMachineEndian();
	if (endianCodeMachine!='B' && endianCodeMachine!='L')
	{
		fclose(fid);
		if (keyread) {delete []keyread; keyread=0;}
		return ((char *)"This program only supports big- or little- endian but not other format. Check your data endian");
	}

	b_swap = (endianCodeMachine==endianCodeData)?0:1;

	short int dcode = 0;
	dummy = (int)fread(&dcode, 2, 1, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
	if (b_swap)
		swap2bytes((void *)&dcode);

	switch (dcode)
	{
		case 1:
			datatype = 1; /* temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. */
			break;

		case 2:
			datatype = 2;
			break;

		case 4:
			datatype = 4;
			break;

		default:
			fclose(fid);
			if (keyread) {delete []keyread; keyread=0;}
			return ((char *)"Unrecognized data type code. The file type is incorrect or this code is not supported in this version");
	}

	V3DLONG unitSize = datatype; // temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. 

	bool is_4_byte_file = true; // assumes 4 byte file
	V3DLONG totalUnit;
	BIT32_UNIT mysz[4]; // actual buffer 
	mysz[0]=mysz[1]=mysz[2]=mysz[3]=0;

	// temporary buffer: allocate space for four 4 byte integers
	BIT16_UNIT *sz_2bytes = new BIT16_UNIT[8];

	// try 2 byte integers first
	int tmpn=(int)fread(sz_2bytes, 2, 4, fid); // because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read.
	if (tmpn!=4) {
		fclose(fid);
		if (keyread) {delete []keyread; keyread=0;}
		return ((char *)"This program only reads [4] units");
	}
	memcpy(mysz,sz_2bytes,2*4); // save bytes in case it is a 4 byte file
	if (b_swap) {
		for (i=0;i<4;i++) 
			swap2bytes((void *)(sz_2bytes+i));
	}
	totalUnit = 1;
	for (i=0;i<4;i++) {
		totalUnit *= sz_2bytes[i];
	}
	if ((totalUnit*unitSize+4*2+2+1+lenkey) == fileSize) { // 2 byte file
		is_4_byte_file = false;
		// assign sizes to actual buffer
		for (i=0;i<4;i++) {
			mysz[i] = (BIT32_UNIT)sz_2bytes[i];
		}
	}
	delete sz_2bytes;

	if ( is_4_byte_file ) {
		// reads four more 2 byte elements 
		tmpn=(int)fread(mysz+2, 2, 4, fid); // because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. 
		if (tmpn!=4) {
			fclose(fid);
			if (keyread) {delete []keyread; keyread=0;}
			return ((char *)"This program only reads [4] units");
		}
		if (b_swap) {
			for (i=0;i<4;i++) 
				swap4bytes((void *)(mysz+i));
		}
		totalUnit = 1;
		for (i=0;i<4;i++) {
			totalUnit *= mysz[i];
		}
		if ((totalUnit*unitSize+4*4+2+1+lenkey) != fileSize) {
			fclose(fid);
			if (keyread) {delete []keyread; keyread=0;}
			return ((char *)"The input file has a size different from what specified in the header");
		}
	}

	if (sz) {delete []sz; sz=0;}
	sz = new V3DLONG [4]; // reallocate the memory if the input parameter is non-null. Note that this requests the input is also an NULL point, the same to img. 
	if (!sz)
	{
		fclose(fid);
		if (keyread) {delete []keyread; keyread=0;}
		return ((char *)"Fail to allocate memory");
	}

	for (i=0;i<4;i++)
	{
		sz[i] = (V3DLONG)mysz[i];
	}

	// clean and return 
	if (keyread) {delete [] keyread; keyread = 0;}

	fhandle = fid;
    header_len = (int)ftell(fid);

	return ((char *) 0);
}

void closeRawFile ( void *fhandle ) {
	fclose((FILE *) fhandle);
}

char *loadRaw2SubStack ( void *fhandle, unsigned char *img, V3DLONG *sz, 
						 V3DLONG startx, V3DLONG starty, V3DLONG startz, 
						 V3DLONG endx,V3DLONG endy, V3DLONG endz, 
						 int datatype, int b_swap, int header_len ) //this is the function of 4-byte raw format.
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

    /* This function reads 2-4D image stack from raw data generated by the program "saveStack2Raw.m". */
	/* The input parameter img should be empty. */
	int i;
	FILE * fid = (FILE *) fhandle;
	
	fseek (fid, 0, SEEK_END);
	//V3DLONG fileSize = ftell(fid); // unused
	rewind(fid);
		
	V3DLONG unitSize = datatype; /* temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. */
	
	V3DLONG totalUnit = 1;
	for (i=0;i<4;i++)
	{
		totalUnit *= sz[i];
	}	
	
	V3DLONG tmpw = endx - startx;
	V3DLONG tmph = endy - starty;
	V3DLONG tmpz = endz - startz;

	V3DLONG head = header_len; //4*4+2+1+lenkey; // header_len ?
	V3DLONG pgsz1=sz[2]*sz[1]*sz[0], pgsz2=sz[1]*sz[0], pgsz3=sz[0];
	V3DLONG cn = tmpw*tmph*tmpz;
	V3DLONG kn = tmpw*tmph;
	V3DLONG total = tmpw*tmph*tmpz*sz[3];

	//V3DLONG count=0; // unused
	V3DLONG c,j,k;
	for (c = 0; c < sz[3]; c++)
	{
		for (k = startz; k < endz; k++) 
		{
			for (j = starty; j< endy; j++)
			{
				rewind(fid);
				fseek(fid, (long)(head+(c*pgsz1 + k*pgsz2 + j*pgsz3 + startx)*unitSize), SEEK_SET);
				int dummy = (int)ftell(fid);	
				dummy = (int)fread(img+(c*cn+(k-startz)*kn + (j-starty)*tmpw)*unitSize,unitSize,tmpw,fid);
			}
		}
	}

	/* swap the data bytes if necessary */
	
	if (b_swap==1)
	{
		if (unitSize==2)
		{
			for (i=0;i<total; i++)
			{
				swap2bytes((void *)(img+i*unitSize));
			}
		}
		else if (unitSize==4)
		{
			for (i=0;i<total; i++)
			{
				swap4bytes((void *)(img+i*unitSize));
			}
		}
	}

	
	/* clean and return */
	
	return ((char *) 0);
}



char *loadRaw2WholeStack ( char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype) //this is the function of 4-byte raw format.
{
    /* This function reads 2-4D image stack from raw data generated by the program "saveStack2Raw.m". */
	/* The input parameters img, sz, and datatype should be empty, especially the pointers "img" and "sz". */
	if (b_VERBOSE_PRINT)
		printf("size of [V3DLONG]=[%ld], [V3DLONG]=[%ld] [int]=[%ld], [short int]=[%ld], [double]=[%ld], [float]=[%ld]\n",
		//mexPrintf("size of [V3DLONG]=[%d], [V3DLONG]=[%d] [int]=[%d], [short int]=[%d], [double]=[%d], [float]=[%d]\n",
          sizeof(V3DLONG), sizeof(V3DLONG), sizeof(int), sizeof(short int), sizeof(double), sizeof(float));
    //printf("\n\n\n");

	/*
#ifdef _MSC_VER
	struct stat stbuf;
	int myfd;
	myfd = open(filename, O_RDONLY);
	if (myfd<0) 
	{
		printf("Fail to use open() for this file. \n");
		berror=1;
		return berror;
	}
	FILE * fid = fdopen(myfd, "rb");
	{
		printf("Fail to open file for reading.\n");
		berror = 1;
		return berror;
	}
	if (fstat(myfd, &stbuf) == -1)
	{
		printf("Fail to get the file status. \n");
		berror=1;
		return berror;
	}
	V3DLONG fileSize = stbuf.st_size;
#else
*/
	FILE * fid = fopen(filename, "rb");
	if (!fid)
	{
		return ((char *)"Fail to open file for reading");
	}

	fseek (fid, 0, SEEK_END);
	V3DLONG fileSize = ftell(fid);
	rewind(fid);
/*
#endif
*/
	/* Read header */

	char formatkey[] = "raw_image_stack_by_hpeng";
	V3DLONG lenkey = strlen(formatkey);

#ifndef _MSC_VER //added by PHC, 2010-05-21
	if (fileSize<lenkey+2+4*4+1) // datatype has 2 bytes, and sz has 4*4 bytes and endian flag has 1 byte. 
	{
		sprintf(err_message,"The size of your input file is too small and is not correct, -- it is too small to contain the legal header.\n"
				"The fseek-ftell produces a file size = %ld.", fileSize);
		return err_message;
	}
#endif

	char * keyread = new char [lenkey+1];
	if (!keyread)
		return ((char *)"Fail to allocate memory");

	V3DLONG nread = fread(keyread, 1, lenkey, fid);
	if (nread!=lenkey) {
		if (keyread) {delete []keyread; keyread=0;}
		return ((char *)"File unrecognized or corrupted file");
	}

	keyread[lenkey] = '\0';

	V3DLONG i;
	if (strcmp(formatkey, keyread)) /* is non-zero then the two strings are different */
	{
		if (keyread) {delete []keyread; keyread=0;}
		return ((char *)"Unrecognized file format");
	}

	char endianCodeData;
	int dummy = (int)fread(&endianCodeData, 1, 1, fid);
	printf("The data endian code is [%c]\n", endianCodeData);
	if (endianCodeData!='B' && endianCodeData!='L')
	{
		if (keyread) {delete []keyread; keyread=0;}
		return ((char *)"This program only supports big- or little- endian but not other format. Check your data endian");
	}

	char endianCodeMachine;
	endianCodeMachine = checkMachineEndian();
	printf("The machine endian code is [%c]\n", endianCodeMachine);
	if (endianCodeMachine!='B' && endianCodeMachine!='L')
	{
		if (keyread) {delete []keyread; keyread=0;}
		return ((char *)"This program only supports big- or little- endian but not other format. Check your data endian");
	}

	int b_swap = (endianCodeMachine==endianCodeData)?0:1;
	if (b_VERBOSE_PRINT)
    {
        printf((char *)"machine endian=[%c] data endian=[%c] b_swap=%d\n", endianCodeMachine, endianCodeData, b_swap);
    }


	short int dcode = 0;
	dummy = (int)fread(&dcode, 2, 1, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
	if (b_swap)
		swap2bytes((void *)&dcode);

	switch (dcode)
	{
		case 1:
			datatype = 1; /* temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. */
			break;

		case 2:
			datatype = 2;
			break;

		case 4:
			datatype = 4;
			break;

		default:
			if (keyread) {delete []keyread; keyread=0;}
			return ((char *)"Unrecognized data type code. The file type is incorrect or this code is not supported in this version");
	}

	V3DLONG unitSize = datatype; // temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. 

	BIT32_UNIT mysz[4];
	mysz[0]=mysz[1]=mysz[2]=mysz[3]=0;
	int tmpn=(int)fread(mysz, 4, 4, fid); // because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. 
	if (tmpn!=4) {
		if (keyread) {delete []keyread; keyread=0;}
		return ((char *)"This program only reads [4] units");
	}

	if (b_swap)
	{
		for (i=0;i<4;i++)
		{
			//swap2bytes((void *)(mysz+i));
			if (b_VERBOSE_PRINT)
				printf("mysz raw read unit[%ld]: [%d] ", i, mysz[i]);
			swap4bytes((void *)(mysz+i));
			if (b_VERBOSE_PRINT)
				printf("swap unit: [%d][%0x] \n", mysz[i], mysz[i]);
		}
	}

	if (sz) {delete []sz; sz=0;}
	sz = new V3DLONG [4]; // reallocate the memory if the input parameter is non-null. Note that this requests the input is also an NULL point, the same to img. 
	if (!sz)
	{
		if (keyread) {delete []keyread; keyread=0;}
		return ((char *)"Fail to allocate memory");
	}

	V3DLONG totalUnit = 1;
	for (i=0;i<4;i++)
	{
		sz[i] = (V3DLONG)mysz[i];
		totalUnit *= sz[i];
	}

#ifndef _MSC_VER //added by PHC, 2010-05-21
	if ((totalUnit*unitSize+4*4+2+1+lenkey) != fileSize)
	{
		sprintf(err_message,"The input file has a size [%ld bytes], different from what specified in the header [%ld bytes]. Exit.\n"
				"The read sizes are: %ld %ld %ld %ld\n", fileSize, totalUnit*unitSize+4*4+2+1+lenkey, sz[0], sz[1], sz[2], sz[3]);
		if (keyread) {delete []keyread; keyread=0;}
		if (sz) {delete []sz; sz=0;}
		return err_message;
	}
#endif

	if (img) {delete []img; img=0;}
	V3DLONG totalBytes = V3DLONG(unitSize)*V3DLONG(totalUnit);
	try 
	{
		img = new unsigned char [totalBytes];
	}
	catch (...)
	{
		if (keyread) {delete []keyread; keyread=0;}
		if (sz) {delete []sz; sz=0;}
		return ((char *)"Fail to allocate memory in loadRaw2Stack()");
	}

	V3DLONG remainingBytes = totalBytes;
    
    DEFINE_NBYTE2G
	
	V3DLONG cntBuf = 0;
	while (remainingBytes>0)
	{
		V3DLONG curReadBytes = (remainingBytes<nBytes2G) ? remainingBytes : nBytes2G;
		V3DLONG curReadUnits = curReadBytes/unitSize;
		nread = fread(img+cntBuf*nBytes2G, unitSize, curReadUnits, fid);
		if (nread!=curReadUnits)
		{
			sprintf(err_message,"Something wrong in file reading. The program reads [%ld data points] but the file says there should be [%ld data points]", nread, totalUnit);
			if (keyread) {delete []keyread; keyread=0;}
			if (sz) {delete []sz; sz=0;}
			if (img) {delete []img; img=0;}
			return err_message;
		}
		
		remainingBytes -= nBytes2G;
		cntBuf++;
	}

	// swap the data bytes if necessary 

	if (b_swap==1)
	{
		if (unitSize==2)
		{
			for (i=0;i<totalUnit; i++)
			{
				swap2bytes((void *)(img+i*unitSize));
			}
		}
		else if (unitSize==4)
		{
			for (i=0;i<totalUnit; i++)
			{
				swap4bytes((void *)(img+i*unitSize));
			}
		}
	}

	// clean and return 

	if (keyread) {delete [] keyread; keyread = 0;}
	fclose(fid); //bug fix on 060412

	//a debug check of the min max value
	//double minvv=10000, maxvv=-1;
	//for (V3DLONG myii=0; myii<sz[0]*sz[1]*sz[2];myii++)
	//{
	//	if (minvv>img[myii]) minvv=img[myii];
	//	else if (maxvv<img[myii]) maxvv=img[myii];
	//}
	//printf("*** for loadRaw2Stack() all readin bytes: minvv=%5.3f maxvv=%5.3f\n", minvv, maxvv);	
	
	return ((char *) 0);
}


char *saveWholeStack2Raw(const char *filename, unsigned char *img, V3DLONG *sz, int datatype) {

	int i;

	FILE * fid = fopen(filename, "wb");
	if (!fid)
	{
		return ((char *)"Fail to open file for writing.\n");
	}

	/* Write header */
	char formatkey[] = "raw_image_stack_by_hpeng";
	int lenkey = (int)strlen(formatkey);

	V3DLONG nwrite = fwrite(formatkey, 1, lenkey, fid);
	if (nwrite!=lenkey)
	{
		return ((char *)"File write error.\n");
	}

	char endianCodeMachine = checkMachineEndian();
	if (endianCodeMachine!='B' && endianCodeMachine!='L')
	{
		return ((char *)"This program only supports big- or little- endian but not other format. Cannot save data on this machine.\n");
	}

	nwrite = fwrite(&endianCodeMachine, 1, 1, fid);
	if (nwrite!=1)
	{
		return ((char *)"Error happened in file writing.\n");
	}

	//int b_swap = (endianCodeMachine==endianCodeData)?0:1;
	//int b_swap = 0; //for this machine itself, should not swap data.

	short int dcode = (short int)datatype;
	if (dcode!=1 && dcode!=2 && dcode!=4)
	{
		return ((char *)"Unrecognized data type code.\n");
	}

	//if (b_swap) swap2bytes((void *)&dcode);
	nwrite = fwrite(&dcode, 2, 1, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
	if (nwrite!=1)
	{
		return ((char *)"Writing file error.\n");
	}

	V3DLONG unitSize = datatype; /* temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. */

	BIT32_UNIT mysz[4];
	for (i=0;i<4;i++) 
		mysz[i] = (BIT32_UNIT) sz[i];
	nwrite = fwrite(mysz, sizeof(BIT32_UNIT), 4, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
	if (nwrite!=4)
	{
		return ((char *)"Writing file error.\n");
	}

	V3DLONG totalUnit = 1;
	for (i=0;i<4;i++)
	{
		totalUnit *= sz[i];
	}

	nwrite = fwrite(img, unitSize, totalUnit, fid);
	if (nwrite!=totalUnit)
	{
		return ((char *)"Something wrong in file writing. The program wrote a different number of pixels than expected.\n");
	}

	/* clean and return */

	fclose(fid);

	return ((char *) 0);
}


char *initRawFile(char *filename, const V3DLONG *sz, int datatype) {

	int i;
	char *completeFilename = new char[strlen(filename)+4+1];
	strcpy(completeFilename,filename);
	strcat(completeFilename,".");
	strcat(completeFilename,VAA3D_SUFFIX);

	FILE * fid = fopen(completeFilename, "wb");
	if (!fid)
	{
		if ( completeFilename ) { delete completeFilename; completeFilename = 0; }
		return ((char *)"Fail to open file for writing.\n");
	}

	/* Write header */
	char formatkey[] = "raw_image_stack_by_hpeng";
	int lenkey = (int)strlen(formatkey);

	V3DLONG nwrite = fwrite(formatkey, 1, lenkey, fid);
	if (nwrite!=lenkey)
	{
		if ( completeFilename ) { delete completeFilename; completeFilename = 0; }
		return ((char *)"File write error.\n");
	}

	char endianCodeMachine = checkMachineEndian();
	if (endianCodeMachine!='B' && endianCodeMachine!='L')
	{
		if ( completeFilename ) { delete completeFilename; completeFilename = 0; }
		return ((char *)"This program only supports big- or little- endian but not other format. Cannot save data on this machine.\n");
	}

	nwrite = fwrite(&endianCodeMachine, 1, 1, fid);
	if (nwrite!=1)
	{
		if ( completeFilename ) { delete completeFilename; completeFilename = 0; }
		return ((char *)"Error happened in file writing.\n");
	}

	//int b_swap = (endianCodeMachine==endianCodeData)?0:1;
	//int b_swap = 0; //for this machine itself, should not swap data.

	short int dcode = (short int)datatype;
	if (dcode!=1 && dcode!=2 && dcode!=4)
	{
		if ( completeFilename ) { delete completeFilename; completeFilename = 0; }
		return ((char *)"Unrecognized data type code.\n");
	}

	//if (b_swap) swap2bytes((void *)&dcode);
	nwrite = fwrite(&dcode, 2, 1, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
	if (nwrite!=1)
	{
		if ( completeFilename ) { delete completeFilename; completeFilename = 0; }
		return ((char *)"Writing file error.\n");
	}

	BIT32_UNIT mysz[4];
	for (i=0;i<4;i++) 
		mysz[i] = (BIT32_UNIT) sz[i];
	nwrite = fwrite(mysz, sizeof(BIT32_UNIT), 4, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
	if (nwrite!=4)
	{
		return ((char *)"Writing file error.\n");
	}

	// WARNING: 
	// The following code that fully allocate the file is made useless 
	// by the use of r+ mode in writeSlice2RawFile
	//
	//V3DLONG totalUnit = 1;
	//for (i=0;i<4;i++)
	//{
	//	totalUnit *= sz[i];
	//}
	//
	//int header_length = ftell(fid); // ununsed
	//
	//fseek(fid,(long)(totalUnit-1),SEEK_CUR); // last byte has to be actually written to file
	//
	//unsigned char buf[1];
	//fwrite(buf,sizeof(unsigned char),1,fid);

	closeRawFile(fid);

	if ( completeFilename ) { delete completeFilename; completeFilename = 0; }

	return 0;
}


char *writeSlice2RawFile ( char *filename, int slice, unsigned char *img, int img_height, int img_width ) {

	char *err_rawfmt;
	FILE *fid;
	V3DLONG *sz = 0;
	int datatype;
	int b_swap;
	int header_len;
	
	if ( (err_rawfmt = loadMetadata(filename,sz,datatype,b_swap,header_len)) != 0 ) {
		if ( sz ) delete[] sz;
		return err_rawfmt;
	}
	if ( (sz[1] != img_height) || (sz[0] != img_width) ) 
	{
		delete []sz;
		return ((char *)"Wrong slice dimensions.\n");
	}

	fid = fopen(filename, "r+b");
	if (!fid)
	{
		delete []sz;
		return ((char *)"Fail to open file for writing slice.\n");
	}


	V3DLONG slice_size = sz[0]*sz[1]*datatype;
	V3DLONG block_size = slice_size*sz[2]; // it has been already multiplied by datatype
	for ( int c=0; c<sz[3]; c++ ) {
		//int a = 0;
		//for ( int j=0; j<slice_size; j++ )
		//	if ( img[c][j] > 31 ) {
		//		printf("%d %d %d %d\n",c,slice,j,img[c][j]);
		//		a = 1;
		//		break;
		//	}
		fseek(fid,(long)(header_len + c*block_size + slice*slice_size),SEEK_SET);
		fwrite((img + (c*slice_size)),sizeof(unsigned char),slice_size,fid);
		//if ( a )
		//	closeRawFile(fid);
	}

	closeRawFile(fid);

	delete []sz;

	return 0;
}


char *copyRawFileBlock2Buffer ( char *filename, int sV0, int sV1, int sH0, int sH1, int sD0, int sD1,
							    unsigned char *buf, int pxl_size, sint64 offs, sint64 stridex, sint64 stridexy, sint64 stridexyz ) {

	//if ( pxl_size != sizeof(unsigned char) ) 
	//{
	//	return ((char *)"Wrong pixel size.\n");
	//}

	char *err_rawfmt;
	FILE *fid;
	void *fhandle;
	V3DLONG *sz = 0;
	int datatype;
	int b_swap;
	int header_len;
	
	if ( (err_rawfmt = loadRaw2Metadata(filename,sz,datatype,b_swap,fhandle,header_len)) != 0 ) {
		if ( sz ) delete[] sz;
		return err_rawfmt;
	}
	if ( datatype != pxl_size ) 
	{
		delete []sz;
		return ((char *)"requested and native data types are different.\n");
	}
	fid = (FILE *) fhandle;

	fseek (fid, 0, SEEK_END);
	//V3DLONG fileSize = ftell(fid); // unused
	rewind(fid);
		
	V3DLONG unitSize = datatype; /* temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. */
	
	V3DLONG startx = sH0;
	V3DLONG starty = sV0;
	V3DLONG startz = sD0;
	V3DLONG endx   = sH1;
	V3DLONG endy   = sV1;
	V3DLONG endz   = sD1; 

	V3DLONG tmpw = endx - startx; // width of sub block
	V3DLONG tmph = endy - starty; // height of sub block
	V3DLONG tmpz = endz - startz; // depth of sub block

	V3DLONG head = header_len; //4*4+2+1+lenkey; // header_len ?
	V3DLONG pgsz1=sz[2]*sz[1]*sz[0]; // #values per channel
	V3DLONG pgsz2=sz[1]*sz[0];       // #values per slice
	V3DLONG pgsz3=sz[0];
	//V3DLONG cn = tmpw*tmph*tmpz;  // unused
	//V3DLONG kn = tmpw*tmph; // unused     
	//V3DLONG total = tmpw*tmph*tmpz*sz[3]; // unused

	//V3DLONG count=0; // unused
	V3DLONG c,i,j,k;

	unsigned char *buftmp_c;
	unsigned char *buftmp_k;
	unsigned char *buftmp_j;

	/* swap the data bytes if necessary */
	
	if (b_swap==1)
	{
		if ( sz ) delete[] sz;
		return ((char *)"Byte swap not supported.\n");

		for (c=0, buftmp_c=(buf + (offs*unitSize)); c<sz[3]; c++, buftmp_c+=(stridexyz*unitSize))
		{
			for (k = startz, buftmp_k=buftmp_c; k < endz; k++, buftmp_k+=(stridexy*unitSize)) 
			{
				for (j = starty, buftmp_j=buftmp_k; j< endy; j++, buftmp_j+=(stridex*unitSize))
				{
					rewind(fid);
					fseek(fid, (long)(head+(c*pgsz1 + k*pgsz2 + j*pgsz3 + startx)*unitSize), SEEK_SET);
					int dummy = (int)ftell(fid);	
					dummy = (int)fread(buftmp_j,unitSize,tmpw,fid);

					// swap the bytes: this code has not be checked yey
					if (unitSize==2)
					{
						for (i=0;i<tmpw; i++)
						{
							swap2bytes((void *)(buftmp_j+i*unitSize));
						}
					}
					else if (unitSize==4)
					{
						for (i=0;i<tmpw; i++)
						{
							swap4bytes((void *)(buftmp_j+i*unitSize));
						}
					}
				}
			}
		}
	}
	else{ // no swap is required
		for (c=0, buftmp_c=(buf + (offs*unitSize)); c<sz[3]; c++, buftmp_c+=(stridexyz*unitSize))
		{
			for (k = startz, buftmp_k=buftmp_c; k < endz; k++, buftmp_k+=(stridexy*unitSize)) 
			{
				for (j = starty, buftmp_j=buftmp_k; j< endy; j++, buftmp_j+=(stridex*unitSize))
				{
					rewind(fid);
					fseek(fid, (long)(head+(c*pgsz1 + k*pgsz2 + j*pgsz3 + startx)*unitSize), SEEK_SET);
					int dummy = (int)ftell(fid);	
					dummy = (int)fread(buftmp_j,unitSize,tmpw,fid);
				}
			}
		}
	}

	closeRawFile(fid);

	delete[] sz;

	return 0;
}


char *streamer_open ( Streamer_Descr_t *streamer ) {

	char *err_rawfmt;
	void *fhandle;
	V3DLONG *sz = 0;
	int datatype;
	int b_swap;
	int header_len;
	
	for ( int i=0; i<streamer->n_blocks; i++ ) {
		if ( (err_rawfmt = loadRaw2Metadata(streamer->bDescr[i].filename,
											sz,datatype,b_swap,fhandle,header_len)) != 0 ) {
			if ( sz ) delete[] sz;
			return err_rawfmt;
		}
		streamer->bDescr[i].fhandle = fhandle;
	}

	if ( sz ) delete[] sz;

	return 0;
}

char *streamer_dostep ( Streamer_Descr_t *streamer, unsigned char *buffer2 ) {

	streamer->cur_step++;

	if ( streamer->cur_step > streamer->steps ) {
		return ((char *)"Too many steps in a streamed operation.\n");
	}

	for ( int i=0; i<streamer->n_blocks; i++ ) { 

		if ( streamer->cur_step > streamer->bDescr[i].step_r ) { // from now the number of stripes to be copied is decremented by 1 
			streamer->bDescr[i].step_n--;
			streamer->bDescr[i].step_r = streamer->steps; // guarantees that next times this clause should be skipped
		}

		// copy step_n stripes of i-th sub-block from file to buffer
		FILE *fid;
		fid = (FILE *) streamer->bDescr[i].fhandle;

		fseek (fid, 0, SEEK_END);
		//V3DLONG fileSize = ftell(fid); // unused
		rewind(fid);
			
		V3DLONG unitSize = streamer->bDescr[i].datatype; /* temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. */
		
		int bstridey = (int) (streamer->stridexy / streamer->stridex);                     // stridey of buffer
		int fstridey = (int) (streamer->bDescr[i].stridexy / streamer->bDescr[i].stridex); // stridey of file

		for ( int c=0; c<streamer->n_chans; c++ ) {

			// set initial pointer to buffer; each channel starts from the same relative position
			unsigned char *buftmp = streamer->buf + ((c * streamer->stridexyz) + 
				streamer->bDescr[i].boffs + streamer->stridex * (streamer->cur_step - 1)) * (int)unitSize; 
 
			// beginning of second slice in the buffer
			unsigned char *next_bslice = streamer->buf + ((c * streamer->stridexyz) + 
				streamer->bDescr[i].boffs + streamer->bDescr[i].height * streamer->stridex) * (int)unitSize; 

			// set initial position of file; each channel starts from the same relative position
			sint64 postmp = streamer->bDescr[i].header_len + ((c * streamer->bDescr[i].stridexyz) + 
				streamer->bDescr[i].foffs + streamer->bDescr[i].stridex * (streamer->cur_step - 1)) * (int)unitSize;                                       

			// beginning of second slice in the file
			sint64 next_fslice = streamer->bDescr[i].header_len + ((c * streamer->bDescr[i].stridexyz) + 
				streamer->bDescr[i].foffs + streamer->bDescr[i].height * streamer->bDescr[i].stridex) * (int)unitSize;  

			for (int s=0; s<streamer->bDescr[i].step_n; s++, 
														buftmp+=streamer->steps * streamer->stridex * (int)unitSize,
														postmp+=streamer->steps * streamer->bDescr[i].stridex * (int)unitSize ) { 
				// before the copy, the pointer to buffer and the file position has to be advanced of steps stripes

				if ( buftmp >= next_bslice ) { // moved to next buffer slice
					//further incease the pointer to buffer to skip other sub-blocks 
					buftmp += streamer->stridex * (bstridey - streamer->bDescr[i].height) * (int)unitSize;
					next_bslice += streamer->stridexy * (int)unitSize;
				}
				if ( postmp >= next_fslice ) { // moved to next file slice
					//further incease the position into file to skip other sub-blocks 
					postmp += streamer->bDescr[i].stridex * (fstridey - streamer->bDescr[i].height) * (int)unitSize;
					next_fslice += streamer->bDescr[i].stridexy * (int)unitSize;
				}

				// copy one stripe
				rewind(fid);
				fseek(fid,(long)postmp,SEEK_SET);
				int dummy = (int)ftell(fid);	
				dummy = (int)fread(buftmp,unitSize,streamer->bDescr[i].width,fid);

				// WARNING: code for testing
				if ( buffer2 ) { 
					for ( int j=0; j<streamer->bDescr[i].width; j++ ) {
						unsigned char *buftmp2 = buffer2 + (buftmp - streamer->buf);
						if ( buftmp[j]!=buftmp2[j] ) {
							//printf("%d %d\n",buftmp[j],buftmp2[j]);
							return ((char *)"Mismatch between streamed and non-streamed operation.\n");
						}
					}
				}
			}
		}
	}

	return 0;
}

char *streamer_cpydata ( Streamer_Descr_t *streamer, unsigned char *buffer, unsigned char *buffer2 ) {

	if ( streamer->cur_step > streamer->steps ) {
		return ((char *)"Too many steps in a streamed operation.\n");
	}

	for ( int i=0; i<streamer->n_blocks; i++ ) { 

		V3DLONG unitSize = streamer->bDescr[i].datatype; /* temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. */
	
		int bstridey = (int) (streamer->stridexy / streamer->stridex);                     // stridey of buffer
		//int fstridey = (int) (streamer->bDescr[i].stridexy / streamer->bDescr[i].stridex); // stridey of file (unused)

		for ( int c=0; c<streamer->n_chans; c++ ) {

			// set initial pointer to buffers; each channel starts from the same relative position
			unsigned char *sbuftmp = streamer->buf + ((c * streamer->stridexyz) + 
				streamer->bDescr[i].boffs + streamer->stridex * (streamer->cur_step - 1)) * (int)unitSize; 
			unsigned char *dbuftmp = buffer + (sbuftmp - streamer->buf);

			// beginning of second slice in the buffer
			unsigned char *next_bslice = streamer->buf + ((c * streamer->stridexyz) + 
				streamer->bDescr[i].boffs + streamer->bDescr[i].height * streamer->stridex) * (int)unitSize; 

			for (int s=0; s<streamer->bDescr[i].step_n; s++, 
														sbuftmp+=streamer->steps * streamer->stridex * (int)unitSize,
														dbuftmp+=streamer->steps * streamer->stridex * (int)unitSize ) { 
				// before the copy, the pointers to both buffer have to be advanced of steps stripes

				if ( sbuftmp >= next_bslice ) { // moved to next buffer slice
					//further incease the pointer to buffer to skip other sub-blocks 
					sbuftmp += streamer->stridex * (bstridey - streamer->bDescr[i].height) * (int)unitSize;
					dbuftmp += streamer->stridex * (bstridey - streamer->bDescr[i].height) * (int)unitSize;
					next_bslice += streamer->stridexy * (int)unitSize;
				}

				// copy one stripe
				memcpy(dbuftmp,sbuftmp,streamer->bDescr[i].width);
				//memcpy(dbuftmp,sbuftmp,streamer->stridex);

				// WARNING: code for testing
				if ( buffer2 ) { 
					for ( int j=0; j<streamer->bDescr[i].width; j++ ) {
						unsigned char *buftmp2 = buffer2 + (sbuftmp - streamer->buf);
						if ( dbuftmp[j]!=buftmp2[j] ) {
							//printf("%d %d\n",buftmp[j],buftmp2[j]);
							return ((char *)"Mismatch between streamed and non-streamed operation.\n");
						}
					}
				}
			}
		}
	}

	return 0;
}

char *streamer_close ( Streamer_Descr_t *streamer ) {

	for ( int i=0; i<streamer->n_blocks; i++ ) {
		closeRawFile(streamer->bDescr[i].fhandle);
	}

	return 0;
}


/***************************** Streamer_Descr_t METHODS *****************************/

Streamer_Descr_t::Streamer_Descr_t ( unsigned char *_buf, int _pxl_size, sint64 _stridex, sint64 _stridexy, sint64 _stridexyz, sint64 _n_chans, int _steps ) { 
	buf        = _buf; 
	pxl_size   = _pxl_size;
	stridex    = _stridex;
	stridexy   = _stridexy;
	stridexyz  = _stridexyz;
	n_chans    = _n_chans;
	steps      = _steps;
	max_blocks = DEFAULT_MAX_BLOCKS;
	n_blocks   = 0;
	bDescr     = new Block_Descr_t[DEFAULT_MAX_BLOCKS];
	cur_step  = 0; // the first step is 1
}

Streamer_Descr_t::~Streamer_Descr_t ( )
{

	if ( bDescr ) {
		for ( int i=0; i<n_blocks; i++ )
			delete bDescr[i].filename;
		delete[] bDescr;
	}
}

char *Streamer_Descr_t::addSubBlock ( char *filename, sint64 boffs, int sV0, int sV1, int sH0, int sH1, int sD0, int sD1 ) {

	// get file attributes
	char *err_rawfmt;
	V3DLONG *sz = 0;
	int datatype;
	int b_swap;
	int header_len;
	
	if ( (err_rawfmt = loadMetadata(filename,sz,datatype,b_swap,header_len)) != 0 ) {
		if ( sz ) delete[] sz;
		return err_rawfmt;
	}
	
	if ( datatype != 1 ) 
	{
		delete []sz;
		return ((char *)"Wrong file data type.\n");
	}

	// check if blocks are too many
	if ( n_blocks == max_blocks ) {
		Block_Descr_t *tmp = bDescr;
		max_blocks += DEFAULT_MAX_BLOCKS;
		bDescr = new Block_Descr_t[max_blocks];
		memcpy(bDescr,tmp,n_blocks*sizeof(Block_Descr_t));
		delete tmp;
	}

	// initializes a new block descriptor
	bDescr[n_blocks].stridex    = (int)sz[0];
	bDescr[n_blocks].stridexy   = (int)(sz[0]*sz[1]);
	bDescr[n_blocks].stridexyz  = (int)(sz[0]*sz[1]*sz[2]);
	bDescr[n_blocks].foffs      = (sH0 + sV0*bDescr[n_blocks].stridex + sD0*bDescr[n_blocks].stridexy) * datatype; // the header_len offset is added in the dostep operation
	bDescr[n_blocks].boffs      = boffs;
	bDescr[n_blocks].width      = sH1 - sH0;
	bDescr[n_blocks].height     = sV1 - sV0;
	bDescr[n_blocks].n_stripes  = (sV1 - sV0) * (sD1 - sD0);
	bDescr[n_blocks].step_n     = bDescr[n_blocks].n_stripes / this->steps + 1; // in case the remainder is not 0
	bDescr[n_blocks].step_r     = bDescr[n_blocks].n_stripes % this->steps;
	bDescr[n_blocks].fhandle    = 0;
	bDescr[n_blocks].filename   = new char[strlen(filename)+1];
	strcpy(bDescr[n_blocks].filename,filename);
	bDescr[n_blocks].datatype   = datatype;
	bDescr[n_blocks].header_len = header_len;

	n_blocks++,

	// terminates
	delete []sz;

	return 0;
}

/********* SUPPORT TO VISUALIZATION OF 16/32 BITS PER CHANNEL ***********/

char *convert2depth8bits ( int red_factor, sint64 totalBlockSize, sint64 sbv_channels, uint8 *&subvol ) {
	int c, i, j, p;
	sint64 totalUnits = totalBlockSize * sbv_channels;

	char endianCodeMachine = checkMachineEndian();
	if ( endianCodeMachine == 'L' ) {
		j = red_factor - 1; // MSB is the last
	}
	else if ( endianCodeMachine == 'B' ) {
		j = 0;              // MSB is the first
	}
	else {
		return ((char *) "unknown machine endianess"); 
	}

	// look for maximum values in each channel and rescale each channel separately
	unsigned short maxVal;
	unsigned short *temp = (unsigned short *) subvol;
	sint64 count;
	for ( c=0; c<sbv_channels; c++, temp+=totalBlockSize ) {
		for ( i=0, maxVal=0; i<totalBlockSize; i++ )
			if ( temp[i] > maxVal )
				maxVal = temp[i];
		for ( i=1, p=8*red_factor; i<maxVal; i<<=1, p-- )
			;
		// p represents the number of bits of the shift
		for ( i=0, count=0; i<totalBlockSize; i++ ) {
			if ( temp[i] > (0.5*maxVal) )
				count++;
            temp[i] <<= p;
		}
		printf("\t\t\t\tin RawFmtMngr - convert2depth8bits: c=%d, maxVal=%d, p=%d, count=%lld\n\n",c,maxVal,p,count);
	}
	
	uint8 *temp_buf = new uint8[totalUnits];
	memset(temp_buf,0,totalUnits);
	for ( i=0; i<totalUnits; i++, j+=red_factor )
		temp_buf[i] = subvol[j];

	delete[] subvol;
	subvol = temp_buf;

	return 0;
}




