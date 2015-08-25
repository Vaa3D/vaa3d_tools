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

/******************
*    CHANGELOG    *
*******************
* 2014-09-01. Alessandro. @ADDED template class 'interval'.
* 2014-08-25. Alessandro. @ADDED SPARSE_DATA parameter to turn on/off sparse data support.
*/

#ifndef VM_CONFIG_H
#define VM_CONFIG_H

#include <string>
#include <cstdarg>
#include <vector>
#include <sstream>
#include <limits>
#include <cstring>
#include <algorithm>
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
	class VirtualVolumeFactory;
	class VirtualStack;
	class StackedVolume;
	class BlockVolume;
	class Stack;
	class Block;
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *    CONSTANTS     *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
	const std::string MODULE_ID = "terastitcher::volumemanager";
    const double PI = 3.14159265;
	const std::string BINARY_METADATA_FILENAME =  "mdata.bin";	// binary metadata file name
	const float BINARY_METADATA_VERSION = 1.5;					// version of binary metadata file (used to check compatibility)
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *    MACROS        *
    ********************
    @warning: macros are not namespaced
    ---------------------------------------------------------------------------------------------------------------------------*/
    #define VM_VERBOSE 0					//verbosity level of current module    
    #define VM_STATIC_STRINGS_SIZE 5000
    #define S_TIME_CALC						//if enabled, single-phase processing time will be computed


    /*******************
    *    PARAMETERS    *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
	extern std::string VOLUME_INPUT_FORMAT_PLUGIN;	// plugin to manage the input volume format
	extern std::string VOLUME_OUTPUT_FORMAT_PLUGIN;	// plugin to manage the output volume format
    extern std::string IMG_FILTER_REGEX;    // regular expression used to filter image filenames when the volume is imported
    extern bool SPARSE_DATA;                // flag to turn on/off sparse data support
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    * BASIC STRUCTURES *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
	// 1D templated interval
    template <class T> struct interval{
        T start, end;
		interval() : start(-1), end(-1) {}
		interval(T _start, T _end) : start(_start), end(_end) {}
    };

	// axis
	enum axis {vertical=1, inv_vertical=-1, horizontal=2, inv_horizontal=-2, depth=3, inv_depth=-3, axis_invalid=0};	
	inline const char* axis_to_str(axis ax)
	{
		if(ax==axis_invalid)         return "axis_invalid";
		else if(ax==vertical)        return "vertical";
		else if(ax==inv_vertical)    return "inv_vertical";
		else if(ax==horizontal)      return "horizontal";
		else if(ax==inv_horizontal)  return "inv_horizontal";
		else if(ax==depth)           return "depth";
		else if(ax==inv_depth)       return "inv_depth";
		else                         return "unknown";
	}
	inline axis str2axis(const std::string & str)
	{
		if(str.compare("vertical") == 0				|| str.compare("1") == 0	|| str.compare("v") == 0	|| str.compare("V") == 0	|| str.compare("Y")== 0		|| str.compare("y")== 0)
			return vertical;
		else if(str.compare("inv_vertical") == 0	|| str.compare("-1") == 0	|| str.compare("-v") == 0	|| str.compare("-V") == 0	|| str.compare("-Y")== 0		|| str.compare("-y")== 0)
			return inv_vertical;
		else if(str.compare("horizontal") == 0		|| str.compare("2") == 0	|| str.compare("h") == 0	|| str.compare("H") == 0	|| str.compare("X")== 0		|| str.compare("x")== 0)
			return horizontal;
		else if(str.compare("inv_horizontal") == 0	|| str.compare("-2") == 0	|| str.compare("-h") == 0	|| str.compare("-H") == 0	|| str.compare("-X")== 0		|| str.compare("-x")== 0)
			return inv_horizontal;
		else if(str.compare("depth") == 0			|| str.compare("3") == 0	|| str.compare("d") == 0	|| str.compare("D") == 0	|| str.compare("Z")== 0		|| str.compare("z")== 0)
			return depth;
		else if(str.compare("inv_depth") == 0		|| str.compare("-3") == 0	|| str.compare("-d") == 0	|| str.compare("-D") == 0	|| str.compare("-Z")== 0		|| str.compare("-z")== 0)
			return inv_depth;
		else
			return axis_invalid;
	}

	// reference system	
	struct ref_sys 
	{
		axis first, second, third; 
		ref_sys(axis _first, axis _second, axis _third) : first(_first), second(_second), third(_third){}
		ref_sys(): first(axis_invalid), second(axis_invalid), third(axis_invalid){}
	};
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

	// removes all tab, space and newline characters from the given string
	inline std::string cls(std::string& string){
		string.erase(std::remove(string.begin(), string.end(), '\t'), string.end());
		string.erase(std::remove(string.begin(), string.end(), ' '),  string.end());
		string.erase(std::remove(string.begin(), string.end(), '\n'), string.end());
		return string;
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

	// tokenizer
	inline void	split(const std::string & theString, std::string delim, std::vector<std::string>& tokens)
	{
		size_t  start = 0, end = 0;
		while ( end != std::string::npos)
		{
			end = theString.find( delim, start);

			// If at end, use length=maxLength.  Else use length=end-start.
			tokens.push_back( theString.substr( start,
				(end == std::string::npos) ? std::string::npos : end - start));

			// If at end, use start=maxSize.  Else use start=end+delimiter.
			start = (   ( end > (std::string::npos - delim.size()) )
				?  std::string::npos  :  end + delim.size());
		}
	}

	//number to string conversion function and vice versa
	template <typename T>
	std::string num2str ( T Number ){
		std::stringstream ss;
		ss << Number;
		return ss.str();
	}
	template <typename T>
	T str2num ( const std::string &Text ){                              
		std::stringstream ss(Text);
		T result;
		return ss >> result ? result : 0;
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
