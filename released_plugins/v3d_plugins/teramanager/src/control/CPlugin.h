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

#ifndef __TERAMANAGER_C_PLUGIN_H__
#define __TERAMANAGER_C_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include <limits>

/*******************************************************************************************************************************
 *   Interfaces, types, parameters and constants   													       *
 *******************************************************************************************************************************/
namespace teramanager
{
    /*******************
    *    INTERFACES    *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    class CPlugin;              //the class defined in this header and derived from V3DPluginInterface2_1
    class PMain;                //main presentation class: it contains the main frame
    class PDialogImport;        //presentation class for the import dialog
    class PConverter;           //presentation class for the volume converter dialog
    class PAbout;               //about window
    class PLog;                 //log window
    class CImport;              //control class to perform the import step in a separate non-GUI-blocking thread
    class CVolume;              //control class to perform data loading in a separate non-GUI-blocking thread
    class CSettings;            //control class to manage persistent platform-independent application settings
    class CExplorerWindow;      //control class used to encapsulate all the informations needed to manage 3D navigation windows
    class CConverter;           //control class used to perform volume conversion operations in a separate non-GUI-blocking thread
    class CAnnotations;         //control class used to manage annotations (markers, curves, etc.) among all the resolutions
    class QArrowButton;         //Qt-customized class to model arrow buttons
    class QHelpBox;             //Qt-customized class to model help box
    class QGradientBar;         //Qt-customized class to model a gradient-colored bar
    class QLineTree;            //Qt-customized class to model a three-lined tree
    class QGLRefSys;            //Qt-customized OpenGL widget to render the XYZ reference system applied to a 3D cube
    struct annotation;          //base class for annotations

    class myRenderer_gl1;       //Vaa3D-customized class
    class myV3dR_GLWidget;      //Vaa3D-customized class
    class myV3dR_MainWindow;    //Vaa3D-customized class
    class myImage4DSimple;      //Vaa3D-customized class    
    struct point;

    enum  debug_level { NO_DEBUG, LEV1, LEV2, LEV3, LEV_MAX };  //debug levels
    class RuntimeException;		//exception thrown by functions in the current module
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *    CONSTANTS     *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    const char   undefined_str[] = "undefined";
    const int    undefined_int32 = -1;
    const int    int_inf = std::numeric_limits<int>::max();
    const float  undefined_real32 = -1.0f;
    const int    STATIC_STRING_SIZE = 2000;
    const int    FILE_LINE_BUFFER_SIZE = 10000;
    const double pi = 3.14159265359;
    const int    MAX_ANNOTATIONS_NUMBER = 1000000;       //maximum number of annotations objects (markers, curve points, etc.)
    const int    ZOOM_HISTORY_SIZE = 3;
    const std::string VMAP_BIN_FILE_NAME = "vmap.bin";   //name of volume map binary file
    const std::string RESOLUTION_PREFIX = "RES";         // prefix identifying a folder containing data of a certain resolution
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *    PARAMETERS    *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    extern std::string version;                 //version number of current module
    extern int DEBUG;							//debug level of current module
    extern bool DEBUG_TO_FILE;                  //whether debug messages should be printed on the screen or to a file (default: screen)
    extern std::string DEBUG_FILE_PATH;         //filepath where to save debug information
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

    //interval type
    struct interval_t
    {
        int start, end;
        interval_t(void) : start(-1), end(-1)  {}
        interval_t(int _start, int _end) : start(_start), end(_end){}
    };

    //block type
    struct block_t
    {
        interval_t xInt, yInt, zInt;
        block_t(interval_t _xInt, interval_t _yInt, interval_t _zInt) : xInt(_xInt), yInt(_yInt), zInt(_zInt){}
    };

    /*-------------------------------------------------------------------------------------------------------------------------*/


    /********************************************
     *   Cross-platform UTILITY functions	    *
     ********************************************
    ---------------------------------------------------------------------------------------------------------------------------*/
    // round functions
    inline int round(float  x) { return static_cast<int>(x > 0.0f ? x + 0.5f : x - 0.5f);}
    inline int round(double x) { return static_cast<int>(x > 0.0  ? x + 0.5  : x - 0.5 );}

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

    // split
    inline void	split(std::string& theString, std::string delim, std::vector<std::string>& tokens)
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
    inline void warning(const char* message, const char* source = 0){
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

    inline void debug(debug_level dbg_level, const char* message=0, const char* source=0){
        if(DEBUG >= dbg_level){
            if(DEBUG_TO_FILE)
            {
                FILE* f = fopen(DEBUG_FILE_PATH.c_str(), "a");
                if(message && source)
                    fprintf(f, "\n--------------------- teramanager plugin: DEBUG (level %d) ----: in \"%s\") ----\n"
                             "                      message: %s\n\n", dbg_level, source, message);
                else if(message)
                    fprintf(f, "\n--------------------- teramanager plugin: DEBUG (level %d) ----: %s\n", dbg_level, message);
                else if(source)
                    fprintf(f, "\n--------------------- teramanager plugin: DEBUG (level %d) ----: in \"%s\"\n", dbg_level, source);
                fclose(f);
            }
            else
            {
                if(message && source)
                    printf("\n--------------------- teramanager plugin: DEBUG (level %d) ----: in \"%s\") ----\n"
                             "                      message: %s\n\n", dbg_level, source, message);
                else if(message)
                    printf("\n--------------------- teramanager plugin: DEBUG (level %d) ----: %s\n", dbg_level, message);
                else if(source)
                    printf("\n--------------------- teramanager plugin: DEBUG (level %d) ----: in \"%s\"\n", dbg_level, source);
            }
        }
    }

    class RuntimeException
    {
        private:

            std::string source;
            std::string message;
            RuntimeException(void);

        public:

            RuntimeException(std::string _message, std::string _source = "unknown"){
                source = _source; message = _message;}
            ~RuntimeException(void){}
            const char* what() const {return message.c_str();}
            const char* getSource() const {return source.c_str();}
    };
    /*-------------------------------------------------------------------------------------------------------------------------*/
}
namespace itm = teramanager;	//a short alias for the current namespace: Icon Tera Manager (itm)


class teramanager::CPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1)

    public:

        //V3D plugin attributes and methods
        float getPluginVersion() const {return getMajorVersionFloat(version);}
        QStringList menulist() const;
        void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
        QStringList funclist() const ;
        bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

        //returns true if the given shared library can be loaded
        static float getMajorVersionFloat(string _version)
        {
            size_t pos = _version.rfind(".");
            string major_version = _version.substr(0, pos);
            QString tmp(major_version.c_str());
            return tmp.toFloat();
        }
        static float getMinorVersionFloat(string _version)
        {
            size_t pos = _version.rfind(".");
            string minor_version = _version.substr(pos, string::npos);
            QString tmp(minor_version.c_str());
            return tmp.toFloat();
        }
        static string getMajorVersion(){return QString::number(getMajorVersionFloat(version), 'f', 1).toStdString();}

        // returns true if version >= min_required_version, where version format is version.major.minor
        static bool checkPluginVersion(std::string version, std::string min_required_version);
};

#endif

