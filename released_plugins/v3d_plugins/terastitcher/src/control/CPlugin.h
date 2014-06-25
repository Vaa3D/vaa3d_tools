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

#ifndef __TERASTITCHER_C_PLUGIN_H__
#define __TERASTITCHER_C_PLUGIN_H__

#define TSP_DEBUG  //debug mode

#include <QtGui>
#include <v3d_interface.h>
#include "MyException.h"

//defining TeraStitcher plugin interface
namespace terastitcher
{
    class CPlugin;              //the class defined in this header and derived from V3DPluginInterface2_1
    class PMain;                //main presentation class: it contains the main frame with tabs, progress bar and start/stop buttons
    class PTabImport;           //presentation class for the "Import" step of the Stitching process
    class CImport;              //control class for the "Import" step, which is performed in a separate thread since it can be time-consuming
    class CPreview;             //control class for the preview feature, which is performed in a separate thread since it can be time-consuming
    class PTabDisplComp;        //presentation class for the "Pairwise Displacement Computation" step
    class CDisplComp;           //control class for the "Pairwise Displacement Computation" step, which is performed in a separate thread since it can be time-consuming
    class PTabDisplProj;        //presentation class for the "Displacement Projection" step
    class PTabDisplThresh;      //presentation class for the "Displacement Thresholding" step
    class PTabPlaceTiles;       //presentation class for the "Optimal tiles placement" step
    class PTabMergeTiles;       //presentation class for the "Merging tiles" step
    class CMergeTiles;          //control class for the "Merging tiles" step, which is performed in a separate thread since it can be time-consuming
    class QMyTabWidget;         //Qt-class which slightly modifies the QTabWidget class
    class QHelpBox;             //customized Qt label
    class QPrefixSuffixValidator;
    class QPrefixSuffixLineEdit;
    enum  debug_level { NO_DEBUG, LEV1, LEV2, LEV3, LEV_MAX };  //debug levels

    /*******************
    *    CONSTANTS     *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    const std::string IMAGE_FORMAT_SERIES           = "Series of 2D images";
    const std::string IMAGE_FORMAT_TILED_2D_ANY     = "Tiles of 2D image stacks";
    const std::string IMAGE_FORMAT_TILED_3D_ANY     = "Tiles of 3D image stacks";
    const std::string IMAGE_FORMAT_TILED_3D_TIFF    = "Tiles of multipage TIFF stacks";
    /*-------------------------------------------------------------------------------------------------------------------------*/

    /*******************
    *    PARAMETERS    *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    extern std::string version;
    extern int DEBUG;							//debug level of current module
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
    # define __tsp__current__function__ __PRETTY_FUNCTION__
    #elif defined(__DMC__) && (__DMC__ >= 0x810)
    # define __tsp__current__function__ __PRETTY_FUNCTION__
    #elif defined(__FUNCSIG__)
    # define __tsp__current__function__ __FUNCSIG__
    #elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
    # define __tsp__current__function__ __FUNCTION__
    #elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
    # define __tsp__current__function__ __FUNC__
    #elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
    # define __tsp__current__function__ __func__
    #else
    # define __tsp__current__function__ "(unknown)"
    #endif

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
                   printf("\n--------------------- TeraStitcher plugin: DEBUG (level %d) ----: in \"%s\") ----\n"
                            "                      message: %s\n\n", dbg_level, source, message);
               else if(message)
                   printf("\n--------------------- TeraStitcher plugin: DEBUG (level %d) ----: %s\n", dbg_level, message);
               else if(source)
                   printf("\n--------------------- TeraStitcher plugin: DEBUG (level %d) ----: in \"%s\"\n", dbg_level, source);
           }
       }
}

namespace tsp = terastitcher;	//a short alias for the current namespace

class terastitcher::CPlugin : public QObject, public V3DPluginInterface2_1
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

