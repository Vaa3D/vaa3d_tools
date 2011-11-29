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
 *  stackutil-11.h
 *  
 *
 *  Created by Hanchuan Peng on 7/31/06.
 *  Copyright 2006 __Hanchuan Peng__. All rights reserved.
 *
 *  Updated: 060816: add tif support
 *  060828: add surfix extraction function here
 *  060920: add the 2-byte raw support for compatibility
 *  070214: add two simple interface to read any file formats supported (raw or tif) based on the surfix of filename
 *  070806: add LSM file reading functions
 *  070819: add LSM thumbnail stack reading and the middle-slice-only reading
 *  070823: add a more comprehensive function to read either 1 slice (original or thumbnail) or the entire stack (original or thumbnail) from lsm file
 *  080213: add functions to read/ single slice/thumbnail from tiff/raw stacks
 * 081204: add overload functions to read only a channel of a stack
 * 090413: add MRC rec file reading and writing
 * 090802: add .raw5d read and write interface
 * 100519: add v3d_basicdatatype.h
 * 100817: add mylib interface, PHC
 */

#ifndef __STACKUTIL__
#define __STACKUTIL__

//the folowing conditional compilation is added by PHC, 2010-05-20
#if defined (_MSC_VER)
#include "../basic_c_fun/vcdiff.h"
#else
#endif

extern "C" {
#include "tiffio.h"
};

#include "v3d_basicdatatype.h"

typedef char BIT8_UNIT;
typedef short int BIT16_UNIT;
typedef int BIT32_UNIT;
typedef V3DLONG BIT64_UNIT;

int loadMRC2Stack(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype); //simple MRC reading
int saveStack2MRC(const char * filename, const unsigned char * img, const V3DLONG * sz, int datatype); //simple MRC writing

int loadRaw5d2Stack(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype); //4-byte raw reading
int loadRaw5d2Stack(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype, int stack_id_to_load); //overload for convenience to read only 1 stack in a time series
int saveStack2Raw5d(const char * filename, const unsigned char * img, const V3DLONG * sz, int datatype); //4-byte raw writing


int loadRaw2Stack(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype); //4-byte raw reading

int loadRaw2Stack(char * filename, unsigned char * & img, V3DLONG * & sz, V3DLONG startx, V3DLONG starty, V3DLONG startz, 
				  V3DLONG endx,V3DLONG endy, V3DLONG endz, int & datatype); //4-byte raw reading


int loadRaw2Stack(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype, int chan_id_to_load); //overload for convenience to read only 1 channel

int loadRaw2Stack_resamping(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype, int resampling_size); //resampling image from tc file;
int loadRaw2Stack_raw_resamping(char * filename, unsigned char * & img, V3DLONG * & szo, V3DLONG *& sz, int & datatype, int &resampling_size);//resampling image from rawdata;
int Create_multi_levelraw(char * filename, unsigned char * & img, V3DLONG * & szo, V3DLONG *& sz, int & datatype, int resampling_size);


int saveStack2Raw(const char * filename, const unsigned char * img, const V3DLONG * sz, int datatype); //4-byte raw writing

int loadRaw2Stack_2byte(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype);
int loadRaw2Stack_2byte(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype, int chan_id_to_load); //overload for convenience to read only 1 channel
int saveStack2Raw_2byte(const char * filename, const unsigned char * img, const V3DLONG * sz, int datatype);

void swap2bytes(void *targetp);
void swap4bytes(void *targetp);
char checkMachineEndian();


//use libtiff to read tiff files. MUST < 2G
int loadTif2Stack(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype);
int loadTif2Stack(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype, int chan_id_to_load); //overload for convenience to read only 1 channel
int saveStack2Tif(const char * filename, const unsigned char * img, const V3DLONG * sz, int datatype);

//the following two functions are the major routines to load LSM file using libtiff, the file should have a size < 2G
int loadLsm2Stack_obsolete(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype); //070806
int loadLsm2Stack(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype); //070806
int loadLsm2Stack(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype, int chan_id_to_load);   //overload for convenience to read only 1 channel
int loadTif2Stack(char * filename, unsigned char * & img, V3DLONG * & sz, V3DLONG startx, V3DLONG starty, V3DLONG startz, 
				  V3DLONG endx,V3DLONG endy, V3DLONG endz, int & datatype);



