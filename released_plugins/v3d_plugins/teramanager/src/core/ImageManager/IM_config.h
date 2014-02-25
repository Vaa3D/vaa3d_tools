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


#ifndef _IM_CONFIG_H
#define _IM_CONFIG_H

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

//#ifdef __MACH__
//#include <mach/clock.h>
//#include <mach/mach.h>
//#endif

namespace IconImageManager
{

    /*******************
    *       TYPES      *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    typedef signed char	sint8;					//8-bit  signed   integers (-128                       -> +127)
    typedef short sint16;						//16-bit signed   integers (-32,768                    -> +32,767)
    typedef int sint32;							//32-bit signed   integers (-2,147,483,648             -> +2,147,483,647)
    typedef long long sint64;					//64-bit signed   integers (9,223,372,036,854,775,808 -> +9,223,372,036,854,775,807)
    typedef unsigned char uint8;				//8-bit  unsigned integers (0 -> +255)
    typedef unsigned short int uint16;			//16-bit unsigned integers (0 -> +65,535)
    typedef unsigned int uint32;				//32-bit unsigned integers (0 -> +4,294,967,295)
    typedef unsigned long long uint64;			//64-bit unsigned integers (0 -> +18,446,744,073,709,551,615
    typedef float real32;						//real single precision
    typedef double real64;						//real double precision
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *    CONSTANTS     *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    const std::string VERSION = "1.0.0";                        // version of current module
    const std::string MDATA_BIN_FILE_NAME  = "mdata.bin";       // name of binary metadata file
    const int         MDATA_BIN_FILE_VERSION = 2;               // version of binary metadata file
    const std::string MC_MDATA_BIN_FILE_NAME = "cmap.bin";      // name of binary metadata file for multichannel volumes
    const std::string CHANNEL_PREFIX = "CH_";                   // prefix identifying a folder containing data of a certain channel
    const std::string TIME_FRAME_PREFIX = "T_";                 // prefix identifying a folder containing data of a certain time frame
    const int         DEF_IMG_DEPTH = 8;                        // default image depth
    const int         NUL_IMG_DEPTH = 0;                        // invalid image depth
    const int         NATIVE_RTYPE = 0;                         // loadVolume returns 1 byte per channel type
    const std::string DEF_IMG_FORMAT = "tif";                   // default image format
    const int         STATIC_STRINGS_SIZE = 1024;               // size of static C-strings
    const std::string TILED_MC_FORMAT    = "Vaa3D raw (tiled, 4D)";    // unique ID for the TiledMCVolume class
    const std::string TILED_FORMAT       = "Vaa3D raw (tiled, RGB)";   // unique ID for the TiledVolume class
    const std::string STACKED_FORMAT     = "Image series (tiled)";     // unique ID for the StackedVolume class
    const std::string SIMPLE_FORMAT      = "Image series (nontiled)";  // unique ID for the SimpleVolume class
    const std::string SIMPLE_RAW_FORMAT  = "Vaa3D raw (series)";       // unique ID for the SimpleVolumeRaw class
    const std::string RAW_FORMAT         = "Vaa3D raw";                // unique ID for the RawVolume class
    const std::string TIF3D_FORMAT       = "3D TIFF (nontiled)";       // unique ID for the Tiff3DVolume class
    const std::string TILED_TIF3D_FORMAT = "3D TIFF (tiled)";          // unique ID for the TiledTiff3DVolume class
    const std::string TIME_SERIES        = "Time series";              // unique ID for the TimeSeries class

    const double      PI = 3.14159265;                          // pi
    const int         TMITREE_MAX_HEIGHT  = 10;                 // maximum depth of the TMITREE
    const int         TMITREE_MIN_BLOCK_DIM = 250;              // minimum dimension of TMITREE block along X/Y/Z
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *    PARAMETERS    *
    ******************** - default values have to be set in IM_config.cpp.
    ---------------------------------------------------------------------------------------------------------------------------*/
    extern int DEBUG;                                           // debug level of current module
    extern bool DEBUG_TO_FILE;                                  // whether debug messages should be printed on the screen or to a file (default: screen)
    extern std::string DEBUG_FILE_PATH;                         // filepath where to save debug information
    extern bool ADD_NOISE_TO_TIME_SERIES;                       // whether to mark individual frames of a time series with increasing gaussian noise
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *       ENUMS      *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    enum  axis        { vertical=1, inv_vertical=-1, horizontal=2, inv_horizontal=-2, depth=3, inv_depth=-3, axis_invalid=0};
    enum  debug_level { NO_DEBUG, LEV1, LEV2, LEV3, LEV_MAX };
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *      STRUCTS     *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    struct VHD_triple{int V, H, D;};
    struct interval_t{
        int start, end;
        interval_t(void) :				   start(-1),	  end(-1)  {}
        interval_t(int _start, int _end) : start(_start), end(_end){}
    };
    struct ref_sys{
        axis first, second, third;
        ref_sys(axis _first, axis _second, axis _third) : first(_first), second(_second), third(_third){}
        ref_sys(const ref_sys &_rvalue) : first(_rvalue.first), second(_rvalue.second), third(_rvalue.third){}
        ref_sys(): first(axis_invalid), second(axis_invalid), third(axis_invalid){}
    };


    /********************************************
     * Cross-platform UTILITY inline functions	*
     ********************************************
    ---------------------------------------------------------------------------------------------------------------------------*/
    // round functions
    inline int round(float  x) { return static_cast<int>(x > 0.0f ? x + 0.5f : x - 0.5f);}
    inline int round(double x) { return static_cast<int>(x > 0.0  ? x + 0.5  : x - 0.5 );}

    // sign function
    template <typename T> int sgn ( T x ){ return x < 0 ? -1 : 1;}

