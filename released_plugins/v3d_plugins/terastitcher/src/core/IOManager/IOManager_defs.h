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
*
*       Bria, A., et al., (2012) "Stitching Terabyte-sized 3D Images Acquired in Confocal Ultramicroscopy", Proceedings of the 9th IEEE International Symposium on Biomedical Imaging.
*       Bria, A., Iannello, G., "TeraStitcher - A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Microscopy Images", submitted for publication, 2012.
*
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

#ifndef _IO_MANAGER_DEFS_H
#define _IO_MANAGER_DEFS_H

/**************************************************************************************************************************
 *   GLOBAL definitions					   																			      *
 **************************************************************************************************************************/
#define IO_M_VERBOSE 0					//verbosity level of the IOManager module
#define IO_TIME_CALC					//if defined, I/O time measurements are enabled
#define IO_DEF_IMG_FORMAT "tif"			//default format of saved images
#define IO_DEF_IMG_DEPTH 8				//default bit depth of saved images
#define IO_STATIC_STRINGS_SIZE 5000		//size of static C-strings used for paths, messages, etc.
typedef float          real_t;			//real type definition (float for single precision, double for double precision)
typedef unsigned char  uint8;			//8-bit  unsigned integer data type
typedef unsigned short uint16;			//16-bit unsigned integer data type
typedef unsigned int   uint32;			//32-bit unsigned integer data type
typedef int			   sint32;			//32-bit signed integer data type
typedef long long	   sint64;			//64-bit signed integer data type

/**************************************************************************************************************************
 *	 UTILITY FUNCTIONS (compatible with both WIN/UNIX systems)															  *
 **************************************************************************************************************************/
#include <ctime>
#ifdef _WIN32
#define TIME( arg ) (((double) clock()) / CLOCKS_PER_SEC)
#define system_PAUSE() 		\
	system("PAUSE"); 		\
	cout<<endl;
#define system_CLEAR() system("cls");
#else
#define TIME( arg ) (time( arg ))
#define system_CLEAR() system("clear");
#define system_PAUSE()									\
	cout<<"\n\nPress RETURN key to continue..."<<endl<<endl;	\
	cin.clear();										\
	cin.ignore();										\
	cin.get();
#endif
#endif /* _IO_MANAGER_DEFS_H */
