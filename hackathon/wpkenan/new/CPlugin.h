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
*       Bria, A., Iannello, G., "A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Datasets", submitted on July 2012 to IEEE Transactions on Information Technology in Biomedicine.
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

// 20170623 RZC: change __TERAMANAGER_C_PLUGIN_H__ to __TERAFLY_C_PLUGIN_H__ for merging mozak/terafly
//#ifndef __TERAMANAGER_C_PLUGIN_H__
//#define __TERAMANAGER_C_PLUGIN_H__
#ifndef __TERAFLY_C_PLUGIN_H__
#define __TERAFLY_C_PLUGIN_H__


#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
#include <QWidget>
#else
#include <QtGui>
#endif
#include <limits>
#include <sstream>
#include <algorithm>
#include <QThread>
#include "v3d_core.h"
#include <cmath>
#include "v3d_interface.h"

class V3DPluginCallback2;

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

/*******************************************************************************************************************************
 *   Interfaces, types, parameters and constants   													                           *
 *******************************************************************************************************************************/
namespace terafly
{
    /***************************
    *    CLASS DECLARATIONS    *
    ****************************
    ---------------------------------------------------------------------------------------------------------------------------*/
    class TeraFly;              //the class defined in this header and derived from V3DPluginInterface2_1
    class PMain;                //main presentation class: it contains the main frame
    class PDialogImport;        //presentation class for the import dialog
    class PConverter;           //presentation class for the volume converter dialog
    class PDialogProofreading;  //presentation class for the proofreading dialog
    class PAbout;               //about window
    class PLog;                 //log window
    class PAnoToolBar;          //annotation toolbar
    class CImport;              //control class to perform the import step in a separate non-GUI-blocking thread
    class CVolume;              //control class to perform data loading in a separate non-GUI-blocking thread
    class CSettings;            //control class to manage persistent platform-independent application settings
    class CViewer;              //control class used to encapsulate all the informations needed to manage 3D navigation windows
    class CConverter;           //control class used to perform volume conversion operations in a separate non-GUI-blocking thread
    class CAnnotations;         //control class used to manage annotations (markers, curves, etc.) among all the resolutions
    class CImageUtils;          //control class containing image processing functions
    class COperation;           //control class to keep track of performed operations
    class QArrowButton;         //Qt-customized class to model arrow buttons
    class QHelpBox;             //Qt-customized class to model help box
    class QGradientBar;         //Qt-customized class to model a gradient-colored bar
    class QLineTree;            //Qt-customized class to model a three-lined tree
    class QGLRefSys;            //Qt-customized OpenGL widget to render the XYZ reference system applied to a 3D cube
    class QPixmapToolTip;       //Qt-customized class to model pixmap tooltips
    class QUndoMarkerCreate;    //QUndoCommand for marker creation
    class QUndoMarkerDelete;    //QUndoCommand for marker deletion
    class QUndoMarkerDeleteROI; //QUndoCommand for marker in ROI deletion
    class QUndoVaa3DNeuron;     //QUndoCommand for Vaa3D neuron editing
    class VirtualPyramid;       //entity class to model virtual pyramid image where data are filled online from an unconverted highres image
    class VirtualPyramidLayer;  //entity class to model a virtual pyramid layer/volume
    class HyperGridCache;  //entity class to model a virtual pyramid cache
    class myRenderer_gl1;       //Vaa3D-inhrerited class
    class myV3dR_GLWidget;      //Vaa3D-inhrerited class
    class myV3dR_MainWindow;    //Vaa3D-inhrerited class
    class myImage4DSimple;      //Vaa3D-inhrerited class
    class PluginInterface;
    struct annotation;          //base class for annotations
    struct volume_format;       //enum-like class to distinguish different volume formats
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /********************************
    *    UTILITY CLASSES AND ENUMS  *
    *********************************
    ---------------------------------------------------------------------------------------------------------------------------*/
    // interval type
    struct interval_t
    {
        int start, end;
        interval_t(void) : start(-1), end(-1)  {}
        interval_t(int _start, int _end) : start(_start), end(_end){}
    };

    // block type
    struct block_t
    {
        interval_t xInt, yInt, zInt;
        block_t(interval_t _xInt, interval_t _yInt, interval_t _zInt) : xInt(_xInt), yInt(_yInt), zInt(_zInt){}
    };

