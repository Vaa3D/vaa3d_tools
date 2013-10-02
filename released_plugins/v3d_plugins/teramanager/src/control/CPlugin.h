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

#define TMP_DEBUG 1                 //debug verbosity level
#define TMP_VMAP_FNAME "vmap.bin"   //name of volume map binary file
#define TMP_VMAP_MAXSIZE 200        //maximum size (in MVoxels) of the volume 3D map to be generated

#include <QtGui>
#include <v3d_interface.h>
#include "../core/ImageManager/MyException.h"
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
    struct annotation;          //base class for annotations

    class myRenderer_gl1;       //Vaa3D-customized class
    class myV3dR_GLWidget;      //Vaa3D-customized class
    class myV3dR_MainWindow;    //Vaa3D-customized class
    class myImage4DSimple;      //Vaa3D-customized class

    enum  debug_level { NO_DEBUG, LEV1, LEV2, LEV3, LEV_MAX };  //debug levels
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *    CONSTANTS     *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    const char undefined_str[] = "undefined";
    const int  undefined_int32 = -1;
    const int  int_inf = std::numeric_limits<int>::max();
    const float undefined_real32 = -1.0f;
    const int STATIC_STRING_SIZE = 2000;
    const int FILE_LINE_BUFFER_SIZE = 10000;
    const double pi = 3.14159265359;
    const int MAX_ANNOTATIONS_NUMBER = 1000000; //the maximum number of annotations objects (markers, curve points, etc.)
    const int ZOOM_HISTORY_SIZE = 3;
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *    PARAMETERS    *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    extern std::string version;                 //version number of current module
    extern int DEBUG;							//debug level of current module
    /*-------------------------------------------------------------------------------------------------------------------------*/


    /*******************
    *    TYPES         *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    typedef unsigned int uint32;

    //interval type
    struct interval_t
    {
        int start, end;
        interval_t(void) : start(-1), end(-1)  {}
        interval_t(int _start, int _end) : start(_start), end(_end){}
    };
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
        if(source)
            printf("\n**** WARNING (source: \"%s\") ****\n"
            "    |=> \"%s\"\n\n", source, message);
        else
            printf("\n**** WARNING ****: %s\n", message);
    }

    inline void debug(debug_level dbg_level, const char* message=0, const char* source=0){
        if(DEBUG >= dbg_level){
            if(message && source)
                printf("\n--------------------- teramanager plugin: DEBUG (level %d, source: \"%s\") ----\n"
                         "               |====> message: \"%s\"\n\n", dbg_level, source, message);
            else if(message)
                printf("\n--------------------- teramanager plugin: DEBUG (level %d) ----: %s\n", dbg_level, message);
            else if(source)
                printf("\n--------------------- teramanager plugin: DEBUG (level %d) ----: in \"%s\"\n", dbg_level, source);
        }
    }
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
        static string getMajorVersion(){return QString::number(getMajorVersionFloat(version), 'f', 1).toStdString();}
};

#endif