//the following three functions are the low-level routines to load LSM file
int read_lsm_slice(TIFF *in, unsigned char * pointer_first_page, V3DLONG pagepixelnumber, V3DLONG channelpixelnumber, int datatype); 
int read_lsm_slice_strip(TIFF *in, unsigned char * pointer_first_page, V3DLONG pagepixelnumber, V3DLONG channelpixelnumber, int datatype);
int read_lsm_slice_tile(TIFF *in, unsigned char * pointer_first_page, V3DLONG pagepixelnumber, V3DLONG channelpixelnumber, int datatype);
int loadLsm2Stack_middle(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype); //070819

//read LSM thumbnail. 070819
int loadLsmThumbnail2Stack(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype);
int loadLsmThumbnail2Stack_middle(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype);

//a more comprehensive way to read LSM file. 070823
int loadLsmSlice(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype, V3DLONG sliceno, bool b_thumbnail);

//more comprehensive interfaces to read tiff and raw files. 080213
int loadTifSlice(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype, V3DLONG sliceno, bool b_thumbnail);
int read_tif_slice(TIFF *tif, unsigned char * pointer_first_page, V3DLONG width, V3DLONG height);

int read_tif_slice_strip(TIFF *in, unsigned char * pointer_first_page, V3DLONG pagepixelnumber, V3DLONG channelpixelnumber, int datatype);
int read_tif_slice_tile(TIFF *in, unsigned char * pointer_first_page, V3DLONG pagepixelnumber, V3DLONG channelpixelnumber, int datatype);

int loadRawSlice(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype, V3DLONG sliceno, bool b_thumbnail);
int loadRawSlice_2byte(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype, V3DLONG sliceno, bool b_thumbnail);

//
char * getSurfix(char *filename); //note that no need to delete the returned pointer as it is actually a location to the "filename"

bool ensure_file_exists_and_size_not_too_big(char *filename, V3DLONG sz_thres);

bool loadImage(char imgSrcFile[], unsigned char *& data1d, V3DLONG * &sz, int & datatype); //070215
bool loadImage(char imgSrcFile[], unsigned char *& data1d, V3DLONG * &sz, int & datatype, int chan_id_to_load); //081204
bool loadImage(char imgSrcFile[], unsigned char *& data1d, V3DLONG * &sz, V3DLONG *&szo, V3DLONG startx,V3DLONG starty,V3DLONG startz, 
			   V3DLONG endx,V3DLONG endy,V3DLONG endz,int & datatype); //

bool loadImage_resampling(char imgSrcFile[], unsigned char *& data1d, V3DLONG * &sz, V3DLONG *&szo, int & datatype,int resampling_size); //

bool loadImage_raw_resampling(char imgSrcFile[], unsigned char *& data1d, V3DLONG * &sz, V3DLONG *&szo, int & datatype,int &resampling_size); //

bool loadImage_mutil_levelraw(char imgSrcFile[], unsigned char *& data1d, V3DLONG * &szo, V3DLONG *&sz, int & datatype,int resampling_size);

bool saveImage(const char filename[], const unsigned char * data1d, const V3DLONG * sz, const int datatype); //070214

int get_channel_point(FILE * fid,V3DLONG head,V3DLONG channelid, V3DLONG sx, V3DLONG sy, V3DLONG sz,V3DLONG sc,V3DLONG unitSize);
int get_zslice_point(FILE * fid,V3DLONG head,V3DLONG channelid, V3DLONG zid, V3DLONG sx, V3DLONG sy, V3DLONG sz,V3DLONG sc,V3DLONG unitSize);
int get_c_z_y_point(FILE * fid,V3DLONG head,V3DLONG channelid, V3DLONG zid,V3DLONG yid, V3DLONG sx, V3DLONG sy, V3DLONG sz,V3DLONG sc,V3DLONG unitSize);
int get_c_z_y_x_point(FILE * fid,V3DLONG head,V3DLONG channelid, V3DLONG zid,V3DLONG yid,V3DLONG xid, V3DLONG sx, V3DLONG sy, V3DLONG sz,V3DLONG sc,V3DLONG unitSize);



#include "imageio_mylib.h" //100817. PHC

#endif