    // sleeper
    class Sleeper : public QThread
    {
        public:
            static void usleep(unsigned long usecs){QThread::usleep(usecs);}
            static void msleep(unsigned long msecs){QThread::msleep(msecs);}
            static void sleep(unsigned long secs){QThread::sleep(secs);}
    };

    // custom exception
    class RuntimeException : public std::exception
    {
        private:

            std::string source;
            std::string message;
            RuntimeException(void);

        public:

            RuntimeException(std::string _message, std::string _source = "unknown"){
                source = _source; message = _message;}
            virtual ~RuntimeException() throw(){}
            virtual const char* what() throw() {return message.c_str();}
            const char* getSource() const {return source.c_str();}
    };

    // emulate initializer list for STL vector
    template <typename T>
    class make_vector
    {
        public:

          typedef make_vector<T> my_type;
          my_type& operator<< (const T& val) {
            data_.push_back(val);
            return *this;
          }
          operator std::vector<T>() const {
            return data_;
          }
        private:
          std::vector<T> data_;
    };

    enum  debug_level { NO_DEBUG, LEV1, LEV2, LEV3, LEV_MAX };  // debug levels
    enum  debug_output { TO_STDOUT, TO_GUI, TO_FILE};           // where debug messages should be printed
    enum  direction {x, y, z};                                  // axis direction
    /*-------------------------------------------------------------------------------------------------------------------------*/



    /*******************
    *    CONSTANTS     *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    const double pi = 3.14159265359;
    const std::string VMAP_BIN_FILE_NAME = "vmap.bin";      // name of volume map binary file
    const std::string RESOLUTION_PREFIX = "RES";            // prefix identifying a folder containing data of a certain resolution
    const char   undefined_str[] = "undefined";
    const int    ZOOM_HISTORY_SIZE = 3;
    /*-------------------------------------------------------------------------------------------------------------------------*/



    /*******************
    *    PARAMETERS    *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    extern std::string version;                 // version number of current module
    extern int DEBUG;							// debug level of current module
    extern debug_output DEBUG_DEST;             // where debug messages should be print (default: stdout)
    extern std::string DEBUG_FILE_PATH;         // filepath where to save debug information
    /*-------------------------------------------------------------------------------------------------------------------------*/



    /*******************
    *    TYPES         *
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
    typedef std::vector<int> integer_array;     //need to typedef this so as to register with qRegisterMetaType
    /*-------------------------------------------------------------------------------------------------------------------------*/



    /*******************
    *  CONCURRENCY     *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    extern QMutex updateGraphicsInProgress;
    /*-------------------------------------------------------------------------------------------------------------------------*/



    /********************************************
     *   Cross-platform UTILITY functions	    *
     ********************************************
    ---------------------------------------------------------------------------------------------------------------------------*/
    // round functions
    inline int round(float  x) { return static_cast<int>(x > 0.0f ? x + 0.5f : x - 0.5f);}
    inline int round(double x) { return static_cast<int>(x > 0.0  ? x + 0.5  : x - 0.5 );}
    // special round functions: ceil if x > 0.5
    inline int round05(float x)  { return x - int(x) > 0.5 ? round(x) : int(x);}
    inline int round05(double x) { return x - int(x) > 0.5 ? round(x) : int(x);}

    // positive and negative infinity
    template<typename T>
    T infp()
    {
        if(std::numeric_limits<T>::has_infinity)
            return std::numeric_limits<T>::infinity();
        else
            return std::numeric_limits<T>::max();
    }
    template<typename T>
    T infn()
    {
        if(std::numeric_limits<T>::has_infinity)
            return -std::numeric_limits<T>::infinity();
        else
            return std::numeric_limits<T>::min();
    }