    // integer pow
    inline int powInt(int base, int exp){ return static_cast<int>( pow(static_cast<float>(base), exp)); }

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

    //returns true if the given string <fullString> ends with <ending>
    inline bool hasEnding (std::string const &fullString, std::string const &ending){
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
        } else {
            return false;
        }
    }

    //returns file extension, if any (otherwise returns "")
    inline std::string getFileExtension(const std::string& FileName){
        if(FileName.find_last_of(".") != std::string::npos)
            return FileName.substr(FileName.find_last_of(".")+1);
        return "";
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

    //time computation
//    #ifdef _WIN32
//    inline double getTimeSeconds(){
//        return static_cast<double>(clock()) / CLOCKS_PER_SEC;
//    }
//    #else
//        #ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
//        inline double getTimeSeconds()
//        {
//            clock_serv_t cclock;
//            mach_timespec_t mts;
//            host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
//            clock_get_time(cclock, &mts);
//            mach_port_deallocate(mach_task_self(), cclock);
//            return mts.tv_sec;
//        }
//        #else
//        inline double getTimeSeconds(){
//            timespec event;
//            clock_gettime(CLOCK_REALTIME, &event);
//            return (event.tv_sec*1000.0 + event.tv_nsec/1000000.0)/1000.0;
//        }
//        #endif
//    #endif


    //make dir
    #ifdef _WIN32
    #include <errno.h>
    inline bool makeDir(const char* arg){
//        printf("Creating directory \"%s\" ...", arg);
        bool done = _mkdir(arg) == 0;
        bool result = done || errno != ENOENT;
//        printf("%s\n", result? "DONE!" : "ERROR!");
        return result;
    }
    #else
    inline bool makeDir(const char* arg){
//        printf("Creating directory \"%s\" ...", arg);
        bool done = mkdir(arg, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
        bool result = done || errno == EEXIST;
//        printf("%s\n", result? "DONE!" : "ERROR!");
        return result;
    }
    #endif

    // check-and-makedir
    inline bool check_and_make_dir(const char *dirname){
        if(isDirectory(dirname))
            return true;
        else
            return makeDir(dirname);
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

    //cross-platform current function macro
    #if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600))
    # define __iim__current__function__ __PRETTY_FUNCTION__
    #elif defined(__DMC__) && (__DMC__ >= 0x810)
    # define __iim__current__function__ __PRETTY_FUNCTION__
    #elif defined(__FUNCSIG__)
    # define __iim__current__function__ __FUNCSIG__
    #elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
    # define __iim__current__function__ __FUNCTION__
    #elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
    # define __iim__current__function__ __FUNC__
    #elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
    # define __iim__current__function__ __func__
    #else
    # define __iim__current__function__ "(unknown)"
    #endif


    inline const char* axis_to_str(axis ax){
        if(ax == 1)
            return "Vertical";
        else if(ax == -1)
            return "Inverse Vertical";
        else if(ax == 2)
            return "Horizontal";
        else if(ax == -2)
            return "Inverse Horizontal";
        else if(ax == 3)
            return "Depth";
        else if(ax == -3)
            return "Inverse Depth";
        else return "<unknown>";
    }
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /***********************************************
    *    DEBUG, WARNING and EXCEPTION FUNCTIONS    *
    ************************************************
    ---------------------------------------------------------------------------------------------------------------------------*/
    inline void warning(const char* message, const char* source = 0)
    {
        if(DEBUG_TO_FILE)
        {
            FILE* f = fopen(DEBUG_FILE_PATH.c_str(), "a");
            if(source)
                fprintf(f, "\n**** WARNING (source: \"%s\") ****\n"
                "    |=> \"%s\"\n\n", source, message);
            else
                fprintf(f, "\n**** WARNING ****: %s\n", message);
            fclose(f);
        }
        else
        {
            if(source)
                printf("\n**** WARNING (source: \"%s\") ****\n"
                "    |=> \"%s\"\n\n", source, message);
            else
                printf("\n**** WARNING ****: %s\n", message);
        }
    }

    inline void debug(debug_level dbg_level, const char* message=0, const char* source=0)
    {
        if(DEBUG >= dbg_level){
            if(DEBUG_TO_FILE)
            {
                FILE* f = fopen(DEBUG_FILE_PATH.c_str(), "a");
                if(message && source)
                    fprintf(f, "\n--------------------- IconImageManager module: DEBUG (level %d) ----: in \"%s\") ----\n"
                             "                      message: %s\n\n", dbg_level, source, message);
                else if(message)
                    fprintf(f, "\n--------------------- IconImageManager module: DEBUG (level %d) ----: %s\n", dbg_level, message);
                else if(source)
                    fprintf(f, "\n--------------------- IconImageManager module: DEBUG (level %d) ----: in \"%s\"\n", dbg_level, source);
                fclose(f);
            }
            else
            {
                if(message && source)
                    printf("\n--------------------- IconImageManager module: DEBUG (level %d) ----: in \"%s\") ----\n"
                             "                      message: %s\n\n", dbg_level, source, message);
                else if(message)
                    printf("\n--------------------- IconImageManager module: DEBUG (level %d) ----: %s\n", dbg_level, message);
                else if(source)
                    printf("\n--------------------- IconImageManager module: DEBUG (level %d) ----: in \"%s\"\n", dbg_level, source);
            }
        }
    }


    class IOException
    {
        private:

            std::string source;
            std::string message;
            IOException(void);

        public:

            IOException(std::string _message, std::string _source = "unknown"){
                source = _source; message = _message;}
            ~IOException(void){}
            const char* what() const {return message.c_str();}
            const char* getSource() const {return source.c_str();}
    };
}
namespace iim = IconImageManager;	//a short alias for the current namespace

#endif //_IM_CONFIG_H


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
#endif
