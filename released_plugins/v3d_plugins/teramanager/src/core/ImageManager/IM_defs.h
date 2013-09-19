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

#ifndef _IM_DEFS_H
#define _IM_DEFS_H

/******************************
 ***    TYPES definitions   ***
 ******************************/
typedef unsigned char  uint8;			//8-bit  unsigned integer data type
typedef unsigned short uint16;			//16-bit unsigned integer data type
typedef unsigned int   uint32;			//32-bit unsigned integer data type
typedef int            sint32;                  //32-bit signed integer data type
typedef long long      sint64;			//64-bit signed integer data type
typedef float          REAL_T;


#define IM_VERBOSE 4
#define IM_SAVE_SUBVOLUMES
#define IM_METADATA_FILE_NAME "mdata.bin"
#define IM_CHANNEL_PREFIX "CH_"
#define IM_METADATA_FILE_VERSION 2
#define IM_STATIC_STRINGS_SIZE 3000
#define IM_DEF_IMG_FORMAT "tif"
#define IM_NUL_IMG_DEPTH 0
#define IM_DEF_IMG_DEPTH 8

//for time computation
#include <ctime>
#ifdef _WIN32
#define TIME( arg ) (((double) clock()) / CLOCKS_PER_SEC)
#else
#define TIME( arg ) (time( arg ))
#endif

#ifndef MAX
#define MAX(a,b)       (((a)>(b)) ? (a) : (b))
#endif
#ifndef SIGN // iannello ADDED
#define SIGN( arg ) (arg < 0 ? -1 : 1)
#endif // iannello ADDED
#define ROUND( arg ) ( SIGN(arg) == 1 ? arg + 0.5 : arg - 0.5)

//"PAUSE" function
#ifdef _WIN32
#define system_PAUSE() system("PAUSE"); cout<<endl;
#define system_CLEAR() system("cls");
#else
#define system_CLEAR() system("clear");
#define system_PAUSE()											\
	cout<<"\n\nPress RETURN key to continue..."<<endl<<endl;	\
	cin.clear();												\
	cin.ignore();												\
	cin.get();
#endif

//file deleting
#ifndef RM_FILE // iannello ADDED
#ifdef _WIN32
#define RM_FILE( arg ) 					\
	char sys_cmd[500]; 					\
	sprintf(sys_cmd, "del /F /Q %s", arg);  \
	if(system(sys_cmd)!=0)				\
		fprintf(stderr,"Can't delete file %s\n", arg);
#else
#define RM_FILE( arg ) \
	char sys_cmd[500]; \
	sprintf(sys_cmd, "rm -f %s", arg); \
	if(system(sys_cmd)!=0)				\
		fprintf(stderr,"Can't delete file %s\n", arg);
#endif
#endif // iannello ADDED

//directory creation
#ifndef make_dir // to avoid double definitions
#ifdef _WIN32
#include <direct.h>
#define make_dir(x) _mkdir(x)
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#define make_dir(x) mkdir(x,S_IRWXU | S_IRWXG | S_IROTH | S_IWOTH | S_IXOTH)
#endif
#endif

#endif //_IM_DEFS_H