    //string-based sprintf function
    inline std::string strprintf(const std::string fmt, ...)
    {
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

    // split
    inline void	split(const std::string& theString, std::string delim, std::vector<std::string>& tokens)
    {
        tokens.clear();
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

    inline std::vector<std::string> parse(const std::string & line, const std::string & delim, int nTokensExpected, const std::string & filename) throw (RuntimeException)
    {
        std::vector<std::string> tokens;
        split(line, delim, tokens);
        if(tokens.size() != nTokensExpected)
            throw RuntimeException(strprintf("in file \"%s\", line \"%s\": expected %d \"%s\"-separated tokens, found %d",
                                   filename.c_str(), line.c_str(), nTokensExpected, delim.c_str(), tokens.size()));
        return tokens;
    }

    inline void parse(const std::string & line, const std::string & delim, int nTokensExpected, const std::string & filename, std::vector<std::string> &tokens) throw (RuntimeException)
    {;
        tokens.clear();
        split(line, delim, tokens);
        if(tokens.size() != nTokensExpected)
            throw RuntimeException(strprintf("in file \"%s\", line \"%s\": expected %d \'%s\'-separated tokens, found %d",
                                   filename.c_str(), line.c_str(), nTokensExpected, delim.c_str(), tokens.size()));
    }

    //returns true if the given string <fullString> ends with <ending>
    inline bool hasEnding (std::string const &fullString, std::string const &ending)
    {
        if (fullString.length() >= ending.length())
            return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
        else
            return false;
    }

    //returns file extension, if any (otherwise returns "")
    inline std::string getFileExtension(const std::string& FileName)
    {
        if(FileName.find_last_of(".") != std::string::npos)
            return FileName.substr(FileName.find_last_of(".")+1);
        return "";
    }

    // removes all tab, space and newline characters from the given string (in-place version and copy-based version)
    inline std::string clsi(std::string& string)
    {
        string.erase(std::remove(string.begin(), string.end(), '\t'), string.end());
        string.erase(std::remove(string.begin(), string.end(), ' '),  string.end());
        string.erase(std::remove(string.begin(), string.end(), '\n'), string.end());
        string.erase(std::remove(string.begin(), string.end(), '\r'), string.end());
        return string;
    }
    inline std::string cls(std::string string)
    {
        string.erase(std::remove(string.begin(), string.end(), '\t'), string.end());
        string.erase(std::remove(string.begin(), string.end(), ' '),  string.end());
        string.erase(std::remove(string.begin(), string.end(), '\n'), string.end());
        string.erase(std::remove(string.begin(), string.end(), '\r'), string.end());
        return string;
    }


    //extracts the filename from the given path and stores it into <filename>
    inline std::string getFileName(std::string const & path, bool save_ext = true)
    {
        std::string filename = path;

        // Remove directory if present.
        // Do this before extension removal in case directory has a period character.
        const size_t last_slash_idx = filename.find_last_of("\\/");
        if (std::string::npos != last_slash_idx)
            filename.erase(0, last_slash_idx + 1);

        // Remove extension if present.
        if(!save_ext)
        {
            const size_t period_idx = filename.rfind('.');
            if (std::string::npos != period_idx)
                filename.erase(period_idx);
        }

        return filename;
    }

    // changes directory by moving one directory up from the current directory
    inline std::string cdUp(std::string const & path)
    {
        return path.substr(0, path.find_last_of("/\\"));
    }

    // removes carriage return characters
    inline std::string clcr(const std::string & _str)
    {
        std::string str = _str;
        str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
        str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
        return str;
    }

    //number to string conversion function and vice versa
    template <typename T>
    std::string num2str ( T Number )
    {
        std::stringstream ss;
        ss << Number;
        return ss.str();
    }
    template <typename T>
    T str2num ( const std::string &Text )
    {
        std::stringstream ss(Text);
        T result;
        return ss >> result ? result : 0;
    }


    template<class T>
    inline static const T& kClamp( const T& x, const T& low, const T& high )
    {
        if ( x < low )       return low;
        else if ( high < x ) return high;
        else                 return x;
    }

    template<class T>
    inline static const T saturate_trim(T value, T limit){return value <= limit ? value : limit;}

    // linear interpolation
    template <typename T>
    inline static T linear(T a, T b, float t)
    {
        return a * (1 - t) + b * t;
    }
    template <typename T>
    inline static T linear(T a, T b, int step_index, int steps_number)
    {
        return (b - a) * step_index / static_cast<float>(steps_number) + a;
    }

    // partition a discrete range into subranges which differ by 1 at most
    template <typename T>
    inline std::vector<T> partition(T range, T desired_part_size) throw (RuntimeException)
    {
        if(range <= 0)
            throw RuntimeException(strprintf("in partition(): range is <= 0 (%s)", num2str<T>(range).c_str()));
        if(desired_part_size <=0)
            throw RuntimeException(strprintf("in partition(): desired_part_size is <= 0 (%s)", num2str<T>(desired_part_size).c_str()));
        std::vector<T> subranges(static_cast<T>(ceil(range/(float)desired_part_size )));
        for(int i=0; i<subranges.size(); i++)
            subranges[i] =  range/subranges.size() + (i < range % subranges.size() ? 1:0 );
        return subranges;
    }

    //cross-platform current function macro
    #if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600))
    # define __itm__current__function__ __PRETTY_FUNCTION__
    #elif defined(__DMC__) && (__DMC__ >= 0x810)
    # define __itm__current__function__ __PRETTY_FUNCTION__
    #elif defined(__FUNCSIG__)
    # define __itm__current__function__ __FUNCSIG__
    #elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
    # define __itm__current__function__ __FUNCTION__
    #elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
    # define __itm__current__function__ __FUNC__
    #elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
    # define __itm__current__function__ __func__
    #else
    # define __itm__current__function__ "(unknown)"
    #endif
    /*-------------------------------------------------------------------------------------------------------------------------*/

    /***********************************************
    *    DEBUG, WARNING and EXCEPTION FUNCTIONS    *
    ************************************************
    ---------------------------------------------------------------------------------------------------------------------------*/
    inline std::string shortFuncName(const std::string & longname)
    {
		std::vector <std::string> tokens;
		split(longname, "::", tokens);
		tokens[0] = tokens[0].substr(tokens[0].rfind(" ")+1);
		int k=1; bool found =false;
		for(;k<tokens.size() && !found; k++)
			if(tokens[k].find("(") != std::string::npos)
			{
				found = true;
				tokens[k] = tokens[k].substr(0,tokens[k].find("("));
			}
		std::string result=tokens[0];
		for(int i=1; i<k; i++)
			result = result + "::" +  tokens[i];
		return result;

	}
    inline void warning(const char* message, const char* source = 0)
    {
        if(DEBUG_DEST == TO_FILE)
        {
            FILE* f = fopen(DEBUG_FILE_PATH.c_str(), "a");
            if(source)
                fprintf(f, "\n**** WARNING (source: \"%s\") ****\n"
                "    |=> \"%s\"\n\n", shortFuncName(source).c_str(), message);
            else
                fprintf(f, "\n**** WARNING ****: %s\n", message);
            fclose(f);
        }
        else if(DEBUG_DEST == TO_STDOUT)
        {
            if(source)
                printf("\n**** WARNING (source: \"%s\") ****\n"
                "    |=> \"%s\"\n\n", shortFuncName(source).c_str(), message);
            else
                printf("\n**** WARNING ****: %s\n", message);
        }
        else if(DEBUG_DEST == TO_GUI)
        {
            if(source)
                v3d_msg(strprintf("\n**** WARNING (source: \"%s\") ****\n"
                "    |=> \"%s\"\n\n", shortFuncName(source).c_str(), message).c_str());
            else
                v3d_msg(strprintf("\n**** WARNING ****: %s\n", message).c_str());
        }
    }


    inline void debug(debug_level dbg_level, const char* message=0, const char* source=0, bool short_print = false)
    {
        if(DEBUG >= dbg_level)
        {
            if(DEBUG_DEST == TO_FILE)
            {
                FILE* f = fopen(DEBUG_FILE_PATH.c_str(), "a");
				if(message && source && !short_print)
					fprintf(f,"\n---(debug level %d)--- in \"%s\"\n"
					            "                             message: %s\n\n", dbg_level, shortFuncName(source).c_str(), message);
				else if(message && !short_print)
					fprintf(f,"\n---(debug level %d)---       message: %s\"\n\n", dbg_level, message);
				else if(source && !short_print)
					fprintf(f,"\n---(debug level %d)--- in \"%s\"\n", dbg_level, shortFuncName(source).c_str());
				else if(short_print && message)
					fprintf(f,"\n                             message: %s\n", message);
                fclose(f);
            }
            else if (DEBUG_DEST == TO_STDOUT)
            {
                if(message && source && !short_print)
                    printf("\n---(debug level %d)--- in \"%s\"\n"
                             "                             message: %s\n\n", dbg_level, shortFuncName(source).c_str(), message);
                else if(message && !short_print)
					printf("\n---(debug level %d)---       message: %s\"\n\n", dbg_level, message);
                else if(source && !short_print)
                    printf("\n---(debug level %d)--- in \"%s\"\n", dbg_level, shortFuncName(source).c_str());
                else if(short_print && message)
					printf("\n                             message: %s\n", message);
            }
            else if (DEBUG_DEST == TO_GUI)
            {
                if(message && source && !short_print)
                    v3d_msg(strprintf("\n---(debug level %d)--- in \"%s\"\n"
                             "                             message: %s\n\n", dbg_level, shortFuncName(source).c_str(), message).c_str());
                else if(message && !short_print)
                    v3d_msg(strprintf("\n---(debug level %d)---       message: %s\"\n\n", dbg_level, message).c_str());
                else if(source && !short_print)
                    v3d_msg(strprintf("\n---(debug level %d)--- in \"%s\"\n", dbg_level, shortFuncName(source).c_str()).c_str());
                else if(short_print && message)
                    v3d_msg(strprintf("\n                             message: %s\n", message).c_str());
            }
        }
    }
    /*-------------------------------------------------------------------------------------------------------------------------*/




    /********************************
    *    OTHER UTILITY CLASSES      *
    *********************************
    ---------------------------------------------------------------------------------------------------------------------------*/
    // 3D point (x + y + z)
    template<class T>
    struct xyz
    {
        T x,y,z;
        xyz(void) : x(0), y(0), z(0){}
        xyz(T v) : x(v), y(v), z(v){}
        xyz(T _x, T _y, T _z) : x(_x), y(_y), z(_z){}
        xyz(XYZ &p) : x(p.x), y(p.y), z(p.z){}

        bool operator == (const xyz &p) const{
            return p.x == x && p.y == y && p.z == z;
        }

        bool operator <  (const xyz &p) const{
            return p.x < x && p.y < y && p.z < z;
        }

		const T& operator[](size_t i) const{
			if(i==0)
				return x;
			else if(i==1)
				return y;
			else 
				return z;
		}
		T& operator[](size_t i){
			if(i==0)
				return x;
			else if(i==1)
				return y;
			else 
				return z;
		}

        xyz<T> operator*(T s) const
		{
			xyz<T> res(x,y,z);
			res.x *= s;
			res.y *= s;
			res.z *= s;
			return res;
		}

        template <typename T2>
        operator xyz<T2>() const {
            return xyz<T2>(T2(x),T2(y),T2(z));
        }

        std::string toString(){
            return strprintf("{%s,%s,%s}", num2str<T>(x).c_str(), num2str<T>(y).c_str(), num2str<T>(z).c_str());
        }

        static xyz<T> biggest(){
            return xyz<T> (std::numeric_limits<T>::has_infinity ? std::numeric_limits<T>::infinity() : std::numeric_limits<T>::max());
        }
    };

    // 4D point (x + y + z + time)
    template<class T>
    struct xyzt
    {
        T x,y,z,t;
        xyzt(void) : x(0), y(0), z(0), t(0){}
        xyzt(T _x, T _y, T _z, T _t) : x(_x), y(_y), z(_z), t(_t){}

        template <typename T2>
        operator xyzt<T2>() const {
            return xyzt<T2>(T2(x),T2(y),T2(z),T2(t));
        }
    };

    // 5D point (x + y + z + channel + time)
    template<class T>
    struct xyzct
    {
        T x,y,z,c,t;
        xyzct(void) : x(0), y(0), z(0), c(0), t(0){}
        xyzct(T _x, T _y, T _z, T _c=0, T _t=0) : x(_x), y(_y), z(_z), c(_c), t(_t){}

        const T& operator[](size_t i) const{
            if(i==0)
                return x;
            else if(i==1)
                return y;
            else if(i==2)
                return z;
            else if(i==3)
                return c;
            else
                return t;
        }
        T& operator[](size_t i){
            if(i==0)
                return x;
            else if(i==1)
                return y;
            else if(i==2)
                return z;
            else if(i==3)
                return c;
            else
                return t;
        }

        std::string toString(){
            return strprintf("%s(x) x %s(y) x %s(z) x %s(c) x %s(t)", num2str<T>(x).c_str(), num2str<T>(y).c_str(), num2str<T>(z).c_str(), num2str<T>(c).c_str(), num2str<T>(t).c_str());
        }

        T size(){return x*y*z*c*t;}

        xyz<T> toXYZ(){
            return xyz<T>(x,y,z);
        }
    };

    // 4D Volume Of Interest
    template<class T>
    struct voi4D
    {
        xyzt<T> start;
        xyzt<T> end;
        voi4D() : start(0,0,0,0), end(0,0,0,0){}
        voi4D(xyzt<T> _start, xyzt<T> _end) : start(_start), end(_end){}
        xyzt<size_t> dims(){
            return xyzt<size_t>(
                        size_t(end.x > start.x ? end.x-start.x : 0),
                        size_t(end.y > start.y ? end.y-start.y : 0),
                        size_t(end.z > start.z ? end.z-start.z : 0),
                        size_t(end.t > start.t ? end.t-start.t : 0));
        }
        size_t size(){
            xyzt<size_t> _dims=dims(); return _dims.x*_dims.y*_dims.z*_dims.t;
        }

        bool isValid(){
            return end.x > start.x && end.y > start.y && end.z > start.z && end.t > start.t;
        }

        voi4D<T>& operator/=(xyz<T> s){
            start.x /= T(s.x);
            start.y /= T(s.y);
            start.z /= T(s.z);
            end.x   /= T(s.x);
            end.y   /= T(s.y);
            end.z   /= T(s.z);
            return *this;
		}

        voi4D<T>& operator*=(xyz<T> s){
            start.x *= T(s.x);
            start.y *= T(s.y);
            start.z *= T(s.z);
            end.x   *= T(s.x);
            end.y   *= T(s.y);
            end.z   *= T(s.z);
            return *this;
        }

        voi4D<T>& operator-=(xyz<T> s){
            start.x -= T(s.x);
            start.y -= T(s.y);
            start.z -= T(s.z);
            end.x   -= T(s.x);
            end.y   -= T(s.y);
            end.z   -= T(s.z);
            return *this;
        }

        voi4D<T>& operator-=(xyzt<size_t> s){
            start.x -= T(s.x);
            start.y -= T(s.y);
            start.z -= T(s.z);
            start.t -= T(s.t);
            end.x   -= T(s.x);
            end.y   -= T(s.y);
            end.z   -= T(s.z);
            end.t   -= T(s.t);
            return *this;
        }

        voi4D<T>& operator-=(xyzct<size_t> s){
            start.x -= T(s.x);
            start.y -= T(s.y);
            start.z -= T(s.z);
            start.t -= T(s.t);
            end.x   -= T(s.x);
            end.y   -= T(s.y);
            end.z   -= T(s.z);
            end.t   -= T(s.t);
            return *this;
        }

        voi4D<T>& operator+=(xyz<T> s){
            start.x += T(s.x);
            start.y += T(s.y);
            start.z += T(s.z);
            end.x   += T(s.x);
            end.y   += T(s.y);
            end.z   += T(s.z);
            return *this;
        }

        template <typename T2>
        operator voi4D<T2>() const {
            voi4D<T2> voi;
            voi.start.x = T2(start.x);
            voi.start.y = T2(start.y);
            voi.start.z = T2(start.z);
            voi.start.t = T2(start.t);
            voi.end.x = T2(end.x);
            voi.end.y = T2(end.y);
            voi.end.z = T2(end.z);
            voi.end.t = T2(end.t);
            return voi;
        }

        voi4D<int> rounded() const{
            voi4D<int> voi;
            voi.start.x = round05(start.x);
            voi.start.y = round05(start.y);
            voi.start.z = round05(start.z);
            voi.start.t = round(start.t);
            voi.end.x = round(end.x);
            voi.end.y = round(end.y);
            voi.end.z = round(end.z);
            voi.end.t = round(end.t);
            return voi;
        }

        /*voi4D<int> ceil() const{
            voi4D<int> voi;
            voi.start.x = std::floor(start.x);
            voi.start.y = std::floor(start.y);
            voi.start.z = std::floor(start.z);
            voi.start.t = std::floor(start.t);
            voi.end.x = std::ceil(end.x);
            voi.end.y = std::ceil(end.y);
            voi.end.z = std::ceil(end.z);
            voi.end.t = std::ceil(end.t);
            return voi;
        }*/
    };

    template<class T = unsigned int>
    struct active_channels
    {
        T* table;
        size_t dim;
        active_channels() : table(0), dim(0){}
        active_channels(T* _table, size_t _dim) : table(_table), dim(_dim){}
        std::string toString() const{
            std::string res;
            for(size_t i=0; i<dim; i++)
                res = res+num2str<T>(table[i]) + (i < dim-1 ? "," :  "");
            return res;
        }
        static std::string toString(std::vector<T> chans){
            std::string res;
            for(size_t i=0; i<chans.size(); i++)
                res = res+num2str<T>(chans[i]) + (i < chans.size()-1 ? "," :  "");
            return res;
        }

        // Copy constructor
        active_channels(const active_channels & chans)
        {
            // do the copy
            dim = chans.dim;
            table = new T[dim];
            for(size_t i=0; i<dim; i++)
                table[i] = chans.table[i];
        }

        // Assignment operator
        active_channels& operator= (const active_channels & chans)
        {
            // self-assignment guard
            if (this == &chans)
                return *this;

            // do the copy
            dim = chans.dim;
            table = new T[dim];
            for(size_t i=0; i<dim; i++)
                table[i] = chans.table[i];

            // return the existing object so we can chain this operator
            return *this;
        }
    };



    template<class T>
    struct image5D
    {
        T* data;
        xyzt<size_t> dims;
        active_channels<> chans;
        image5D() : data(0), dims(xyzt<size_t>(0,0,0,0)), chans(0,0){}
        image5D(T* _data, xyzt<size_t> _dims, active_channels<> _chans) : data(_data), dims(_dims), chans(_chans){}
        xyzct<size_t> getDims() const {return xyzct<size_t>(dims.x, dims.y, dims.z, static_cast<size_t>(chans.dim), dims.t);}
        size_t size() const {return sizeof(T)*dims.x*dims.y*dims.z*dims.t*chans.dim;}
    };
    /*-------------------------------------------------------------------------------------------------------------------------*/

}
namespace tf  = terafly;	//a short alias for the current namespace: TeraFly

class terafly::TeraFly : public QObject
{
    Q_OBJECT

    public:

        // access points
        static void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
        static void doaction(const QString &action_name);

        // returns true if version >= min_required_version, where version format is version.major.minor
        static bool checkVersion(std::string version, std::string min_required_version);
};

class terafly::PluginInterface
{
    public:

        // access the 3D curve set for the whole image at the given resolution (default: highest resolution)
        static NeuronTree getSWC(int resolution = infp<int>());
        static bool setSWC(NeuronTree & nt, int resolution = infp<int>());

        // access the 3D landmark list defined for the whole image at the given resolution (default: highest resolution)
        static LandmarkList getLandmark(int resolution = infp<int>());
        static bool setLandmark(LandmarkList & landmark_list, int resolution = infp<int>());

        // get path of the image volume at the given resolution (default: highest resolution)
        static std::string getPath(int resolution = infp<int>());

        // get currently displayed image (readonly)
        static const Image4DSimple* getImage();

        // get image metadata from the given image file/folder path
        static size_t getXDim(const std::string & path);
        static size_t getYDim(const std::string & path);
        static size_t getZDim(const std::string & path);
        static size_t getCDim(const std::string & path);
        static size_t getTDim(const std::string & path);

        // get image subvolume from the given image file/folder path
        // x = horizontal axis, y = vertical axis, z = depth axis, t = time axis
        // intervals are open at right, e.g. [x0, x1)
        static unsigned char* getSubVolume(const std::string & path, size_t x0, size_t x1, size_t y0, size_t y1, size_t z0, size_t z1, size_t t0=0, size_t t1=std::numeric_limits<size_t>::max());

        // release memory allocated for storing volume objects used in the get...(path) methods
        static void releaseOpenedVolumes();

        static std::string version(){ return "1.1.0"; }
};

#endif

