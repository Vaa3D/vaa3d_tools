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
*       Bria, A., Iannello, G., "TeraStitcher - A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Microscopy Images", (2012) BMC Bioinformatics, 13 (1), art. no. 316.
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
* 2014-09-02. Alessandro. @ADDED 'uint64' type.
*/


#ifndef _IO_MANAGER_DEFS_H
#define _IO_MANAGER_DEFS_H

#include <ctime>
#include <cstdarg>
#include <vector>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>

/******************************************************************************************************************************
 *   Interfaces, constants, enums, parameters, and cross-platform utility functions	                                          *
 ******************************************************************************************************************************/
namespace iomanager
{
	/*******************
	*    TYPES         *
	********************
	---------------------------------------------------------------------------------------------------------------------------*/
	typedef float				real_t;			// real type definition (float for single precision, double for double precision)
	typedef unsigned char		uint8;			// 8-bit  unsigned integer data type
	typedef unsigned short		uint16;			// 16-bit unsigned integer data type
	typedef unsigned int		uint32;			// 32-bit unsigned integer data type
	typedef int					sint32;			// 32-bit signed integer data type
	typedef long long			sint64;			// 64-bit signed integer data type
	typedef unsigned long long	uint64;			// 64-bit unsigned integer data type
	/*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *    INTERFACES    *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
	class IOPlugin;
	class IOPlugin2D;
	class IOPlugin3D;
	class IOPluginFactory;
	class exception;
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *    CONSTANTS     *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    const std::string DEF_IMG_FORMAT = "tif";   // default image format
    const int DEF_BPP = 8;                 // default image depth
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *       ENUMS      *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    enum debug_level { NO_DEBUG, LEV1, LEV2, LEV3, LEV_MAX };
    enum channel { ALL, R, G, B };
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *    PARAMETERS    *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    extern int DEBUG;                       // debug level of current module
    extern bool TIME_CALC;                  // whether to enable time measurements
    extern channel CHANS;					// channel to be loaded (default is ALL)
	extern std::string IMIN_PLUGIN;			// plugin to manage input image format
	extern std::string IMIN_PLUGIN_PARAMS;	// additional parameters <param1=val,param2=val,...> to the plugin for image input 
	extern std::string IMOUT_PLUGIN;		// plugin to manage output image format
	extern std::string IMOUT_PLUGIN_PARAMS;	// additional parameters <param1=val,param2=val,...> to the plugin for image output 
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /********************************************
     *   Cross-platform UTILITY functions	    *
     ********************************************
    ---------------------------------------------------------------------------------------------------------------------------*/

    //string-based sprintf function
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

	// removes all tab, space and newline characters from the given string
	inline std::string cls(std::string string){
		string.erase(std::remove(string.begin(), string.end(), '\t'), string.end());
		string.erase(std::remove(string.begin(), string.end(), ' '),  string.end());
		string.erase(std::remove(string.begin(), string.end(), '\n'), string.end());
		return string;
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

    //cross-platform current function macros (@WARNING: as they are macros, they are NOT namespaced)
    #if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600))
    # define __iom__current__function__ __PRETTY_FUNCTION__
    #elif defined(__DMC__) && (__DMC__ >= 0x810)
    # define __iom__current__function__ __PRETTY_FUNCTION__
    #elif defined(__FUNCSIG__)
    # define __iom__current__function__ __FUNCSIG__
    #elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
    # define __iom__current__function__ __FUNCTION__
    #elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
    # define __iom__current__function__ __FUNC__
    #elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
    # define __iom__current__function__ __func__
    #else
    # define __iom__current__function__ "(unknown)"
    #endif

    //time macros (@WARNING: as they are macros, they are NOT namespaced)
    #ifdef _WIN32
        #define TIME( arg ) (((double) clock()) / CLOCKS_PER_SEC)
    #else
        #define TIME( arg ) (time( arg ))
    #endif

	//clear macros (@WARNING: as they are macros, they are NOT namespaced)
	#ifdef _WIN32
	#define system_CLEAR() system("cls");
	#else
	#define system_CLEAR() system("clear");
	#endif


	
    /***********************************************
    *    DEBUG, WARNING and EXCEPTION FUNCTIONS    *
    ************************************************
    ---------------------------------------------------------------------------------------------------------------------------*/
    inline void warning(const char* message, const char* source = 0)
    {
        if(source)
           printf("\n**** WARNING (source: \"%s\") ****\n    |=> \"%s\"\n\n", source, message);
        else
           printf("\n**** WARNING ****: %s\n", message);
    }

    inline void debug(debug_level dbg_level, const char* message=0, const char* source=0)
    {
        if(DEBUG >= dbg_level)
        {
           if(message && source)
               printf("\n----------------------- iomanager module: DEBUG (level %d) ----: in \"%s\") ----\n"
                        "                        message: %s\n\n", dbg_level, source, message);
           else if(message)
               printf("\n----------------------- iomanager module: DEBUG (level %d) ----: %s\n", dbg_level, message);
           else if(source)
               printf("\n----------------------- iomanager module: DEBUG (level %d) ----: in \"%s\"\n", dbg_level, source);
        }
    }

	class exception  : public std::exception
	{
		private:

			std::string message;

		public:

			exception(const std::string & new_message, std::string source = ""){
				if(source.empty())
					message = new_message;
				else
					message = std::string("in ") + source + ": " + new_message;
			}
			virtual ~exception() throw (){}
			virtual const char* what() const throw (){return message.c_str();}
	};



}

namespace iom = iomanager;

#endif /* _IO_MANAGER_DEFS_H */
