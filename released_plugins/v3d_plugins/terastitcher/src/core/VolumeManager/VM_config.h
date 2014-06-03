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

#ifndef VM_CONFIG_H
#define VM_CONFIG_H

#include <string>
#include <cstdarg>
#include <vector>
#include <sstream>
#include <limits>
#include <cstring>
#include <math.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <ctime>
#include <direct.h>
#else
#include <time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#endif

namespace volumemanager
{

    /*******************
    *   INTERFACES     *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    class VirtualVolume;
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *    CONSTANTS     *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    const double PI = 3.14159265;
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *    MACROS        *
    ********************
    @warning: macros are not namespaced
    ---------------------------------------------------------------------------------------------------------------------------*/
    #define VM_VERBOSE 0					//verbosity level of current module
    #define VM_BIN_METADATA_FILE_NAME "mdata.bin"
    #define VM_STATIC_STRINGS_SIZE 5000
    #define S_TIME_CALC						//if enabled, single-phase processing time will be computed


    /*******************
    *    PARAMETERS    *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    extern std::string IMG_FILTER_REGEX;    // regular expression used to filter image filenames when the volume is imported
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /********************************************
     * Cross-platform UTILITY inline functions	*
     ********************************************
    ---------------------------------------------------------------------------------------------------------------------------*/
    // string-based sprintf function
    inline std::string strprintf(const std::string fmt, ...){
        int size = 100;
        std::string str;
        va_list ap;
        while (1) {
            str.resize(size);
            va_start(ap, fmt);
            int n = vsnprintf((char *)str.c_str(), size, fmt.c_str(), ap);
            va_end(ap);
            if (n > -1 && n < size) {
                str.resize(n);
                return str;
            }
            if (n > -1)
                size = n + 1;
            else
                size *= 2;
        }
        return str;
    }

    //returns true if the given path is a directory
    inline bool isDirectory(std::string path){
        struct stat s;
        if( stat(path.c_str(),&s) == 0 )
        {
            if( s.st_mode & S_IFDIR )
                return true;
            else if( s.st_mode & S_IFREG )
                return false;
            else return false;
        }
        else return false;
    }

    //returns true if the given path is a file
    inline bool isFile(std::string path){
        struct stat s;
        if( stat(path.c_str(),&s) == 0 )
        {
            if( s.st_mode & S_IFDIR )
                return false;
            else if( s.st_mode & S_IFREG )
                return true;
            else return false;
        }
        else return false;
    }

    //make dir
    #ifdef _WIN32
    #include <errno.h>
    inline bool make_dir(const char* arg){
        bool done = _mkdir(arg) == 0;
        bool result = done || errno != ENOENT;
        return result;
    }
    #else
    inline bool make_dir(const char* arg){
        bool done = mkdir(arg, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
        bool result = done || errno == EEXIST;
        return result;
    }
    #endif

    // check-and-makedir
    inline bool check_and_make_dir(const char *dirname){
        if(isDirectory(dirname))
            return true;
        else
            return make_dir(dirname);
    }



    //file deleting
    #ifdef _WIN32
    inline void delete_file( const char* arg ){
        if(system(strprintf("del /F /Q /S \"%s\"", arg).c_str())!=0)
            fprintf(stderr,"Can't delete file \"%s\"\n", arg);
    }
    #else
    inline void delete_file( const char* arg ){
        if(system(strprintf("rm -f \"%s\"", arg).c_str())!=0)
            fprintf(stderr,"Can't delete file \"%s\"\n", arg);
    }
    #endif

    //time computation
    #ifdef TIME_CALC
    #ifdef _WIN32
    #define TIME( arg ) (((double) clock()) / CLOCKS_PER_SEC)
    #else
    #define TIME( arg ) (time( arg ))
    #endif
    #endif


    //MAX
    #define ISR_MAX(a,b)       ( (a>b) ? (a) : (b) )

    //MAX
    #define ISR_MIN(a,b)       ( (a<b) ? (a) : (b) )

    //SIGN
    #define SIGN( arg )	   ( arg < 0 ? -1 : 1 )

    //ROUND
    #define ROUND( arg )   ( SIGN(arg) == 1 ? arg + 0.5 : arg - 0.5)

    //ALMOST EQUAL
    #define ALMOST_EQUAL(a,b) ( ( abs((a)-(b)) < 0.001 ) ? true : false )

    //INTEGER POW
    #define POW_INT(base,exp) ( (   (int) pow( (float)(base), exp)   ) )

    //SAFE DIVISION: when dividing by zero, <infinite> instead of +inf is returned
    #define SAFE_DIVIDE(dividend, divisor, infinite) ( (divisor)==0 ? (infinite) : ((dividend)/(divisor)) );    
    /*-------------------------------------------------------------------------------------------------------------------------*/
}

namespace vm = volumemanager;

#endif
