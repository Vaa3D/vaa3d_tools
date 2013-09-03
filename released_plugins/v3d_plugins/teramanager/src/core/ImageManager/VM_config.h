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

#ifndef VM_CONFIG_H
#define VM_CONFIG_H
#include <math.h>

//*** GLOBAL DEFINITIONS ****
#define PI 3.14159265					//pi
#define VM_VERBOSE 0					//verbosity level of current module
#define VM_BIN_METADATA_FILE_NAME "mdata.bin"
#define VM_STATIC_STRINGS_SIZE 5000
#define S_TIME_CALC						//if enabled, single-phase processing time will be computed

//enables Vaa3D-specific code in the ProgressBar.cpp file
//#define _VAA3D_PLUGIN_MODE

//*** FUNCTIONS DEFINITIONS ****

//time computation
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

//directory creation
#ifndef make_dir // to avoid double definitions
#ifdef _WIN32
#include <direct.h>
#define make_dir(V) _mkdir(V)
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#define make_dir(V) mkdir(V,S_IRWXU | S_IRWXG | S_IROTH | S_IWOTH | S_IXOTH)
#endif
#endif

//file deleting
#ifndef RM_FILE
#ifdef _WIN32
#define RM_FILE( arg ) \
	char sys_cmd[500]; \
	sprintf(sys_cmd, "del /F /Q %s", arg); \
	system(sys_cmd);
#else
#define RM_FILE( arg ) \
	char sys_cmd[500]; \
	sprintf(sys_cmd, "rm -f %s", arg); \
	system(sys_cmd);
#endif
#endif

//MAX
#define ISR_MAX(a,b)       ( (a>b) ? (a) : (b) )

//MAX
#define ISR_MIN(a,b)       ( (a<b) ? (a) : (b) )

//SIGN
#ifndef SIGN
#define SIGN( arg )	   ( arg < 0 ? -1 : 1 )
#endif

//ROUND
#define ROUND( arg )   ( SIGN(arg) == 1 ? arg + 0.5 : arg - 0.5)

//ALMOST EQUAL
#define ALMOST_EQUAL(a,b) ( ( abs((a)-(b)) < 0.001 ) ? true : false )

//INTEGER POW
#define POW_INT(base,exp) ( (   (int) pow( (float)(base), exp)   ) )

//SAFE DIVISION: when dividing by zero, <infinite> instead of +inf is returned
#define SAFE_DIVIDE(dividend, divisor, infinite) ( (divisor)==0 ? (infinite) : ((dividend)/(divisor)) );

#endif
